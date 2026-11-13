#pragma once

#include "CoreMinimal.h"
#include "P2PHelperStream.h"
#include "example1/p2p_peer.h"
#include "example1/p2p_connection_datagram.h"
#include <memory>

struct FMqasPeerState;
class FMqasDatagram;

// All methods and delegates are used on the game thread. The SDK retains the
// native callback state until the peer has finished closing.
class MQASNET_API FMqasPeer : public std::enable_shared_from_this<FMqasPeer>
{
public:
    static std::shared_ptr<FMqasPeer> Create();
    ~FMqasPeer();
    // Configure a datagram receiver before Start on the server side. A client
    // may attach one after starting its transport.
    bool SetDatagram(const std::shared_ptr<FMqasDatagram>& Datagram);
    ErrorCode Start(const FString& ConfigPath, GSY_PeerId Self, const FP2PHelperResult& Result);
    void Close();
    bool IsConnected() const;
    GSY_PeerId GetRemotePeerId() const;
    GSY_ConnectionHwnd GetConnection() const;
    DECLARE_MULTICAST_DELEGATE(FConnected);
    DECLARE_MULTICAST_DELEGATE_OneParam(FTerminal, ErrorCode);
    FConnected OnConnected;
    FTerminal OnFailed;
    FTerminal OnClosed;
private:
    FMqasPeer();
    std::shared_ptr<FMqasPeerState> State;
    friend struct FMqasPeerState;
};

class MQASNET_API FMqasDatagram : public std::enable_shared_from_this<FMqasDatagram>
{
public:
    static std::shared_ptr<FMqasDatagram> Create();
    ~FMqasDatagram();
    // Client only; the server receives streams through the peer's I/O callback.
    ErrorCode Open(GSY_RequestId RequestId = NONE_RID);
    ErrorCode Send(const TArray<uint8>& Data, GSY_RequestId RequestId = NONE_RID);
    ErrorCode Disconnect(const FString& Reason = FString(), GSY_RequestId RequestId = NONE_RID);
    bool IsConnected() const;
    DECLARE_MULTICAST_DELEGATE(FConnected);
    DECLARE_MULTICAST_DELEGATE_OneParam(FFailed, ErrorCode);
    DECLARE_MULTICAST_DELEGATE_OneParam(FReceived, const TArray<uint8>&);
    DECLARE_MULTICAST_DELEGATE_OneParam(FDisconnected, const FString&);
    FConnected OnConnected;
    FFailed OnFailed;
    FReceived OnReceived;
    FDisconnected OnDisconnected;
private:
    FMqasDatagram() = default;
    std::weak_ptr<FMqasPeerState> State;
    friend struct FMqasPeerState;
    friend class FMqasPeer;
};

// Called by the SDK owner after GSY_terminate has drained native callbacks.
void MQASNET_API MqasP2PShutdown();
