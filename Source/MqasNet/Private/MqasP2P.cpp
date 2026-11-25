#include "MqasP2P.h"
#include "DirectStreamState.h"
#include "MqasAgent.h"
#include "MqasServer.h"
#include "MqasSocket.h"
#include "Containers/Ticker.h"
#include "Async/Async.h"
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace {
std::mutex RegistryMutex;
std::unordered_map<GSY_StreamId, std::shared_ptr<FMqasDirectStreamState>> Streams;
}

struct FMqasPeerState : std::enable_shared_from_this<FMqasPeerState> {
    std::weak_ptr<FMqasPeer> Owner;
    std::atomic<GSY_ConnectionHwnd> Connection{InvalidConnection};
    std::atomic<bool> Connected{false}, Started{false}, Finished{false};
    bool Server = false;
    uint64 RuntimeEpoch = 0;
    GSY_PeerId Self = INVALID_PID, Other = INVALID_PID;
    FP2PHelperResult Session;
    std::shared_ptr<FMqasServer> Listener;
    FMqasSocketHandle Socket;
    std::weak_ptr<P2PHelperStream> Lobby;
    FDelegateHandle ReadyHandle;
    FTSTicker::FDelegateHandle ReadyTimeout;
    uint64 Client = 0;
    FString Config;

    void StopWaiting() {
        if (auto Stream = Lobby.lock(); Stream && ReadyHandle.IsValid())
            Stream->OnServerReady.Remove(ReadyHandle);
        ReadyHandle.Reset();
        if (ReadyTimeout.IsValid()) FTSTicker::GetCoreTicker().RemoveTicker(ReadyTimeout);
        ReadyTimeout.Reset();
    }

    ErrorCode Connect() {
        StopWaiting();
        auto Peer = Owner.lock();
        if (!Peer) return EC_InvalidHandler;
        std::weak_ptr<BaseConnect> ConnectionObject;
        return FMqasAgent::Get().ConnectSession(Client, Config, Session.Data.session_handle, ConnectionObject, Peer);
    }

    void Finish(ErrorCode Error) {
        if (Finished.exchange(true)) return;
        StopWaiting();
        Socket.Reset();
        auto ServerOwner = std::move(Listener);
        if (ServerOwner && !Connected && ServerOwner->IsStarted())
            GSY_CancelServerP2PSession(ServerOwner->GetHandle(), Session.Data.session_handle);
        const bool WasConnected = Connected.exchange(false);
        if (ServerOwner) ServerOwner->Stop();
        if (!WasConnected) Session.ReleaseSession();
        {
            std::lock_guard Lock(RegistryMutex);
            for (auto& Stream : OwnedStreams) {
                Stream->State->Connected = false;
                if (Stream->GetDirectStreamType() == EMqasDirectStreamType::Datagram)
                    std::static_pointer_cast<FMqasDatagram>(Stream)->ChannelOpen = false;
            }
            std::erase_if(Streams, [this](const auto& Entry) { return Entry.second->Peer.lock().get() == this; });
        }
        if (auto Peer = Owner.lock()) {
            if (WasConnected) Peer->OnClosed.Broadcast(Error);
            else Peer->OnFailed.Broadcast(Error);
        }
    }
    GSY_P2PDatagramConnectionContext DatagramContext{};
    // Protected by RegistryMutex. Retains incoming streams until peer closes.
    std::vector<std::shared_ptr<BaseDirectStream>> OwnedStreams;

    template<class F> void Game(F&& Fn) {
        auto Weak = weak_from_this();
        AsyncTask(ENamedThreads::GameThread, [Weak, Fn = std::forward<F>(Fn)]() mutable {
            if (auto State = Weak.lock(); State && !State->Finished &&
                FMqasAgent::Get().GetRuntimeEpoch() == State->RuntimeEpoch) Fn(*State);
        });
    }
    static std::shared_ptr<FMqasDirectStreamState> FindStream(GSY_StreamId Sid) {
        std::lock_guard<std::mutex> Lock(RegistryMutex);
        auto It = Streams.find(Sid); return It == Streams.end() ? nullptr : It->second;
    }
    static void RegisterStream(const std::shared_ptr<FMqasDirectStreamState>& D, GSY_StreamId Sid) {
        std::lock_guard<std::mutex> Lock(RegistryMutex);
        D->Stream = Sid;
        if (auto Peer = D->Peer.lock()) D->Connection = Peer->Connection.load();
        Streams[Sid] = D;
    }
    static void StreamOpen(const std::shared_ptr<FMqasPeerState>& S, GSY_ConnectionHwnd Conn, GSY_StreamId Sid) {
        if (S->Finished) return;
        S->Connection = Conn;
        // make_stream completion registers locally initiated streams before
        // mqas emits on_new_stream; unregistered streams came from the peer.
        if (FindStream(Sid)) return;
        // A QUIC stream arrives before its application ID is known. Do not
        // assign a preconfigured object by arrival order: streams can reorder.
        auto PendingStream = std::make_shared<FMqasDirectStreamState>();
        PendingStream->Peer = S; PendingStream->Incoming = true;
        RegisterStream(PendingStream, Sid);
        const auto Error = GSY_ServerConnectPeerDatagram(Conn, Sid, &S->DatagramContext, NONE_RID);
        if (Error != EC_Ok) {
            { std::lock_guard<std::mutex> Lock(RegistryMutex); Streams.erase(Sid); }
            S->Game([Sid, Error](FMqasPeerState& P) { if (auto O = P.Owner.lock()) O->OnStreamFailed.Broadcast(Sid, Error); });
        }
    }
    static void DatagramConnectedCb(ErrorCode E, GSY_StreamId Sid, GSY_PeerId, const char*, GSY_RequestId) {
        auto D = FindStream(Sid); auto S = D ? D->Peer.lock() : nullptr; if (!S) return;
        if (E == EC_Ok && D->Incoming) {
            GSY_P2PStreamId Local = 0, Remote = 0;
            const auto Result = GSY_GetPeerStreamIds(S->Connection, Sid, &Local, &Remote);
            if (Result != EC_Ok) {
                S->Game([Sid, Result](FMqasPeerState& P) { if (auto O = P.Owner.lock()) O->OnStreamFailed.Broadcast(Sid, Result); });
                return;
            }
            std::shared_ptr<BaseDirectStream> Target;
            {
                std::lock_guard<std::mutex> Lock(RegistryMutex);
                for (auto& Candidate : S->OwnedStreams) {
                    if (Candidate->State->LocalId == Local && !Candidate->State->Opened.exchange(true)) { Target = Candidate; break; }
                }
                if (!Target) {
                    Target = FMqasDatagram::Create(); Target->State->Peer = S; Target->State->LocalId = Local;
                    Target->State->Opened = true; S->OwnedStreams.push_back(Target);
                }
                D = Target->State; D->Incoming = true; D->Stream = Sid; D->Connection = S->Connection.load(); Streams[Sid] = D;
            }
        }
        D->Connected = E == EC_Ok;
        S->Game([D, E](FMqasPeerState& P) { if (auto O = D->Owner.lock()) {
            if (E == EC_Ok) {
                if (D->Incoming) {
                    auto Peer = P.Owner.lock();
                    if (!Peer || !Peer->IsConnected() || !Peer->AttachStream(D->Stream, O)) return;
                }
                if (D->Incoming) if (auto Peer = P.Owner.lock()) Peer->OnIncomingStream.Broadcast(O);
                O->OnConnected.Broadcast();
            } else O->OnFailed.Broadcast(E);
        } });
    }
    static void DatagramChannelResult(GSY_StreamId Sid, int Opening, ErrorCode E, GSY_RequestId RequestId, const char* Reason) {
        auto D = FindStream(Sid); auto S = D ? D->Peer.lock() : nullptr; if (!S) return;
        if (E == EC_Ok) if (auto O = D->Owner.lock()) std::static_pointer_cast<FMqasDatagram>(O)->ChannelOpen = Opening != 0;
        const FString Message = Reason ? UTF8_TO_TCHAR(Reason) : TEXT("");
        S->Game([D, Opening, E, RequestId, Message](FMqasPeerState&) { if (auto Base = D->Owner.lock()) {
            auto O = std::static_pointer_cast<FMqasDatagram>(Base);
            O->OnChannelResult.Broadcast(Opening != 0, E, RequestId, Message);
            if (E != EC_Ok) O->OnFailed.Broadcast(E);
            else if (Opening) O->OnChannelOpened.Broadcast(); else O->OnChannelClosed.Broadcast();
        } });
    }
    static void DatagramReceived(const uint8_t* Data, size_t Size, GSY_StreamId Sid, GSY_PeerId, GSY_ConnectionHwnd) {
        auto D = FindStream(Sid); auto S = D ? D->Peer.lock() : nullptr;
        if (!S || (!Data && Size) || Size > MAX_int32) return;
        TArray<uint8> Copy; if (Size) Copy.Append(Data, static_cast<int32>(Size));
        S->Game([D, Copy = MoveTemp(Copy)](FMqasPeerState&) { if (auto O = D->Owner.lock()) std::static_pointer_cast<FMqasDatagram>(O)->OnReceived.Broadcast(Copy); });
    }
    static void DatagramDisconnected(ErrorCode, GSY_StreamId Sid, GSY_PeerId, const char* Reason) {
        auto D = FindStream(Sid); auto S = D ? D->Peer.lock() : nullptr; if (!S) return;
        D->Connected = false; if (auto O = D->Owner.lock()) std::static_pointer_cast<FMqasDatagram>(O)->ChannelOpen = false;
        auto Owner = D->Owner.lock();
        {
            std::lock_guard<std::mutex> Lock(RegistryMutex);
            if (auto It = Streams.find(Sid); It != Streams.end() && It->second == D) Streams.erase(It);
            std::erase_if(S->OwnedStreams, [&D](const auto& Item) { return Item->State == D; });
        }
        FString Copy = Reason ? UTF8_TO_TCHAR(Reason) : TEXT("");
        S->Game([Owner = std::move(Owner), Copy = MoveTemp(Copy)](FMqasPeerState&) { if (Owner) { Owner->OnClose(); Owner->OnDisconnected.Broadcast(Copy); } });
    }
    static void DatagramFailed(ErrorCode E, GSY_StreamId Sid, GSY_PeerId, GSY_RequestId) {
        auto D = FindStream(Sid); auto S = D ? D->Peer.lock() : nullptr;
        if (S) S->Game([D, Sid, E](FMqasPeerState& P) {
            if (auto O = D->Owner.lock()) O->OnFailed.Broadcast(E);
            else if (auto Peer = P.Owner.lock()) Peer->OnStreamFailed.Broadcast(Sid, E);
        });
    }
    static void DatagramError(GSY_ConnectionHwnd, GSY_StreamId Sid, ErrorCode E, const char*, GSY_RequestId) { DatagramFailed(E,Sid,0,0); }
    static void DatagramRejected(GSY_StreamId Sid, const char* Reason, GSY_RequestId RequestId) {
        auto D = FindStream(Sid);
        auto S = D ? D->Peer.lock() : nullptr;
        if (!S) return;
        const FString Message = Reason ? UTF8_TO_TCHAR(Reason) : TEXT("");
        S->Game([D, Message, RequestId](FMqasPeerState&) {
            if (auto O = D->Owner.lock())
                std::static_pointer_cast<FMqasDatagram>(O)->OnRejected.Broadcast(Message, RequestId);
        });
    }

    FMqasPeerState() {
        DatagramContext.on_connected = DatagramConnectedCb; DatagramContext.on_datagram_received = DatagramReceived;
        DatagramContext.on_disconnected = DatagramDisconnected; DatagramContext.on_connect_failed = DatagramFailed;
        DatagramContext.on_stream_rejected = DatagramRejected;
        DatagramContext.on_error = DatagramError; DatagramContext.on_channel_result = DatagramChannelResult;
    }
};

FMqasPeer::FMqasPeer() : State(std::make_shared<FMqasPeerState>()) {}
std::shared_ptr<FMqasPeer> FMqasPeer::Create() { auto P = std::shared_ptr<FMqasPeer>(new FMqasPeer()); P->State->Owner = P; return P; }
FMqasPeer::~FMqasPeer() = default;
bool FMqasPeer::RegisterStream(const std::shared_ptr<BaseDirectStream>& D, GSY_P2PStreamId LocalStreamId) {
    check(IsInGameThread()); if (!D || !LocalStreamId || State->Finished || !D->State->Peer.expired()) return false;
    std::lock_guard<std::mutex> Lock(RegistryMutex);
    for (const auto& Existing : State->OwnedStreams) if (Existing->State->LocalId == LocalStreamId) return false;
    D->State->LocalId = LocalStreamId; D->State->Peer = State; D->State->Owner = D; State->OwnedStreams.push_back(D); return true;
}
ErrorCode FMqasPeer::Start(uint64 Client, const FString& Config, const std::shared_ptr<P2PHelperStream>& Lobby,
    const FP2PHelperResult& Result, GSY_P2PStreamPolicy StreamPolicy,
    decltype(GSY_P2PDatagramConnectionContext::on_channel_request) ChannelPolicy)
{
    check(IsInGameThread());
    if (!Lobby || !Lobby->Valid() || Result.Data.ret != EC_Ok || Result.Data.use_relay ||
        !Result.Data.session_handle || State->Started.exchange(true)) return EC_Fail;
    State->RuntimeEpoch = FMqasAgent::Get().GetRuntimeEpoch();
    State->Session = Result;
    State->Self = Lobby->GetPeerId();
    State->Other = Result.Data.peer_id;
    State->Server = Result.Data.is_server != 0;
    State->DatagramContext.self_id = State->Self;
    State->DatagramContext.is_server = State->Server;
    State->DatagramContext.on_channel_request = ChannelPolicy;
    State->Client = Client;
    State->Config = Config;
    State->Lobby = Lobby;
    std::weak_ptr<FMqasPeerState> Weak = State;
    OnConnectedDelegate.AddLambda([Weak](int Code, GSY_ConnectionHwnd Handle) {
        auto S = Weak.lock();
        if (!S || S->Finished) return;
        GSY_PeerId Remote = INVALID_PID;
        if (Code != EC_Ok || GSY_GetConnectionP2PPeerId(Handle, &Remote) != EC_Ok || Remote != S->Other) {
            if (auto Peer = S->Owner.lock()) Peer->Close();
            return;
        }
        S->Connection = Handle;
        S->Connected = true;
        if (auto Peer = S->Owner.lock()) Peer->OnConnected.Broadcast();
    });
    const auto Terminal = [Weak](int Code, GSY_ConnectionHwnd) {
        if (auto S = Weak.lock()) S->Finish(static_cast<ErrorCode>(Code));
    };
    OnDisconnectedDelegate.AddLambda(Terminal);
    OnConnectErrorDelegate.AddLambda(Terminal);
    if (!State->Server) {
        State->ReadyHandle = Lobby->OnServerReady.AddLambda([Weak](GSY_PeerId Peer, GSY_SessionHandle Session) {
            auto S = Weak.lock();
            if (!S || S->Other != Peer || S->Session.Data.session_handle != Session) return;
            const auto Code = S->Connect();
            if (Code != EC_Pending) S->Finish(Code);
        });
        State->ReadyTimeout = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Weak](float) {
            if (auto S = Weak.lock()) {
                S->ReadyTimeout.Reset();
                S->Finish(EC_Timeout);
            }
            return false;
        }), 15.f);
        return EC_Pending;
    }

    const FString Path = FMqasAgent::Get().GetConfigPath(Client, Config);
    if (Path.IsEmpty()) return EC_Fail;
    const auto SocketResult = State->Socket.Acquire(Lobby->GetHwnd());
    if (SocketResult != EC_Ok) return SocketResult;
    auto Server = FMqasServer::Create();
    std::weak_ptr<BaseConnect> WeakPeer = weak_from_this();
    Server->SetConnectionFactory([WeakPeer](GSY_ConnectionHwnd) { return WeakPeer.lock(); });
    Server->OnStarted.AddLambda([Weak](ErrorCode Code) {
        auto S = Weak.lock();
        if (!S || S->Finished) return;
        S->Socket.Reset();
        if (Code == EC_Ok)
            Code = GSY_RegisterServerP2PSession(S->Listener->GetHandle(), S->Session.Data.session_handle);
        if (Code == EC_Ok) {
            auto LobbyStream = S->Lobby.lock();
            Code = LobbyStream ? LobbyStream->NotifyP2PServerReady(S->Session.Data.session_handle) : EC_InvalidStream;
            if (Code == EC_Pending) return;
        }
        S->Finish(Code);
    });
    Server->OnError.AddLambda([Weak](ErrorCode Code) { if (auto S = Weak.lock()) S->Finish(Code); });
    Server->OnStopped.AddLambda([Weak](ErrorCode) { if (auto S = Weak.lock()) S->Finish(EC_Disconnected); });
    const GSY_P2PServerOptions Options{{GSY_CONTEXT_P2P_SERVER, sizeof(GSY_P2PServerOptions)},
        State->Socket.Get(), StreamPolicy};
    State->Listener = Server;
    if (!Server->Launch(2, Path, &Options.header)) {
        State->Listener.reset();
        State->Socket.Reset();
        return EC_Fail;
    }
    return EC_Pending;
}

void FMqasPeer::OnNativeStreamOpen(GSY_ConnectionHwnd Connection, GSY_StreamId Id, ErrorCode Code)
{
    if (Code == EC_Ok) FMqasPeerState::StreamOpen(State, Connection, Id);
}

void FMqasPeer::OnNativeStreamClose(GSY_ConnectionHwnd, GSY_StreamId Id, ErrorCode)
{
    std::lock_guard Lock(RegistryMutex);
    const auto It = Streams.find(Id);
    if (It == Streams.end() || It->second->Peer.lock() != State) return;
    It->second->Connected = false;
    if (auto Stream = It->second->Owner.lock()) std::static_pointer_cast<FMqasDatagram>(Stream)->ChannelOpen = false;
    const auto StreamState = It->second;
    std::erase_if(State->OwnedStreams, [&StreamState](const auto& Stream) { return Stream->State == StreamState; });
    Streams.erase(It);
}

void FMqasPeer::Close()
{
    if (!State->Started || State->Finished) return;
    if (GetHwnd() != InvalidConnection) BaseConnect::Disconnect();
    else State->Finish(EC_Disconnected);
}

void FMqasPeer::Disconnect() { Close(); }
GSY_PeerId FMqasPeer::GetRemotePeerId() const { return State->Other; }
GSY_ConnectionHwnd FMqasPeer::GetConnection() const { return GetHwnd(); }
ErrorCode FMqasPeer::FindStream(GSY_P2PStreamId RemoteStreamId, GSY_StreamId& OutStream) const {
    check(IsInGameThread()); OutStream = INVALID_SID;
    return State->Connected ? GSY_FindPeerStream(State->Connection, RemoteStreamId, &OutStream) : EC_Fail;
}
std::shared_ptr<BaseDirectStream> FMqasPeer::FindDirectStream(GSY_P2PStreamId RemoteStreamId) const {
    check(IsInGameThread());
    GSY_StreamId Sid = INVALID_SID;
    if (FindStream(RemoteStreamId, Sid) != EC_Ok) return {};
    auto D = FMqasPeerState::FindStream(Sid);
    return D && D->Peer.lock() == State ? D->Owner.lock() : nullptr;
}
std::shared_ptr<BaseDirectStream> FMqasPeer::FindLocalStream(GSY_P2PStreamId LocalStreamId) const {
    check(IsInGameThread());
    std::lock_guard<std::mutex> Lock(RegistryMutex);
    for (const auto& Stream : State->OwnedStreams) if (Stream->State->LocalId == LocalStreamId) return Stream;
    return {};
}
std::shared_ptr<FMqasDatagram> FMqasDatagram::Create() { auto D = std::shared_ptr<FMqasDatagram>(new FMqasDatagram()); D->State->Owner = D; return D; }
FMqasDatagram::~FMqasDatagram() = default;
GSY_P2PDatagramConnectionContext* FMqasDatagram::GetContext()
{
    auto Peer = State->Peer.lock();
    return Peer ? &Peer->DatagramContext : nullptr;
}

ErrorCode FMqasDatagram::Open(GSY_RequestId RequestId)
{
    check(IsInGameThread());
    auto PeerState = State->Peer.lock();
    auto Peer = PeerState ? PeerState->Owner.lock() : nullptr;
    auto Stream = std::static_pointer_cast<FMqasDatagram>(State->Owner.lock());
    if (!Peer || !Stream || !Peer->IsConnected() || State->Opened.exchange(true)) return EC_Fail;
    const auto Code = Peer->MakeStream(Stream,
        [LocalId = State->LocalId, RequestId](GSY_ConnectionHwnd Connection, GSY_P2PDatagramConnectionContext* Context,
            BaseConnect::FStreamCreated Created, void* User) {
            return GSY_MakePeerDatagramStream(Connection, LocalId, Context, RequestId, Created, User);
        },
        [Stream](ErrorCode Result, GSY_StreamId, GSY_RequestId) {
            if (Result != EC_Ok) {
                Stream->State->Opened = false;
                Stream->OnFailed.Broadcast(Result);
            }
        },
        [StreamState = State, PeerState](ErrorCode Result, GSY_StreamId Id, GSY_RequestId) {
            if (Result == EC_Ok && !PeerState->Finished) FMqasPeerState::RegisterStream(StreamState, Id);
        });
    if (Code != EC_Pending) State->Opened = false;
    return Code;
}
ErrorCode FMqasDatagram::Send(const TArray<uint8>& Data, GSY_RequestId RequestId) {
    check(IsInGameThread()); auto S = State->Peer.lock(); if (!S || !ChannelOpen || Data.IsEmpty()) return EC_Fail;
    return GSY_SendDatagram(S->Connection, State->Stream, reinterpret_cast<const char*>(Data.GetData()), Data.Num(), RequestId);
}
ErrorCode FMqasDatagram::Disconnect(const FString& Reason, GSY_RequestId RequestId) {
    check(IsInGameThread()); auto S = State->Peer.lock(); if (!S || !State->Connected) return EC_Fail;
    return GSY_ReqDisconnectPeerDatagram(S->Connection, State->Stream, TCHAR_TO_UTF8(*Reason), RequestId);
}
ErrorCode FMqasDatagram::OpenChannel(GSY_RequestId RequestId) {
    check(IsInGameThread()); auto S = State->Peer.lock(); if (!S || !State->Connected) return EC_Fail;
    return GSY_OpenDatagramChannel(S->Connection, State->Stream, RequestId);
}
ErrorCode FMqasDatagram::CloseChannel(GSY_RequestId RequestId) {
    check(IsInGameThread()); auto S = State->Peer.lock(); if (!S || !State->Connected) return EC_Fail;
    return GSY_CloseDatagramChannel(S->Connection, State->Stream, RequestId);
}
bool FMqasDatagram::IsChannelOpen() const { return ChannelOpen; }
void MqasP2PShutdown() { std::lock_guard<std::mutex> Lock(RegistryMutex); Streams.clear(); }
