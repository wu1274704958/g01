#include "MqasClientSubsystem.h"
TArray<GSY_PeerId> UMqasClientSubsystem::GetPeerIds() const
{
    return FMqasAgent::Get().AgentGetPeerIds(ClientId);
}
void UMqasClientSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ClientId = FMqasAgent::Get().CreateClient();
}
void UMqasClientSubsystem::Deinitialize()
{
    FMqasAgent::Get().ReleaseClient(ClientId);
    ClientId = InvalidMqasClient;
    Super::Deinitialize();
}
