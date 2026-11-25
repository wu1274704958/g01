#pragma once

#include "CoreMinimal.h"
#include "gsync/core/server.h"
#include <functional>
#include <memory>

struct FMqasServerState;
class BaseConnect;

// The policies run synchronously on the SDK I/O thread. Set them before Launch;
// they must be thread-safe, non-blocking and must not access Unreal objects.
// All delegates and other methods are used on the game thread.
class MQASNET_API FMqasServer : public std::enable_shared_from_this<FMqasServer>
{
public:
    using FAcceptPolicy = std::function<bool(GSY_ConnectionHwnd)>;
    // Called on the SDK I/O thread, before any callbacks for this connection.
    // The factory must be thread-safe and must not access Unreal objects.
    using FConnectionFactory = std::function<std::shared_ptr<BaseConnect>(GSY_ConnectionHwnd)>;
    using FStreamOpenHandler = std::function<void(const std::shared_ptr<BaseConnect>&, GSY_StreamId, ErrorCode)>;
    static std::shared_ptr<FMqasServer> Create();
    ~FMqasServer();
    bool SetAcceptPolicy(FAcceptPolicy Policy);
    bool SetConnectionFactory(FConnectionFactory Factory);
    bool SetStreamOpenHandler(FStreamOpenHandler Handler);
    GSY_ServerHwnd Launch(GSY_EngineId Engine, const FString& ConfigPath,
        const GSY_ContextExtension* Options = nullptr);
    ErrorCode Stop();
    GSY_ServerHwnd GetHandle() const;
    bool IsStarted() const;

    DECLARE_MULTICAST_DELEGATE_OneParam(FServerEvent, ErrorCode);
    DECLARE_MULTICAST_DELEGATE_TwoParams(FConnectionEvent, GSY_ConnectionHwnd, ErrorCode);
    DECLARE_MULTICAST_DELEGATE_ThreeParams(FStreamEvent, GSY_ConnectionHwnd, GSY_StreamId, ErrorCode);
    DECLARE_MULTICAST_DELEGATE_OneParam(FAccepted, const std::shared_ptr<BaseConnect>&);
    FAccepted OnAccepted;
    FServerEvent OnStarted, OnStopped, OnError;
    FConnectionEvent OnConnection, OnConnectionError, OnDisconnected;
    FStreamEvent OnStreamOpen, OnStreamClose;
private:
    FMqasServer();
    std::shared_ptr<FMqasServerState> State;
    friend struct FMqasServerState;
};

// Called only after SDK termination has drained callbacks.
void MQASNET_API MqasServerShutdown();
