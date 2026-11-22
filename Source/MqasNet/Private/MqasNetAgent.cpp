#include "MqasNetAgent.h"
#include "MqasClientSubsystem.h"
#include "Engine/GameInstance.h"

FMqasClientId AUMqasNetAgent::GetClient() const
{
    const auto* GI = GetGameInstance();
    const auto* Subsystem = GI ? GI->GetSubsystem<UMqasClientSubsystem>() : nullptr;
    return Subsystem ? Subsystem->GetClientId() : InvalidMqasClient;
}
bool AUMqasNetAgent::IsInitialized() const { return AgentIsInitialized(GetClient()); }
bool AUMqasNetAgent::IsConnected(GSY_ConnectionHwnd H) { return AgentIsConnected(GetClient(), H); }
std::weak_ptr<BaseConnect> AUMqasNetAgent::ConnectTo(GSY_EngineId E, const FString& C, const FString& I, short P)
{ return AgentConnectTo(GetClient(), E, C, I, P); }
bool AUMqasNetAgent::AgentIsInitialized(FMqasClientId C) const
{ return C == GetClient() && FMqasAgent::Get().AgentIsInitialized(C); }
bool AUMqasNetAgent::AgentIsConnected(FMqasClientId C, GSY_ConnectionHwnd H) const
{ return C == GetClient() && FMqasAgent::Get().AgentIsConnected(C, H); }
TArray<GSY_PeerId> AUMqasNetAgent::AgentGetPeerIds(FMqasClientId C) const
{ return C == GetClient() ? FMqasAgent::Get().AgentGetPeerIds(C) : TArray<GSY_PeerId>{}; }
std::weak_ptr<BaseConnect> AUMqasNetAgent::AgentConnectTo(FMqasClientId C, GSY_EngineId E, const FString& F, const FString& I, short P)
{ return C == GetClient() ? FMqasAgent::Get().AgentConnectTo(C,E,F,I,P) : std::weak_ptr<BaseConnect>{}; }

void AUMqasNetAgent::BeginPlay()
{
    Super::BeginPlay();
    FMqasAgent::Get().OnError.AddUObject(this, &AUMqasNetAgent::OnGlobalError);
}
void AUMqasNetAgent::EndPlay(const EEndPlayReason::Type Reason)
{
    FMqasAgent::Get().OnError.RemoveAll(this);
    Super::EndPlay(Reason);
}
void AUMqasNetAgent::OnGlobalError(const FString& Message, int Code) { OnError.Broadcast(Message, Code); }
