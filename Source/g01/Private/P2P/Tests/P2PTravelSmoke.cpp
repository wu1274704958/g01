#if WITH_DEV_AUTOMATION_TESTS
#include "P2P/P2PLinkController.h"
#include "P2P/P2PTravelSubsystem.h"
#include "MqasAgent.h"
#include "MqasClientSubsystem.h"
#include "P2PHelperStream.h"
#include "EUtility/UI/UIUtility.h"
#include "Blueprint/UserWidget.h"
#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "InputKeyEventArgs.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include <memory>

namespace {
struct FTravelSmoke : ILobbyEventListener, std::enable_shared_from_this<FTravelSmoke>
{
    TWeakObjectPtr<UGameInstance> GameInstance;
    std::shared_ptr<BaseConnect> Connection;
    std::shared_ptr<P2PHelperStream> Lobby;
    bool bHost = false, bRequested = false, bMoved = false, bPassed = false;
    bool bRequestedGC = false, bCheckedInput = false;
    bool bKeyPressed = false, bLocalMovedByInput = false;
    GSY_PeerId Self = INVALID_PID;
    double LastFetch = 0;
    double ToastShownAt = 0;

    void Start(UWorld* World, bool Host, uint16 Port)
    {
        GameInstance = World ? World->GetGameInstance() : nullptr;
        bHost = Host;
        auto* GI = GameInstance.Get();
        auto* Client = GI ? GI->GetSubsystem<UMqasClientSubsystem>() : nullptr;
        if (!Client) { Fail(TEXT("GameInstance client missing")); return; }
        Connection = FMqasAgent::Get().AgentConnectTo(Client->GetClientId(), 1,
            TEXT("NetConfig/hole_punching_config.txt"), TEXT("127.0.0.1"), static_cast<short>(Port)).lock();
        if (!Connection) { Fail(TEXT("Lobby connection could not start")); return; }
        std::weak_ptr<FTravelSmoke> Weak = shared_from_this();
        Connection->OnConnectedDelegate.AddLambda([Weak](int Code, GSY_ConnectionHwnd) {
            if (auto SelfPtr = Weak.lock()) SelfPtr->Connected(Code);
        });
        Connection->OnConnectErrorDelegate.AddLambda([Weak](int Code, GSY_ConnectionHwnd) {
            if (auto SelfPtr = Weak.lock()) SelfPtr->Fail(FString::Printf(TEXT("Lobby connection error %d"), Code));
        });
        FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Weak](float Delta) {
            if (auto SelfPtr = Weak.lock()) return SelfPtr->Tick(Delta);
            return false;
        }), 0.2f);
    }
    void Connected(int Code)
    {
        if (Code != EC_Ok) { Fail(TEXT("Lobby handshake failed")); return; }
        const char* Name = bHost ? "g01-smoke-host" : "g01-smoke-join";
        Lobby = Connection->MakeStream<P2PHelperStream, GSY_LobbyStreamContext, const char*, const char*>(
            GSY_RegisterToLobby, static_cast<const char*>(Name), static_cast<const char*>(""));
        if (!Lobby) { Fail(TEXT("Lobby registration could not start")); return; }
        Lobby->AddListener(this);
    }
    void OnRegistrationSuccess(ErrorCode Error, GSY_StreamId, GSY_PeerId Peer) override
    {
        if (Error != EC_Ok) { Fail(TEXT("Lobby registration failed")); return; }
        Self = Peer;
        UE_LOG(LogTemp, Log, TEXT("P2P_TRAVEL_SMOKE registered %u"), Peer);
    }
    void OnReceivePeerList(GSY_PeerData* List, uint32 Size, GSY_StreamId Sid) override
    {
        if (bHost || bRequested || !Connection) return;
        for (uint32 Index = 0; Index < Size; ++Index) {
            if (List[Index].peer_id != Self) {
                bRequested = true;
                if (GSY_RequestConnectPeer(Connection->GetHwnd(), Sid, List[Index].peer_id, NONE_RID) != EC_Pending)
                    Fail(TEXT("Peer request failed"));
                return;
            }
        }
    }
    void OnPeerReqConnect(GSY_PeerData* Peer, GSY_StreamId Sid) override
    {
        if (Peer && Connection && GSY_RespondPeerConnectRequest(Connection->GetHwnd(), Sid, Peer->peer_id, true, NONE_RID) != EC_Pending)
            Fail(TEXT("Peer response failed"));
    }
    void OnHelperResult(GSY_StreamId, const FP2PHelperResult& Result) override
    {
        auto* GI = GameInstance.Get();
        auto* World = GI ? GI->GetWorld() : nullptr;
        if (!World || Result.Data.ret != EC_Ok) { Fail(TEXT("P2P helper failed")); return; }
        auto* PC = UGameplayStatics::GetPlayerController(World, 0);
        if (!PC || !World->GetGameViewport()) { Fail(TEXT("Lobby player or viewport unavailable")); return; }
        PC->SetInputMode(FInputModeUIOnly());
        PC->bShowMouseCursor = true;
        if (!World->GetGameViewport()->IgnoreInput()) { Fail(TEXT("Could not reproduce lobby UI input mode")); return; }
        const auto ToastClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/UI/Toast/UW_ToastDef.UW_ToastDef_C"));
        if (!ToastClass) { Fail(TEXT("Toast class unavailable")); return; }
        UIUtility::ShowToast(World, 0, ToastClass, TEXT("P2P travel regression"), 1.f);
        ToastShownAt = FPlatformTime::Seconds();
        UE_LOG(LogTemp, Log, TEXT("P2P_TRAVEL_SMOKE toast before travel"));
        auto* Link = World->SpawnActor<AP2PLinkController>();
        if (!Link || !Link->StartLink(Self, Result)) { if (Link) Link->Destroy(); Fail(TEXT("P2P link failed to start")); }
    }
    bool Tick(float)
    {
        if (bPassed) return false;
        if (Lobby && Self != INVALID_PID && !bHost && !bRequested && FPlatformTime::Seconds() - LastFetch > 0.5) {
            LastFetch = FPlatformTime::Seconds();
            Lobby->FetchPeerList();
        }
        auto* GI = GameInstance.Get();
        UWorld* World = GI ? GI->GetWorld() : nullptr;
        if (!UP2PTravelSubsystem::IsP2PLevel(World)) return true;
        if (!bRequestedGC && GEngine) {
            bRequestedGC = true;
            GEngine->ForceGarbageCollection(true);
            UE_LOG(LogTemp, Log, TEXT("P2P_TRAVEL_SMOKE requested garbage collection after travel"));
        }
        auto* PC = UGameplayStatics::GetPlayerController(World, 0);
        auto* Local = PC ? Cast<ACharacter>(PC->GetPawn()) : nullptr;
        if (!Local) return true;
        if (!bCheckedInput) {
            auto* Viewport = World->GetGameViewport();
            if (!Viewport || Viewport->IgnoreInput()) {
                Fail(TEXT("Viewport still ignores gameplay input after travel"));
                return false;
            }
            if (PC->bShowMouseCursor) { Fail(TEXT("Mouse cursor still shown after travel")); return false; }
            bCheckedInput = true;
        }
        if (!bMoved) {
            FVector Location = Local->GetActorLocation();
            Location.X = bHost ? -400 : 400;
            Location.Y = bHost ? -200 : 200;
            Local->SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
            bMoved = true;
        }
        for (TActorIterator<AP2PLinkController> It(World); It; ++It) {
            const APawn* Remote = It->GetPawn();
            if (!Remote || Remote == Local) continue;
            const FVector Position = Remote->GetActorLocation();
            const FVector2D Expected = bHost ? FVector2D(400, 200) : FVector2D(-400, -200);
            const float RemoteDistance = FVector2D::Distance(FVector2D(Position.X, Position.Y), Expected);
            auto* Viewport = World->GetGameViewport();
            if (!bKeyPressed && RemoteDistance < 30) {
                bKeyPressed = true;
                UE_LOG(LogTemp, Log, TEXT("P2P_TRAVEL_SMOKE sending W; remote distance %.1f"), RemoteDistance);
                Viewport->InputKey(FInputKeyEventArgs(Viewport->Viewport, FInputDeviceId::CreateFromInternalId(0), EKeys::W, IE_Pressed, FPlatformTime::Cycles64()));
            }
            const FVector LocalPosition = Local->GetActorLocation();
            const FVector2D LocalStart = bHost ? FVector2D(-400, -200) : FVector2D(400, 200);
            if (bKeyPressed && !bLocalMovedByInput && FVector2D::Distance(FVector2D(LocalPosition.X, LocalPosition.Y), LocalStart) > 5) {
                bLocalMovedByInput = true;
                UE_LOG(LogTemp, Log, TEXT("P2P_TRAVEL_SMOKE local input moved character"));
                Viewport->InputKey(FInputKeyEventArgs(Viewport->Viewport, FInputDeviceId::CreateFromInternalId(0), EKeys::W, IE_Released, FPlatformTime::Cycles64()));
            }
            if (bLocalMovedByInput && RemoteDistance > 5) {
                if (ToastShownAt > 0 && FPlatformTime::Seconds() - ToastShownAt >= 3.0) {
                    bPassed = true;
                    UE_LOG(LogTemp, Display, TEXT("P2P_TRAVEL_SMOKE_PASS role=%s remote=(%.1f,%.1f)"),
                        bHost ? TEXT("host") : TEXT("join"), Position.X, Position.Y);
                    return false;
                }
            }
        }
        return true;
    }
    void Fail(const FString& Reason) { UE_LOG(LogTemp, Error, TEXT("P2P_TRAVEL_SMOKE_FAIL %s"), *Reason); }
};
std::shared_ptr<FTravelSmoke> Smoke;
void StartSmoke(const TArray<FString>& Args, UWorld* World)
{
    if (Args.Num() != 2 || !World) { UE_LOG(LogTemp, Error, TEXT("P2P_TRAVEL_SMOKE_FAIL invalid arguments")); return; }
    Smoke = std::make_shared<FTravelSmoke>();
    Smoke->Start(World, Args[0] == TEXT("host"), static_cast<uint16>(FCString::Atoi(*Args[1])));
}
FAutoConsoleCommandWithWorldAndArgs Command(
    TEXT("P2P.TravelSmoke"), TEXT("Test authenticated P2P travel and state sync"),
    FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&StartSmoke));
}
#endif
