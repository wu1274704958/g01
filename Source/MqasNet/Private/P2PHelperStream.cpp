#include "P2PHelperStream.h"

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

void P2PHelperStream::InitData(GSY_StreamId streamId, GSY_ConnectionHwnd hwnd)
{
	BaseStream::InitData(streamId, hwnd);
	s_Registry[streamId] = this;
	_bIsRegistering = true;
}

void P2PHelperStream::OnClose()
{
	s_Registry.erase(_streamId);
	BaseStream::OnClose();
}

P2PHelperStream* P2PHelperStream::GetInstance(GSY_StreamId sid)
{
	auto it = s_Registry.find(sid);
	return it != s_Registry.end() ? it->second : nullptr;
}

void P2PHelperStream::Cb_OnRegistrationSuccess(ErrorCode err, GSY_StreamId sid, GSY_PeerId peerId)
{
	if (auto* const self = GetInstance(sid))
	{
		AsyncTask(ENamedThreads::GameThread, [self,err,sid,peerId]()
		{
			self->OnRegistrationSuccess(err, sid, peerId);
		});
	}
}

void P2PHelperStream::Cb_OnUnregister(ErrorCode err, GSY_StreamId sid)
{
	if (auto* self = GetInstance(sid))
	{
		AsyncTask(ENamedThreads::GameThread, [self, err, sid]()
		{
			self->OnUnregister(err, sid);
		});
	}
}

void P2PHelperStream::Cb_OnConnectResponds(ErrorCode err, GSY_PeerId otherPeer, GSY_StreamId sid)
{
	if (auto* self = GetInstance(sid))
	{
		AsyncTask(ENamedThreads::GameThread, [self, err, otherPeer, sid]()
		{
			self->OnConnectResponds(err, otherPeer, sid);
		});
	}
}

void P2PHelperStream::Cb_OnReceivePeerList(GSY_PeerData* list, uint32_t size, GSY_StreamId sid)
{
	// 拷贝 peer 列表，避免原始指针在 GameThread 执行前失效
	TArray<GSY_PeerData> PeerList(list, size);
	if (auto* self = GetInstance(sid))
	{
		AsyncTask(ENamedThreads::GameThread, [self, PeerList = MoveTemp(PeerList), size, sid]() mutable
		{
			self->OnReceivePeerList(PeerList.GetData(), size, sid);
		});
	}
}

void P2PHelperStream::Cb_OnPeerReqConnect(GSY_PeerData* peer, GSY_StreamId sid)
{
	// 拷贝 peer 数据，避免指针悬空
	GSY_PeerData PeerCopy = *peer;
	if (auto* self = GetInstance(sid))
	{
		AsyncTask(ENamedThreads::GameThread, [self, PeerCopy, sid]() mutable
		{
			self->OnPeerReqConnect(&PeerCopy, sid);
		});
	}
}

void P2PHelperStream::Cb_OnError(GSY_ConnectionHwnd conn, GSY_StreamId sid, ErrorCode err, const char* msg, GSY_RequestId reqId)
{
	// 拷贝 msg 字符串，避免原始指针在 GameThread 执行前失效
	FString MsgCopy = msg ? UTF8_TO_TCHAR(msg) : TEXT("");
	if (auto* self = GetInstance(sid))
	{
		AsyncTask(ENamedThreads::GameThread, [self, conn, sid, err, MsgCopy = MoveTemp(MsgCopy), reqId]()
		{
			self->OnError(conn, sid, err, TCHAR_TO_UTF8(*MsgCopy), reqId);
		});
	}
}

void P2PHelperStream::Cb_OnChangeToHelperResult(GSY_StreamId sid, ErrorCode err, GSY_PeerId peerId)
{
	if (auto* self = GetInstance(sid))
	{
		AsyncTask(ENamedThreads::GameThread, [self, sid, err, peerId]()
		{
			self->OnChangeToHelperResult(sid, err, peerId);
		});
	}
}

void P2PHelperStream::Cb_OnAttemptConnect(GSY_StreamId sid, const char* ip, uint16_t port, uint32_t times, uint32_t verifyCode)
{
	// 拷贝 ip 字符串，避免原始指针在 GameThread 执行前失效
	FString IpCopy = ip ? UTF8_TO_TCHAR(ip) : TEXT("");
	if (auto* self = GetInstance(sid))
	{
		AsyncTask(ENamedThreads::GameThread, [self, sid, IpCopy = MoveTemp(IpCopy), port, times, verifyCode]()
		{
			self->OnAttemptConnect(sid, TCHAR_TO_UTF8(*IpCopy), port, times, verifyCode);
		});
	}
}

void P2PHelperStream::Cb_OnHelperQuitResult(GSY_StreamId sid, GSY_HelperResult* result)
{
	// 拷贝 result 数据，避免指针悬空
	GSY_HelperResult ResultCopy = *result;
	if (auto* self = GetInstance(sid))
	{
		AsyncTask(ENamedThreads::GameThread, [self, sid, ResultCopy]() mutable
		{
			self->OnHelperQuitResult(sid, &ResultCopy);
		});
	}
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

void P2PHelperStream::OnHelperQuitResult(GSY_StreamId sid, GSY_HelperResult* result)
{
	_EventListeners.Dispatch(&ILobbyEventListener::OnHelperQuitResult, sid, result);
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
