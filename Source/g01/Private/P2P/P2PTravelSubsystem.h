#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MqasP2P.h"
#include "P2PTravelSubsystem.generated.h"

class AP2PLinkController;
class APlayerController;

// GameInstance-owned bridge between the lobby world and the P2P game world.
UCLASS()
class UP2PTravelSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void EnterP2PLevel(UWorld* World);
    uint32 NextStateSequence(GSY_PeerId RemotePeer);
    static bool IsP2PLevel(const UWorld* World);

private:
    void OnPostLoadMap(UWorld* World);
    void RebindPeers();

    FDelegateHandle PostLoadHandle;
    FTimerHandle RebindTimer;
    TWeakObjectPtr<UWorld> RebindWorld;
    TWeakObjectPtr<APlayerController> InputController;
    TMap<GSY_PeerId, TWeakObjectPtr<AP2PLinkController>> Links;
    TMap<GSY_PeerId, uint32> Sequences;
    bool bTravelRequested = false;
    bool bLoggedLocalCharacter = false;
    int32 PendingPawnTicks = 0;
};
