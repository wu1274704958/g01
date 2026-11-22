#include "P2PTravelSubsystem.h"

#include "P2PLinkController.h"
#include "MqasClientSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace {
const FName P2PLevelPackage(TEXT("/Game/Maps/P2PLevel1"));
const FString P2PLevelName(TEXT("P2PLevel1"));
}

void UP2PTravelSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    PostLoadHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UP2PTravelSubsystem::OnPostLoadMap);
}

void UP2PTravelSubsystem::Deinitialize()
{
    FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadHandle);
    if (UWorld* World = RebindWorld.Get()) World->GetTimerManager().ClearTimer(RebindTimer);
    Links.Empty();
    Sequences.Empty();
    RebindWorld.Reset();
    InputController.Reset();
    Super::Deinitialize();
}

bool UP2PTravelSubsystem::IsP2PLevel(const UWorld* World)
{
    return World && UWorld::RemovePIEPrefix(World->GetMapName()) == P2PLevelName;
}

void UP2PTravelSubsystem::EnterP2PLevel(UWorld* World)
{
    if (!World || World->GetGameInstance() != GetGameInstance() || bTravelRequested) return;
    if (IsP2PLevel(World)) {
        OnPostLoadMap(World);
        return;
    }
    bTravelRequested = true;
    UE_LOG(LogTemp, Log, TEXT("P2P datagram ready; travelling to %s"), *P2PLevelPackage.ToString());
    UGameplayStatics::OpenLevel(World, P2PLevelPackage);
}

void UP2PTravelSubsystem::OnPostLoadMap(UWorld* World)
{
    if (!World || World->GetGameInstance() != GetGameInstance()) return;
    bTravelRequested = false;
    if (World == RebindWorld.Get() && World->GetTimerManager().IsTimerActive(RebindTimer)) return;
    if (UWorld* Previous = RebindWorld.Get()) Previous->GetTimerManager().ClearTimer(RebindTimer);
    RebindWorld.Reset();
    InputController.Reset();
    Links.Empty();
    PendingPawnTicks = 0;
    bLoggedLocalCharacter = false;
    if (!IsP2PLevel(World)) return;
    RebindWorld = World;
    UE_LOG(LogTemp, Log, TEXT("P2P level loaded; waiting for the local Character and retained peers"));
    World->GetTimerManager().SetTimer(RebindTimer, this, &UP2PTravelSubsystem::RebindPeers, 0.2f, true, 0.f);
}

uint32 UP2PTravelSubsystem::NextStateSequence(GSY_PeerId RemotePeer)
{
    return ++Sequences.FindOrAdd(RemotePeer);
}

void UP2PTravelSubsystem::RebindPeers()
{
    UWorld* World = RebindWorld.Get();
    if (!World || World->bIsTearingDown) return;
    auto* Controller = UGameplayStatics::GetPlayerController(World, 0);
    if (Controller && Controller->IsLocalController() && InputController.Get() != Controller) {
        Controller->SetInputMode(FInputModeGameOnly());
        Controller->bShowMouseCursor = false;
        InputController = Controller;
        UE_LOG(LogTemp, Log, TEXT("P2P level restored gameplay input"));
    }
    if (!Controller || !Cast<ACharacter>(Controller->GetPawn())) {
        if (++PendingPawnTicks == 25)
            UE_LOG(LogTemp, Warning, TEXT("P2P level has no local Character; waiting to attach peers"));
        return;
    }
    if (!bLoggedLocalCharacter) {
        bLoggedLocalCharacter = true;
        UE_LOG(LogTemp, Log, TEXT("P2P level local Character ready: %s"), *GetNameSafe(Controller->GetPawn()));
    }
    const auto* Client = GetGameInstance()->GetSubsystem<UMqasClientSubsystem>();
    if (!Client) return;
    const TArray<GSY_PeerId> PeerIds = Client->GetPeerIds();
    for (auto It = Links.CreateIterator(); It; ++It)
        if (!PeerIds.Contains(It.Key()) || !It.Value().IsValid()) It.RemoveCurrent();
    for (GSY_PeerId PeerId : PeerIds) {
        if (Links.Contains(PeerId)) continue;
        auto* Link = World->SpawnActor<AP2PLinkController>();
        if (!Link) continue;
        if (Link->AttachLink(PeerId)) {
            Links.Add(PeerId, Link);
            UE_LOG(LogTemp, Log, TEXT("P2P level attached peer %u"), PeerId);
        } else {
            Link->Destroy();
        }
    }
}
