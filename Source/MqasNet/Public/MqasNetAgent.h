#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MqasAgent.h"
#include "MqasNet/Public/Common.h"
#include "MqasNetAgent.generated.h"

UCLASS()
class MQASNET_API AUMqasNetAgent : public AActor, public IMqasAgent
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="MqasNetAgent")
    bool IsInitialized() const;
    bool IsConnected(GSY_ConnectionHwnd Handle);
    std::weak_ptr<BaseConnect> ConnectTo(GSY_EngineId Engine, const FString& Config, const FString& Ip, short Port);
    bool AgentIsInitialized(FMqasClientId Client) const override;
    bool AgentIsConnected(FMqasClientId Client, GSY_ConnectionHwnd Handle) const override;
    TArray<GSY_PeerId> AgentGetPeerIds(FMqasClientId Client) const override;
    std::weak_ptr<BaseConnect> AgentConnectTo(FMqasClientId Client, GSY_EngineId Engine,
        const FString& Config, const FString& Ip, short Port) override;
    FOnErrorDelegate OnError;
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    void OnGlobalError(const FString& Message, int Code);
    FMqasClientId GetClient() const;
};
