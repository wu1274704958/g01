#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Controller.h"
#include "MqasP2P.h"
#include "P2PLinkController.generated.h"

class UP2PStateSyncComponent;

// Owns one authenticated peer and its representation of the remote player.
UCLASS(NotBlueprintable)
class AP2PLinkController : public AController
{
    GENERATED_BODY()
public:
    DECLARE_MULTICAST_DELEGATE_OneParam(FStatus, const FString&);
    FStatus OnStatus;
    bool StartLink(GSY_PeerId LocalPeer, const FP2PHelperResult& Result);
    void SendState(const TArray<uint8>& Data);
protected:
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    void TransportConnected();
    void DatagramConnected();
    void ReceiveState(const TArray<uint8>& Data);
    void Failed(ErrorCode Error);
    void DatagramDisconnected(const FString& Reason);
    void Closed(ErrorCode Error);
    void StopLink();
    std::shared_ptr<FMqasPeer> Peer;
    std::shared_ptr<FMqasDatagram> Datagram;
    FP2PHelperResult Session;
    bool bServer = false;
    bool bAuthenticated = false;
    bool bStopping = false;
    uint32 LastSequence = 0;
    bool bHasSequence = false;
    UPROPERTY(Transient) TObjectPtr<UP2PStateSyncComponent> StateSync;
    UPROPERTY(Transient) TObjectPtr<APawn> RemotePawn;
};
