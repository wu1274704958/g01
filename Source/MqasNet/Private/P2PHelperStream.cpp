#include "P2PHelperStream.h"
#include "MqasAgent.h"
#include "Async/Async.h"
#include <utility>

namespace {
template <typename F>
void DispatchLobbyCallback(F&& Callback)
{
    const uint64 Epoch = FMqasAgent::Get().GetRuntimeEpoch();
    AsyncTask(ENamedThreads::GameThread, [Epoch, Callback = std::forward<F>(Callback)]() mutable {
        if (FMqasAgent::Get().GetRuntimeEpoch() == Epoch) Callback();
    });
}
}

FP2PHelperResult::FP2PHelperResult(const GSY_HelperResult& Result) : Data(Result),
    Reason(Result.reason ? Result.reason : ""), RelayToken(Result.relay_token ? Result.relay_token : "")
{
    if (Result.verify_token && Result.verify_token_len)
        VerifyToken.assign(Result.verify_token, Result.verify_token + Result.verify_token_len);
    Data.reason = nullptr; Data.relay_token = nullptr; Data.verify_token = nullptr;
}
GSY_HelperResult FP2PHelperResult::Native() const
{
    auto Copy = Data;
    Copy.reason = Reason.c_str(); Copy.relay_token = RelayToken.c_str();
    Copy.verify_token = VerifyToken.empty() ? nullptr : VerifyToken.data();
    Copy.verify_token_len = VerifyToken.size();
    return Copy;
}

ErrorCode FP2PHelperResult::ReleaseSession() const
{
    return GSY_ReleaseP2PSession(Data.session_handle);
}


P2PHelperStream::P2PHelperStream() : BaseStream()
{
	_Context.extend = this;
	_Context.on_registration_success    = Cb_OnRegistrationSuccess;
	_Context.on_unregister              = Cb_OnUnregister;
	_Context.on_connect_responds        = Cb_OnConnectResponds;
	_Context.on_receive_peer_list       = Cb_OnReceivePeerList;
	_Context.on_peer_req_connect        = Cb_OnPeerReqConnect;
	_Context.on_error                   = Cb_OnError;
	_Context.on_change_to_helper_result = Cb_OnChangeToHelperResult;
	_Context.on_attempt_connect         = Cb_OnAttemptConnect;
	_Context.on_helper_quit_result      = Cb_OnHelperQuitResult;
}

P2PHelperStream::~P2PHelperStream()
{
	if (auto It = s_Registry.find(_streamId); It != s_Registry.end() && It->second == this)
		s_Registry.erase(It);
}

void P2PHelperStream::InitData(GSY_StreamId streamId, GSY_ConnectionHwnd hwnd)
{
	BaseStream::InitData(streamId, hwnd);
	s_Registry[streamId] = this;
	_bIsRegistering = true;
}

void P2PHelperStream::OnClose()
{
	if (auto It = s_Registry.find(_streamId); It != s_Registry.end() && It->second == this)
		s_Registry.erase(It);
	_bIsRegistering = false;
	_bIsRegistered = false;
	_PeerId = INVALID_PID;
	BaseStream::OnClose();
}

P2PHelperStream* P2PHelperStream::GetInstance(GSY_StreamId sid)
{
	auto it = s_Registry.find(sid);
	return it != s_Registry.end() ? it->second : nullptr;
}

void P2PHelperStream::Cb_OnRegistrationSuccess(ErrorCode err, GSY_StreamId sid, GSY_PeerId peerId)
{
    DispatchLobbyCallback([err,sid,peerId] { if (auto* self = GetInstance(sid)) self->OnRegistrationSuccess(err,sid,peerId); });
}
void P2PHelperStream::Cb_OnUnregister(ErrorCode err, GSY_StreamId sid)
{
    DispatchLobbyCallback([err,sid] { if (auto* self = GetInstance(sid)) self->OnUnregister(err,sid); });
}
void P2PHelperStream::Cb_OnConnectResponds(ErrorCode err, GSY_PeerId otherPeer, GSY_StreamId sid)
{
    DispatchLobbyCallback([err,otherPeer,sid] { if (auto* self = GetInstance(sid)) self->OnConnectResponds(err,otherPeer,sid); });
}
void P2PHelperStream::Cb_OnReceivePeerList(GSY_PeerData* list, uint32_t size, GSY_StreamId sid)
{
    TArray<TPair<GSY_PeerId, FString>> Peers;
    for (uint32_t i = 0; list && i < size; ++i) Peers.Emplace(list[i].peer_id, list[i].name ? UTF8_TO_TCHAR(list[i].name) : TEXT(""));
    DispatchLobbyCallback([sid, Peers = MoveTemp(Peers)] {
        if (auto* self = GetInstance(sid)) {
            TArray<std::string> Names; TArray<GSY_PeerData> Native;
            Names.Reserve(Peers.Num()); Native.Reserve(Peers.Num());
            for (const auto& Peer : Peers) Names.Emplace(TCHAR_TO_UTF8(*Peer.Value));
            for (int32 i = 0; i < Peers.Num(); ++i) Native.Add({Peers[i].Key, Names[i].c_str()});
            self->OnReceivePeerList(Native.GetData(), Native.Num(), sid);
        }
    });
}
void P2PHelperStream::Cb_OnPeerReqConnect(GSY_PeerData* peer, GSY_StreamId sid)
{
    if (!peer) return;
    const auto Id = peer->peer_id; FString Name = peer->name ? UTF8_TO_TCHAR(peer->name) : TEXT("");
    DispatchLobbyCallback([sid, Id, Name = MoveTemp(Name)] {
        if (auto* self = GetInstance(sid)) { FTCHARToUTF8 Utf8(*Name); GSY_PeerData Copy{Id, Utf8.Get()}; self->OnPeerReqConnect(&Copy,sid); }
    });
}
void P2PHelperStream::Cb_OnError(GSY_ConnectionHwnd conn, GSY_StreamId sid, ErrorCode err, const char* msg, GSY_RequestId reqId)
{
    FString Message = msg ? UTF8_TO_TCHAR(msg) : TEXT("");
    DispatchLobbyCallback([conn,sid,err,reqId,Message = MoveTemp(Message)] { if (auto* self = GetInstance(sid)) self->OnError(conn,sid,err,TCHAR_TO_UTF8(*Message),reqId); });
}
void P2PHelperStream::Cb_OnChangeToHelperResult(GSY_StreamId sid, ErrorCode err, GSY_PeerId peerId)
{
    DispatchLobbyCallback([sid,err,peerId] { if (auto* self = GetInstance(sid)) self->OnChangeToHelperResult(sid,err,peerId); });
}
void P2PHelperStream::Cb_OnAttemptConnect(GSY_StreamId sid, const char* ip, uint16_t port, uint32_t times, uint32_t verifyCode)
{
    FString Ip = ip ? UTF8_TO_TCHAR(ip) : TEXT("");
    DispatchLobbyCallback([sid,port,times,verifyCode,Ip = MoveTemp(Ip)] { if (auto* self = GetInstance(sid)) self->OnAttemptConnect(sid,TCHAR_TO_UTF8(*Ip),port,times,verifyCode); });
}
void P2PHelperStream::Cb_OnHelperQuitResult(GSY_StreamId sid, GSY_HelperResult* result)
{
    if (!result) return;
    FP2PHelperResult Copy(*result);
    DispatchLobbyCallback([sid,Copy = MoveTemp(Copy)] {
        if (auto* self = GetInstance(sid)) self->OnHelperQuitResult(sid,Copy);
        else if (Copy.Data.ret == EC_Ok) Copy.ReleaseSession();
    });
}

void P2PHelperStream::OnRegistrationSuccess(ErrorCode err, GSY_StreamId sid, GSY_PeerId peerId)
{
	_bIsRegistering = false;
	_bIsRegistered = (err == ErrorCode::EC_Ok);
	if(_bIsRegistered)
		_PeerId = peerId;
	_EventListeners.Dispatch(&ILobbyEventListener::OnRegistrationSuccess, err, sid, peerId);
}

void P2PHelperStream::OnUnregister(ErrorCode err, GSY_StreamId sid)
{
	_bIsRegistered = false;
	_PeerId = INVALID_PID;
	_EventListeners.Dispatch(&ILobbyEventListener::OnUnregister, err, sid);
}

void P2PHelperStream::OnConnectResponds(ErrorCode err, GSY_PeerId otherPeer, GSY_StreamId sid)
{
	_EventListeners.Dispatch(&ILobbyEventListener::OnConnectResponds, err, otherPeer, sid);
}

void P2PHelperStream::OnReceivePeerList(GSY_PeerData* list, uint32_t size, GSY_StreamId sid)
{
	_EventListeners.Dispatch(&ILobbyEventListener::OnReceivePeerList, list, size, sid);
}

void P2PHelperStream::OnPeerReqConnect(GSY_PeerData* peer, GSY_StreamId sid)
{
	_EventListeners.Dispatch(&ILobbyEventListener::OnPeerReqConnect, peer, sid);
}

void P2PHelperStream::OnError(GSY_ConnectionHwnd conn, GSY_StreamId sid, ErrorCode err, const char* msg, GSY_RequestId reqId)
{
	_EventListeners.Dispatch(&ILobbyEventListener::OnError, conn, sid, err, msg, reqId);
}

void P2PHelperStream::OnChangeToHelperResult(GSY_StreamId sid, ErrorCode err, GSY_PeerId peerId)
{
	_EventListeners.Dispatch(&ILobbyEventListener::OnChangeToHelperResult, sid, err, peerId);
}

void P2PHelperStream::OnAttemptConnect(GSY_StreamId sid, const char* ip, uint16_t port, uint32_t times, uint32_t verifyCode)
{
	_EventListeners.Dispatch(&ILobbyEventListener::OnAttemptConnect, sid, ip, port, times, verifyCode);
}

void P2PHelperStream::OnHelperQuitResult(GSY_StreamId sid, const FP2PHelperResult& result)
{
    if (_EventListeners.Num() == 0 && result.Data.ret == EC_Ok) result.ReleaseSession();
    _EventListeners.Dispatch(&ILobbyEventListener::OnHelperResult, sid, result);
    auto Native = result.Native();
    _EventListeners.Dispatch(&ILobbyEventListener::OnHelperQuitResult, sid, &Native);
}

ErrorCode P2PHelperStream::FetchPeerList(GSY_RequestId RequestId)
{
	if (!_bIsRegistered)
	{
		UE_LOG(LogTemp, Warning, TEXT("P2PHelperStream::FetchPeerList - Not registered, cannot fetch peer list"));
		return ErrorCode::EC_Fail;
	}
	return GSY_FetchPeerList(_hwnd, _streamId, RequestId);
}
