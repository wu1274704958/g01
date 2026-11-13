#include "MqasP2P.h"
#include "Async/Async.h"
#include <atomic>
#include <mutex>
#include <unordered_map>

namespace {
std::mutex RegistryMutex;
std::unordered_map<FMqasPeerState*, std::shared_ptr<FMqasPeerState>> Peers;
std::unordered_map<GSY_StreamId, std::weak_ptr<FMqasPeerState>> Streams;
std::shared_ptr<FMqasPeerState> FindPeer(FMqasPeerState* Ptr) {
    std::lock_guard<std::mutex> Lock(RegistryMutex);
    auto It = Peers.find(Ptr); return It == Peers.end() ? nullptr : It->second;
}
std::shared_ptr<FMqasPeerState> FindStream(GSY_StreamId Sid) {
    std::lock_guard<std::mutex> Lock(RegistryMutex);
    auto It = Streams.find(Sid); return It == Streams.end() ? nullptr : It->second.lock();
}
}

struct FMqasPeerState : std::enable_shared_from_this<FMqasPeerState> {
    std::weak_ptr<FMqasPeer> Owner;
    std::weak_ptr<FMqasDatagram> Datagram;
    std::atomic<GSY_P2PPeerHandle> Handle{0};
    std::atomic<GSY_ConnectionHwnd> Connection{InvalidConnection};
    std::atomic<GSY_StreamId> Stream{INVALID_SID};
    std::atomic<bool> Connected{false}, DatagramConnected{false}, Started{false}, Opened{false}, Finished{false};
    bool Server = false;
    GSY_PeerId Self = INVALID_PID, Other = INVALID_PID;
    FP2PHelperResult Session;
    GSY_P2PPeerContext PeerContext{};
    GSY_P2PDatagramConnectionContext DatagramContext{};

    template<class F> void Game(F&& Fn) {
        auto Weak = weak_from_this();
        AsyncTask(ENamedThreads::GameThread, [Weak, Fn = std::forward<F>(Fn)]() mutable {
            if (auto State = Weak.lock()) Fn(*State);
        });
    }
    void RegisterStream(GSY_StreamId Sid) {
        Stream = Sid;
        std::lock_guard<std::mutex> Lock(RegistryMutex);
        Streams[Sid] = shared_from_this();
    }
    static void PeerConnected(GSY_P2PPeerHandle Handle, GSY_ConnectionHwnd Conn, void* User) {
        auto S = FindPeer(static_cast<FMqasPeerState*>(User)); if (!S) return;
        S->Handle = Handle; S->Connection = Conn; S->Connected = true;
        S->Game([](FMqasPeerState& P) { if (auto O = P.Owner.lock()) O->OnConnected.Broadcast(); });
    }
    static void StreamOpen(GSY_P2PPeerHandle, GSY_ConnectionHwnd Conn, GSY_StreamId Sid, void* User) {
        auto S = FindPeer(static_cast<FMqasPeerState*>(User)); if (!S || S->Finished) return;
        if (S->Stream != INVALID_SID && S->Stream != Sid) {
            // The wrapper supports one datagram stream per peer.
            GSY_CloseP2PPeer(S->Handle);
            return;
        }
        S->RegisterStream(Sid);
        if (S->Server) {
            const auto Error = GSY_ServerConnectPeerDatagram(Conn, Sid, &S->DatagramContext, NONE_RID);
            if (Error != EC_Ok) S->Game([Error](FMqasPeerState& P) { if (auto D = P.Datagram.lock()) D->OnFailed.Broadcast(Error); });
        }
    }
    static void Terminal(GSY_P2PPeerHandle, ErrorCode Error, void* User, bool WasConnected) {
        auto S = FindPeer(static_cast<FMqasPeerState*>(User)); if (!S || S->Finished.exchange(true)) return;
        S->Connected = false; S->DatagramConnected = false;
        // Keep state alive through the dispatched terminal event.
        AsyncTask(ENamedThreads::GameThread, [S, Error, WasConnected] {
            if (auto O = S->Owner.lock()) { if (WasConnected) O->OnClosed.Broadcast(Error); else O->OnFailed.Broadcast(Error); }
            else if (!WasConnected) S->Session.ReleaseSession();
        });
        std::lock_guard<std::mutex> Lock(RegistryMutex);
        for (auto It = Streams.begin(); It != Streams.end();) {
            if (It->second.lock() == S) It = Streams.erase(It); else ++It;
        }
        Peers.erase(S.get());
    }
    static void Failed(GSY_P2PPeerHandle H, ErrorCode E, void* U) { Terminal(H,E,U,false); }
    static void Closed(GSY_P2PPeerHandle H, ErrorCode E, void* U) { Terminal(H,E,U,true); }
    static void DatagramConnectedCb(ErrorCode E, GSY_StreamId Sid, GSY_PeerId, const char*, GSY_RequestId) {
        if (auto S = FindStream(Sid)) { S->DatagramConnected = E == EC_Ok; S->Game([E](FMqasPeerState& P) { if (auto D = P.Datagram.lock()) { if (E == EC_Ok) D->OnConnected.Broadcast(); else D->OnFailed.Broadcast(E); } }); }
    }
    static void DatagramReceived(const uint8_t* Data, size_t Size, GSY_StreamId Sid, GSY_PeerId, GSY_ConnectionHwnd) {
        auto S = FindStream(Sid); if (!S || (!Data && Size) || Size > MAX_int32) return;
        TArray<uint8> Copy; if (Size) Copy.Append(Data, static_cast<int32>(Size));
        S->Game([Copy = MoveTemp(Copy)](FMqasPeerState& P) { if (auto D = P.Datagram.lock()) D->OnReceived.Broadcast(Copy); });
    }
    static void DatagramDisconnected(ErrorCode, GSY_StreamId Sid, GSY_PeerId, const char* Reason) {
        auto S = FindStream(Sid); if (!S) return; S->DatagramConnected = false;
        FString Copy = Reason ? UTF8_TO_TCHAR(Reason) : TEXT("");
        S->Game([Copy = MoveTemp(Copy)](FMqasPeerState& P) { if (auto D = P.Datagram.lock()) D->OnDisconnected.Broadcast(Copy); });
    }
    static void DatagramFailed(ErrorCode E, GSY_StreamId Sid, GSY_PeerId, GSY_RequestId) {
        if (auto S = FindStream(Sid)) S->Game([E](FMqasPeerState& P) { if (auto D = P.Datagram.lock()) D->OnFailed.Broadcast(E); });
    }
    static void DatagramError(GSY_ConnectionHwnd, GSY_StreamId Sid, ErrorCode E, const char*, GSY_RequestId) { DatagramFailed(E,Sid,0,0); }
    static void StreamCreated(ErrorCode E, GSY_StreamId Sid, GSY_RequestId, void* User) {
        auto S = FindPeer(static_cast<FMqasPeerState*>(User)); if (!S) return;
        if (E == EC_Ok) S->RegisterStream(Sid);
        else S->Game([E](FMqasPeerState& P) { if (auto D = P.Datagram.lock()) D->OnFailed.Broadcast(E); });
    }
    FMqasPeerState() {
        PeerContext.on_connected = PeerConnected; PeerContext.on_stream_open = StreamOpen;
        PeerContext.on_failed = Failed; PeerContext.on_closed = Closed; PeerContext.user_data = this;
        DatagramContext.on_connected = DatagramConnectedCb; DatagramContext.on_datagram_received = DatagramReceived;
        DatagramContext.on_disconnected = DatagramDisconnected; DatagramContext.on_connect_failed = DatagramFailed;
        DatagramContext.on_error = DatagramError;
    }
};

FMqasPeer::FMqasPeer() : State(std::make_shared<FMqasPeerState>()) {}
std::shared_ptr<FMqasPeer> FMqasPeer::Create() { auto P = std::shared_ptr<FMqasPeer>(new FMqasPeer()); P->State->Owner = P; return P; }
FMqasPeer::~FMqasPeer() { Close(); }
bool FMqasPeer::SetDatagram(const std::shared_ptr<FMqasDatagram>& D) {
    check(IsInGameThread()); if (!D || State->Finished || (State->Started && State->Server) || !State->Datagram.expired() || !D->State.expired()) return false;
    State->Datagram = D; D->State = State; return true;
}
ErrorCode FMqasPeer::Start(const FString& ConfigPath, GSY_PeerId Self, const FP2PHelperResult& Result) {
    check(IsInGameThread()); if (State->Started.exchange(true)) return EC_Fail;
    State->Session = Result; State->Self = Self; State->Other = Result.Data.peer_id; State->Server = Result.Data.is_server != 0;
    State->DatagramContext.self_id = Self; State->DatagramContext.is_server = State->Server;
    { std::lock_guard<std::mutex> Lock(RegistryMutex); Peers.emplace(State.get(), State); }
    auto Native = Result.Native(); GSY_P2PPeerHandle Handle = 0;
    const auto Error = GSY_StartP2PPeer(TCHAR_TO_UTF8(*ConfigPath), Self, &Native, &State->PeerContext, &Handle);
    if (Error != EC_Pending) { std::lock_guard<std::mutex> Lock(RegistryMutex); Peers.erase(State.get()); State->Finished = true; return Error; }
    State->Handle = Handle; return Error;
}
void FMqasPeer::Close() { if (State && State->Handle && !State->Finished) GSY_CloseP2PPeer(State->Handle); }
bool FMqasPeer::IsConnected() const { return State->Connected; }
GSY_PeerId FMqasPeer::GetRemotePeerId() const { return State->Other; }
GSY_ConnectionHwnd FMqasPeer::GetConnection() const { return State->Connection; }
std::shared_ptr<FMqasDatagram> FMqasDatagram::Create() { return std::shared_ptr<FMqasDatagram>(new FMqasDatagram()); }
FMqasDatagram::~FMqasDatagram() = default;
ErrorCode FMqasDatagram::Open(GSY_RequestId RequestId) {
    check(IsInGameThread()); auto S = State.lock();
    if (!S || S->Server || !S->Connected || S->Opened.exchange(true)) return EC_Fail;
    const auto E = GSY_ConnectPeerDatagram(S->Connection, S->Other, &S->DatagramContext, RequestId, FMqasPeerState::StreamCreated, S.get());
    if (E != EC_Pending) S->Opened = false;
    return E;
}
ErrorCode FMqasDatagram::Send(const TArray<uint8>& Data, GSY_RequestId RequestId) {
    check(IsInGameThread()); auto S = State.lock(); if (!S || !S->DatagramConnected || Data.IsEmpty()) return EC_Fail;
    return GSY_SendDatagram(S->Connection, S->Stream, reinterpret_cast<const char*>(Data.GetData()), Data.Num(), RequestId);
}
ErrorCode FMqasDatagram::Disconnect(const FString& Reason, GSY_RequestId RequestId) {
    check(IsInGameThread()); auto S = State.lock(); if (!S || !S->DatagramConnected) return EC_Fail;
    return GSY_ReqDisconnectPeerDatagram(S->Connection, S->Stream, TCHAR_TO_UTF8(*Reason), RequestId);
}
bool FMqasDatagram::IsConnected() const { auto S = State.lock(); return S && S->DatagramConnected; }
void MqasP2PShutdown() { std::lock_guard<std::mutex> Lock(RegistryMutex); Streams.clear(); Peers.clear(); }
