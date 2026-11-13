#include "P2PLinkController.h"
#include "P2PStateSyncComponent.h"
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
    Peer->OnConnected.AddUObject(this, &AP2PLinkController::TransportConnected);
    Peer->OnFailed.AddLambda([Result](ErrorCode) { Result.ReleaseSession(); });
    Peer->OnFailed.AddUObject(this, &AP2PLinkController::Failed);
    Peer->OnClosed.AddUObject(this, &AP2PLinkController::Closed);
    Datagram->OnConnected.AddUObject(this, &AP2PLinkController::DatagramConnected);
    Datagram->OnReceived.AddUObject(this, &AP2PLinkController::ReceiveState);
    Datagram->OnFailed.AddUObject(this, &AP2PLinkController::Failed);
    Datagram->OnDisconnected.AddUObject(this, &AP2PLinkController::DatagramDisconnected);
    const FString Config = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("NetConfig/conf_p2p.txt"));
    const auto Error = Peer->Start(Config, LocalPeer, Result);
    if (Error != EC_Pending) { UE_LOG(LogTemp, Warning, TEXT("P2P start failed: %d"), int32(Error)); Session.ReleaseSession(); return false; }
    UE_LOG(LogTemp, Log, TEXT("P2P peer starting: remote=%u server=%d"), Result.Data.peer_id, int32(bServer));
    return true;
}

void AP2PLinkController::TransportConnected()
{
    bAuthenticated = true;
    UE_LOG(LogTemp, Log, TEXT("P2P transport authenticated"));
    OnStatus.Broadcast(TEXT("P2P authenticated; opening data stream..."));
    if (!bServer && Datagram->Open() != EC_Pending) StopLink();
}

void AP2PLinkController::DatagramConnected()
{
    auto* LocalController = UGameplayStatics::GetPlayerController(this, 0);
    auto* Local = LocalController ? Cast<ACharacter>(LocalController->GetPawn()) : nullptr;
    if (!Local) {
        UE_LOG(LogTemp, Warning, TEXT("P2P datagram connected, but the local player has no Character pawn"));
        OnStatus.Broadcast(TEXT("P2P connected, but this map has no local Character pawn"));
        StopLink(); return;
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
    if (Peer) Peer->Close();
    Datagram.reset();
    Peer.reset();
    Super::EndPlay(Reason);
}
