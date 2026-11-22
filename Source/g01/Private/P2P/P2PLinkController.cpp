#include "P2PLinkController.h"
#include "P2PStateSyncComponent.h"
#include "P2PTravelSubsystem.h"
#include "MqasClientSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "P2PStatePacket.h"

bool AP2PLinkController::StartLink(GSY_PeerId LocalPeer, const FP2PHelperResult& Result)
{
    Session = Result;
    bServer = Result.Data.is_server != 0;
    if (Result.Data.ret != EC_Ok || Result.Data.use_relay)
    {
        Session.ReleaseSession();
        return false;
    }
    Peer = FMqasPeer::Create();
    Datagram = FMqasDatagram::Create();
    if (!Peer->SetDatagram(Datagram)) { Session.ReleaseSession(); return false; }
    BindLinkEvents();
    Peer->OnConnected.AddLambda([Weak = std::weak_ptr<FMqasDatagram>(Datagram),
        WeakPeer = std::weak_ptr<FMqasPeer>(Peer), Server = bServer] {
        if (!Server) if (auto D = Weak.lock()) {
            if (D->Open() != EC_Pending) {
                UE_LOG(LogTemp, Warning, TEXT("P2P datagram open failed"));
                if (auto P = WeakPeer.lock()) P->Close();
            }
        }
    });
    Peer->OnFailed.AddLambda([Result](ErrorCode) { Result.ReleaseSession(); });
    auto* Subsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UMqasClientSubsystem>() : nullptr;
    if (!Subsystem) { Session.ReleaseSession(); return false; }
    const auto Client = Subsystem->GetClientId();
    std::shared_ptr<FMqasPeer> ExistingPeer;
    std::shared_ptr<FMqasDatagram> ExistingDatagram;
    if (FMqasAgent::Get().FindPeer(Client, Result.Data.peer_id, ExistingPeer, ExistingDatagram)) {
        UE_LOG(LogTemp, Warning, TEXT("P2P peer %u already has an active link"), Result.Data.peer_id);
        Session.ReleaseSession(); return false;
    }
    const FString Config = FMqasAgent::Get().GetConfigPath(Client, TEXT("NetConfig/conf_p2p.txt"));
    const auto Error = Peer->Start(Config, LocalPeer, Result);
    if (Error != EC_Pending) { UE_LOG(LogTemp, Warning, TEXT("P2P start failed: %d"), int32(Error)); Session.ReleaseSession(); return false; }
    FMqasAgent::Get().RetainPeer(Client, Peer, Datagram);
    UE_LOG(LogTemp, Log, TEXT("P2P peer starting: remote=%u server=%d"), Result.Data.peer_id, int32(bServer));
    return true;
}

void AP2PLinkController::BindLinkEvents()
{
    Peer->OnConnected.AddUObject(this, &AP2PLinkController::TransportConnected);
    Peer->OnFailed.AddUObject(this, &AP2PLinkController::Failed);
    Peer->OnClosed.AddUObject(this, &AP2PLinkController::Closed);
    Datagram->OnConnected.AddUObject(this, &AP2PLinkController::DatagramConnected);
    Datagram->OnReceived.AddUObject(this, &AP2PLinkController::ReceiveState);
    Datagram->OnFailed.AddUObject(this, &AP2PLinkController::Failed);
    Datagram->OnDisconnected.AddUObject(this, &AP2PLinkController::DatagramDisconnected);
}
bool AP2PLinkController::AttachLink(GSY_PeerId RemotePeer)
{
    auto* Subsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UMqasClientSubsystem>() : nullptr;
    if (!Subsystem || Peer || !FMqasAgent::Get().FindPeer(Subsystem->GetClientId(), RemotePeer, Peer, Datagram)) return false;
    bAuthenticated = Peer->IsConnected();
    BindLinkEvents();
    if (Datagram->IsConnected()) DatagramConnected();
    return true;
}

void AP2PLinkController::TransportConnected()
{
    bAuthenticated = true;
    UE_LOG(LogTemp, Log, TEXT("P2P transport authenticated"));
    OnStatus.Broadcast(TEXT("P2P authenticated; opening data stream..."));

}

void AP2PLinkController::DatagramConnected()
{
    if (StateSync || RemotePawn) return;
    if (!UP2PTravelSubsystem::IsP2PLevel(GetWorld())) {
        auto* Travel = GetGameInstance() ? GetGameInstance()->GetSubsystem<UP2PTravelSubsystem>() : nullptr;
        if (Travel) {
            OnStatus.Broadcast(TEXT("P2P connected; loading game level..."));
            Travel->EnterP2PLevel(GetWorld());
        } else {
            UE_LOG(LogTemp, Warning, TEXT("P2P connected, but the travel subsystem is unavailable"));
        }
        return;
    }
    auto* LocalController = UGameplayStatics::GetPlayerController(this, 0);
    auto* Local = LocalController ? Cast<ACharacter>(LocalController->GetPawn()) : nullptr;
    if (!Local) {
        UE_LOG(LogTemp, Warning, TEXT("P2P datagram connected, but the local player has no Character pawn"));
        OnStatus.Broadcast(TEXT("P2P connected, but this map has no local Character pawn"));
        return;
    }
    RemotePawn = GetWorld()->SpawnActorDeferred<APawn>(Local->GetClass(), Local->GetActorTransform(), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (!RemotePawn) { StopLink(); return; }
    RemotePawn->AutoPossessAI = EAutoPossessAI::Disabled;
    RemotePawn->AutoPossessPlayer = EAutoReceiveInput::Disabled;
    RemotePawn->FinishSpawning(Local->GetActorTransform());
    RemotePawn->SetActorEnableCollision(false);
    if (auto* RemoteCharacter = Cast<ACharacter>(RemotePawn)) RemoteCharacter->GetCharacterMovement()->SetComponentTickEnabled(false);
    Possess(RemotePawn);
    StateSync = NewObject<UP2PStateSyncComponent>(Local);
    StateSync->InitializeLink(this);
    StateSync->RegisterComponent();
    UE_LOG(LogTemp, Log, TEXT("P2P datagram ready; character synchronization started"));
    OnStatus.Broadcast(TEXT("P2P connected; character synchronization started"));
}

uint32 AP2PLinkController::NextStateSequence()
{
    if (Peer && GetGameInstance())
        if (auto* Travel = GetGameInstance()->GetSubsystem<UP2PTravelSubsystem>())
            return Travel->NextStateSequence(Peer->GetRemotePeerId());
    return ++FallbackSequence;
}

void AP2PLinkController::SendState(const TArray<uint8>& Data)
{
    if (Datagram && Datagram->IsConnected()) Datagram->Send(Data);
}

void AP2PLinkController::ReceiveState(const TArray<uint8>& Data)
{
    FP2PStatePacket Packet;
    if (!RemotePawn || !FP2PStatePacket::Decode(Data, Packet) ||
        (bHasSequence && !FP2PStatePacket::IsNewer(Packet.Sequence, LastSequence))) return;
    bHasSequence = true;
    LastSequence = Packet.Sequence;
    RemotePawn->SetActorLocationAndRotation(Packet.Position, Packet.Rotation, false, nullptr, ETeleportType::TeleportPhysics);
    if (auto* RemoteCharacter = Cast<ACharacter>(RemotePawn)) RemoteCharacter->GetCharacterMovement()->Velocity = Packet.Velocity;
}

void AP2PLinkController::Failed(ErrorCode Error)
{
    UE_LOG(LogTemp, Warning, TEXT("P2P link failed: %d"), int32(Error));
    OnStatus.Broadcast(FString::Printf(TEXT("P2P connection failed: %d"), int32(Error)));
    if (!bAuthenticated) Session.ReleaseSession();
    StopLink();
}
void AP2PLinkController::DatagramDisconnected(const FString& Reason)
{
    UE_LOG(LogTemp, Log, TEXT("P2P datagram disconnected: %s"), *Reason);
    StopLink();
}
void AP2PLinkController::Closed(ErrorCode Error)
{
    UE_LOG(LogTemp, Log, TEXT("P2P link closed: %d"), int32(Error));
    StopLink();
}
void AP2PLinkController::StopLink()
{
    if (bStopping) return;
    bStopping = true;
    if (Peer) Peer->Close();
    Destroy();
}
void AP2PLinkController::EndPlay(const EEndPlayReason::Type Reason)
{
    if (StateSync) StateSync->DestroyComponent();
    if (RemotePawn) RemotePawn->Destroy();
    if (Peer) { Peer->OnConnected.RemoveAll(this); Peer->OnFailed.RemoveAll(this); Peer->OnClosed.RemoveAll(this); }
    if (Datagram) { Datagram->OnConnected.RemoveAll(this); Datagram->OnReceived.RemoveAll(this); Datagram->OnFailed.RemoveAll(this); Datagram->OnDisconnected.RemoveAll(this); }
    Datagram.reset();
    Peer.reset();
    Super::EndPlay(Reason);
}
