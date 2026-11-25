#include "MqasServer.h"
#include "MqasAgent.h"
#include "Async/Async.h"
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

namespace {
std::mutex ServerMutex;
std::unordered_map<GSY_ServerHwnd, std::shared_ptr<FMqasServerState>> Servers;
std::unordered_map<GSY_ConnectionHwnd, std::weak_ptr<FMqasServerState>> Connections;
std::shared_ptr<FMqasServerState> FindServer(GSY_ServerHwnd Handle) {
    std::lock_guard<std::mutex> Lock(ServerMutex);
    auto It = Servers.find(Handle); return It == Servers.end() ? nullptr : It->second;
}
std::shared_ptr<FMqasServerState> FindConnection(GSY_ConnectionHwnd Handle) {
    std::lock_guard<std::mutex> Lock(ServerMutex);
    auto It = Connections.find(Handle); return It == Connections.end() ? nullptr : It->second.lock();
}
}

struct FMqasServerState : std::enable_shared_from_this<FMqasServerState>
{
    std::weak_ptr<FMqasServer> Owner;
    std::atomic<GSY_ServerHwnd> Handle{InvalidServer};
    std::atomic<bool> Started{false}, Terminal{false};
    uint64 Epoch = 0;
    FMqasServer::FAcceptPolicy Accept;
    FMqasServer::FConnectionFactory ConnectionFactory;
    FMqasServer::FStreamOpenHandler StreamOpen;
    GSY_BaseServerContext Context{};
    // Protected by ServerMutex; each SDK connection has its own stable context.
    struct FAcceptedConnection {
        GSY_BaseConnectionContext Context{};
        std::shared_ptr<BaseConnect> Connection;
    };
    std::unordered_map<GSY_ConnectionHwnd, std::shared_ptr<FAcceptedConnection>> Contexts;

    std::shared_ptr<FAcceptedConnection> FindAccepted(GSY_ConnectionHwnd ConnectionHandle) {
        std::lock_guard<std::mutex> Lock(ServerMutex);
        auto It = Contexts.find(ConnectionHandle);
        return It == Contexts.end() ? nullptr : It->second;
    }
    std::unordered_set<GSY_ConnectionHwnd> DisconnectedNotified;

    template<typename F> void Game(F&& Callback) {
        auto Self = shared_from_this();
        AsyncTask(ENamedThreads::GameThread, [Self, Callback = std::forward<F>(Callback)]() mutable {
            if (FMqasAgent::Get().GetRuntimeEpoch() == Self->Epoch) Callback(*Self);
        });
    }
    static void StartedCb(ErrorCode Code, GSY_ServerHwnd Handle) {
        // FindServer waits if the asynchronous SDK callback beats Launch's return.
        auto S = FindServer(Handle); if (!S) return;
        S->Started = Code == EC_Ok;
        S->Game([Code](FMqasServerState& P) { if (auto O = P.Owner.lock()) O->OnStarted.Broadcast(Code); });
    }
    static void StoppedCb(ErrorCode Code, GSY_ServerHwnd Handle) {
        auto S = FindServer(Handle); if (!S || S->Terminal.exchange(true)) return;
        S->Started = false;
        S->Game([Code](FMqasServerState& P) { if (auto O = P.Owner.lock()) O->OnStopped.Broadcast(Code); });
        std::lock_guard<std::mutex> Lock(ServerMutex);
        for (const auto& [Conn, Context] : S->Contexts) Connections.erase(Conn);
        Servers.erase(Handle);
    }
    static void ErrorCb(int Code, unsigned int Handle) {
        // Invalid engine/handle errors can be delivered synchronously by Launch.
        if (Handle == InvalidServer) return;
        auto S = FindServer(Handle); if (!S) return;
        S->Game([Code](FMqasServerState& P) { if (auto O = P.Owner.lock()) O->OnError.Broadcast(static_cast<ErrorCode>(Code)); });
        if (!S->Started && !S->Terminal.exchange(true)) {
            std::lock_guard<std::mutex> Lock(ServerMutex); Servers.erase(Handle);
        }
    }
    static GSY_BaseConnectionContext* AcceptCb(GSY_ServerHwnd Handle, GSY_ConnectionHwnd Conn) {
        auto S = FindServer(Handle); if (!S || S->Terminal) return nullptr;
        try { if (S->Accept && !S->Accept(Conn)) return nullptr; } catch (...) { return nullptr; }
        auto C = std::make_shared<FAcceptedConnection>();
        try { C->Connection = S->ConnectionFactory ? S->ConnectionFactory(Conn) : std::make_shared<BaseConnect>(); }
        catch (...) { return nullptr; }
        if (!C->Connection) return nullptr;
        C->Context.on_connect = ConnectedCb; C->Context.on_disconnect = DisconnectedCb;
        C->Context.on_error = ConnectionErrorCb;
        C->Context.on_stream_open = StreamOpenCb; C->Context.on_stream_close = StreamCloseCb;
        C->Connection->InitData(Conn, 0, FString(), FString(), 0);
        auto* Result = &C->Context;
        std::lock_guard<std::mutex> Lock(ServerMutex);
        S->Contexts.emplace(Conn, C); Connections[Conn] = S;
        return Result;
    }
    static void ConnectedCb(int Code, unsigned int Conn) {
        if (auto S = FindConnection(Conn)) {
            auto Accepted = S->FindAccepted(Conn);
            if (!Accepted) return;
            S->Game([Code, Conn, Accepted](FMqasServerState& P) {
                Accepted->Connection->OnConnected(Code);
                if (auto O = P.Owner.lock()) {
                    if (Code == EC_Ok) O->OnAccepted.Broadcast(Accepted->Connection);
                    O->OnConnection.Broadcast(Conn, static_cast<ErrorCode>(Code));
                }
            });
        }
    }
    static void ConnectionErrorCb(int Code, unsigned int Conn) {
        if (auto S = FindConnection(Conn)) S->Game([Code, Conn](FMqasServerState& P) { if (auto O = P.Owner.lock()) O->OnConnectionError.Broadcast(Conn, static_cast<ErrorCode>(Code)); });
    }
    static void DisconnectedCb(int Code, unsigned int Conn) {
        if (auto S = FindConnection(Conn)) {
            { std::lock_guard<std::mutex> Lock(ServerMutex); S->DisconnectedNotified.insert(Conn); }
            auto Accepted = S->FindAccepted(Conn);
            S->Game([Code, Conn, Accepted](FMqasServerState& P) {
                if (Accepted) Accepted->Connection->OnDisconnect(Code);
                if (auto O = P.Owner.lock()) O->OnDisconnected.Broadcast(Conn, static_cast<ErrorCode>(Code));
            });
        }
    }
    static void ClientDisconnectedCb(GSY_ServerHwnd Handle, GSY_ConnectionHwnd Conn) {
        auto S = FindServer(Handle); if (!S) return;
        std::shared_ptr<FAcceptedConnection> Context;
        bool Notify = false;
        {
            std::lock_guard<std::mutex> Lock(ServerMutex);
            if (auto It = S->Contexts.find(Conn); It != S->Contexts.end()) { Context = std::move(It->second); S->Contexts.erase(It); }
            Notify = S->DisconnectedNotified.erase(Conn) == 0;
            Connections.erase(Conn);
        }
        // Keep the old context alive through this callback and its game-thread
        // dispatch; a recycled connection handle can be registered immediately.
        S->Game([Context = std::move(Context), Notify, Conn](FMqasServerState& P) {
            if (!Notify) return;
            if (Context) Context->Connection->OnDisconnect(EC_Disconnected);
            if (auto O = P.Owner.lock()) O->OnDisconnected.Broadcast(Conn, EC_Disconnected);
        });
    }
    static void StreamOpenCb(GSY_ConnectionHwnd Conn, GSY_StreamId Stream, ErrorCode Code) {
        if (auto S = FindConnection(Conn)) {
            auto Accepted = S->FindAccepted(Conn);
            try {
                if (Accepted) {
                    Accepted->Connection->OnNativeStreamOpen(Conn, Stream, Code);
                    if (S->StreamOpen) S->StreamOpen(Accepted->Connection, Stream, Code);
                }
            }
            catch (...) { UE_LOG(LogTemp, Error, TEXT("Mqas server stream-open handler threw an exception")); }
            S->Game([Conn, Stream, Code](FMqasServerState& P) { if (auto O = P.Owner.lock()) O->OnStreamOpen.Broadcast(Conn, Stream, Code); });
        }
    }
    static void StreamCloseCb(GSY_ConnectionHwnd Conn, GSY_StreamId Stream, ErrorCode Code) {
        if (auto S = FindConnection(Conn)) {
            auto Accepted = S->FindAccepted(Conn);
            if (Accepted) Accepted->Connection->OnNativeStreamClose(Conn, Stream, Code);
            S->Game([Conn, Stream, Code, Accepted](FMqasServerState& P) {
                if (Accepted) Accepted->Connection->OnStreamClose(Stream, Code);
                if (auto O = P.Owner.lock()) O->OnStreamClose.Broadcast(Conn, Stream, Code);
            });
        }
    }
    FMqasServerState() {
        Context.on_started = StartedCb; Context.on_stopped = StoppedCb; Context.on_error = ErrorCb;
        Context.on_client_connect = AcceptCb; Context.on_client_disconnect = ClientDisconnectedCb;
    }
};

FMqasServer::FMqasServer() : State(std::make_shared<FMqasServerState>()) {}
std::shared_ptr<FMqasServer> FMqasServer::Create() { auto Server = std::shared_ptr<FMqasServer>(new FMqasServer()); Server->State->Owner = Server; return Server; }
FMqasServer::~FMqasServer() { if (State->Handle && !State->Terminal) GSY_stop_server(State->Handle); }
bool FMqasServer::SetAcceptPolicy(FAcceptPolicy Policy) { check(IsInGameThread()); if (State->Handle) return false; State->Accept = std::move(Policy); return true; }
bool FMqasServer::SetConnectionFactory(FConnectionFactory Factory) { check(IsInGameThread()); if (State->Handle) return false; State->ConnectionFactory = std::move(Factory); return true; }
bool FMqasServer::SetStreamOpenHandler(FStreamOpenHandler Handler) { check(IsInGameThread()); if (State->Handle) return false; State->StreamOpen = std::move(Handler); return true; }
GSY_ServerHwnd FMqasServer::Launch(GSY_EngineId Engine, const FString& ConfigPath, const GSY_ContextExtension* Options) {
    check(IsInGameThread()); if (State->Handle || State->Terminal || ConfigPath.IsEmpty()) return InvalidServer;
    State->Epoch = FMqasAgent::Get().GetRuntimeEpoch(); State->Context.options = Options;
    // An I/O-thread accept cannot outpace registration; the SDK launch itself is non-blocking.
    std::lock_guard<std::mutex> Lock(ServerMutex);
    const auto Handle = GSY_launch_server(Engine, TCHAR_TO_UTF8(*ConfigPath), &State->Context);
    State->Context.options = nullptr;
    if (Handle) { State->Handle = Handle; Servers.emplace(Handle, State); }
    return Handle;
}
ErrorCode FMqasServer::Stop() { check(IsInGameThread()); return State->Handle && !State->Terminal ? static_cast<ErrorCode>(GSY_stop_server(State->Handle)) : EC_InvalidHandler; }
GSY_ServerHwnd FMqasServer::GetHandle() const { return State->Handle; }
bool FMqasServer::IsStarted() const { return State->Started && !State->Terminal; }
void MqasServerShutdown() {
    std::lock_guard<std::mutex> Lock(ServerMutex);
    for (auto& [Handle, Server] : Servers) { Server->Terminal = true; Server->Started = false; }
    Connections.clear(); Servers.clear();
}
