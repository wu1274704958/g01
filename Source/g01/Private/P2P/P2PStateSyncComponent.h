#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "P2PStateSyncComponent.generated.h"

class AP2PLinkController;

UCLASS()
class UP2PStateSyncComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UP2PStateSyncComponent();
    void InitializeLink(AP2PLinkController* InLink);
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction) override;
private:
    TWeakObjectPtr<AP2PLinkController> Link;
    float Elapsed = 0;
    uint32 Sequence = 0;
};
