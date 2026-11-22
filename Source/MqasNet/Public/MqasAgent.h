#pragma once
#include "CoreMinimal.h"
#include "BaseConnect.h"
#include "MqasP2P.h"
#include "Common.h"
#include <memory>

using FMqasClientId = uint64;
constexpr FMqasClientId InvalidMqasClient = 0;

class MQASNET_API IMqasAgent
{
public:
    virtual ~IMqasAgent() = default;
    virtual bool AgentIsInitialized(FMqasClientId Client) const = 0;
    virtual bool AgentIsConnected(FMqasClientId Client, GSY_ConnectionHwnd Handle) const = 0;
    // Includes this client's connecting and connected retained peers.
    virtual TArray<GSY_PeerId> AgentGetPeerIds(FMqasClientId Client) const = 0;
    virtual std::weak_ptr<BaseConnect> AgentConnectTo(FMqasClientId Client, GSY_EngineId Engine,
        const FString& Config, const FString& Ip, short Port) = 0;
};

struct FMqasClientState;
class MQASNET_API FMqasAgent final : public IMqasAgent
{
public:
    static FMqasAgent& Get();
    FOnErrorDelegate OnError;
    FMqasClientId CreateClient();
    void ReleaseClient(FMqasClientId Client);
    // Thread-safe generation of the SDK runtime, used to reject stale callbacks.
    uint64 GetRuntimeEpoch() const;
    bool AgentIsInitialized(FMqasClientId Client) const override;
    bool AgentIsConnected(FMqasClientId Client, GSY_ConnectionHwnd Handle) const override;
    TArray<GSY_PeerId> AgentGetPeerIds(FMqasClientId Client) const override;
    std::weak_ptr<BaseConnect> AgentConnectTo(FMqasClientId Client, GSY_EngineId Engine,
        const FString& Config, const FString& Ip, short Port) override;
    FString GetConfigPath(FMqasClientId Client, const FString& Config) const;
    void RetainPeer(FMqasClientId Client, const std::shared_ptr<FMqasPeer>& Peer,
        const std::shared_ptr<FMqasDatagram>& Datagram = nullptr);
    TArray<std::shared_ptr<FMqasPeer>> FindPeers(FMqasClientId Client, GSY_PeerId Remote) const;
    bool FindPeer(FMqasClientId Client, GSY_PeerId Remote, std::shared_ptr<FMqasPeer>& OutPeer,
        std::shared_ptr<FMqasDatagram>& OutDatagram) const;
private:
    FMqasAgent();
    ~FMqasAgent();
    FMqasAgent(const FMqasAgent&) = delete;
    struct FImpl;
    std::unique_ptr<FImpl> Impl;
    static void Error(const char*, int);
    static void Connected(int, GSY_ConnectionHwnd);
    static void Disconnected(int, GSY_ConnectionHwnd);
    static void ConnectError(int, GSY_ConnectionHwnd);
    static void StreamOpen(GSY_ConnectionHwnd, GSY_StreamId, ErrorCode);
    static void StreamClose(GSY_ConnectionHwnd, GSY_StreamId, ErrorCode);
    void Dispatch(int Kind, GSY_ConnectionHwnd Handle, GSY_StreamId Stream, int Code);
    void PruneRetired();
};
