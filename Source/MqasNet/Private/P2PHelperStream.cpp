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

void P2PHelperStream::InitData(GSY_StreamId streamId)
{
	BaseStream::InitData(streamId);
	s_Registry[streamId] = this;
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
	if (auto* self = GetInstance(sid)) self->OnRegistrationSuccess(err, sid, peerId);
}

void P2PHelperStream::Cb_OnUnregister(ErrorCode err, GSY_StreamId sid)
{
	if (auto* self = GetInstance(sid)) self->OnUnregister(err, sid);
}

void P2PHelperStream::Cb_OnConnectResponds(ErrorCode err, GSY_PeerId otherPeer, GSY_StreamId sid)
{
	if (auto* self = GetInstance(sid)) self->OnConnectResponds(err, otherPeer, sid);
}

void P2PHelperStream::Cb_OnReceivePeerList(GSY_PeerData* list, uint32_t size, GSY_StreamId sid)
{
	if (auto* self = GetInstance(sid)) self->OnReceivePeerList(list, size, sid);
}

void P2PHelperStream::Cb_OnPeerReqConnect(GSY_PeerData* peer, GSY_StreamId sid)
{
	if (auto* self = GetInstance(sid)) self->OnPeerReqConnect(peer, sid);
}

void P2PHelperStream::Cb_OnError(GSY_ConnectionHwnd conn, GSY_StreamId sid, ErrorCode err, const char* msg, GSY_RequestId reqId)
{
	if (auto* self = GetInstance(sid)) self->OnError(conn, sid, err, msg, reqId);
}

void P2PHelperStream::Cb_OnChangeToHelperResult(GSY_StreamId sid, ErrorCode err, GSY_PeerId peerId)
{
	if (auto* self = GetInstance(sid)) self->OnChangeToHelperResult(sid, err, peerId);
}

void P2PHelperStream::Cb_OnAttemptConnect(GSY_StreamId sid, const char* ip, uint16_t port, uint32_t times, uint32_t verifyCode)
{
	if (auto* self = GetInstance(sid)) self->OnAttemptConnect(sid, ip, port, times, verifyCode);
}

void P2PHelperStream::Cb_OnHelperQuitResult(GSY_StreamId sid, GSY_HelperResult* result)
{
	if (auto* self = GetInstance(sid)) self->OnHelperQuitResult(sid, result);
}

void P2PHelperStream::OnRegistrationSuccess(ErrorCode err, GSY_StreamId sid, GSY_PeerId peerId)
{
	_EventListeners.Dispatch(&ILobbyEventListener::OnRegistrationSuccess, err, sid, peerId);
}

void P2PHelperStream::OnUnregister(ErrorCode err, GSY_StreamId sid)
{
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
