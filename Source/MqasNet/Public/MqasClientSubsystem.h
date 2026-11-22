#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MqasAgent.h"
#include "MqasClientSubsystem.generated.h"

UCLASS()
class MQASNET_API UMqasClientSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    FMqasClientId GetClientId() const { return ClientId; }
    TArray<GSY_PeerId> GetPeerIds() const;
private:
    FMqasClientId ClientId = InvalidMqasClient;
};
