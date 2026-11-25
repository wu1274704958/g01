#pragma once
#include "BaseStream.h"
#include "gsync/example1/lobby.h"
#include <unordered_map>
#include <string>
#include <vector>

#include "EUtility/EventListenerManager.h"

// Owns all data supplied by the SDK's callback, including binary token bytes.
struct MQASNET_API FP2PHelperResult
{
	GSY_HelperResult Data{};
	std::string Reason;
	std::string RelayToken;
	std::vector<uint8_t> VerifyToken;

	FP2PHelperResult() = default;
	explicit FP2PHelperResult(const GSY_HelperResult& Result);
	GSY_HelperResult Native() const;
	// Release after an unsuccessful attempt or when this result is discarded.
	// Copies share the same SDK handle; only the first successful release succeeds.
	ErrorCode ReleaseSession() const;
};

/**
 * Listener interface for P2PHelperStream lobby events.
 * Implement this interface and register via AddListener / RemoveListener.
 */
class ILobbyEventListener
{
public:
	virtual ~ILobbyEventListener() = default;

	virtual void OnRegistrationSuccess(ErrorCode err, GSY_StreamId sid, GSY_PeerId peerId) {}
	virtual void OnUnregister(ErrorCode err, GSY_StreamId sid) {}
	virtual void OnConnectResponds(ErrorCode err, GSY_PeerId otherPeer, GSY_StreamId sid) {}
	virtual void OnReceivePeerList(GSY_PeerData* list, uint32_t size, GSY_StreamId sid) {}
	virtual void OnPeerReqConnect(GSY_PeerData* peer, GSY_StreamId sid) {}
	virtual void OnError(GSY_ConnectionHwnd conn, GSY_StreamId sid, ErrorCode err, const char* msg, GSY_RequestId reqId) {}
	virtual void OnChangeToHelperResult(GSY_StreamId sid, ErrorCode err, GSY_PeerId peerId) {}
	virtual void OnAttemptConnect(GSY_StreamId sid, const char* ip, uint16_t port, uint32_t times, uint32_t verifyCode) {}
	virtual void OnHelperQuitResult(GSY_StreamId sid, GSY_HelperResult* result) {}
	virtual void OnHelperResult(GSY_StreamId sid, const FP2PHelperResult& result) {}
	virtual void OnP2PServerReady(GSY_StreamId sid, GSY_PeerId peer, GSY_SessionHandle session) {}
};

class MQASNET_API P2PHelperStream : public BaseStream
{
	using ContextType = GSY_LobbyStreamContext;
public:
	P2PHelperStream();
	virtual ~P2PHelperStream() override;

	const ContextType* GetContext() const { return &_Context; }

	void AddListener(ILobbyEventListener* Listener)    { _EventListeners.AddListener(Listener); }
	void RemoveListener(ILobbyEventListener* Listener) { _EventListeners.RemoveListener(Listener); }
	virtual void InitData(GSY_StreamId streamId, GSY_ConnectionHwnd hwnd) override;
protected:
	virtual void OnClose() override;

	virtual void OnRegistrationSuccess(ErrorCode err, GSY_StreamId sid, GSY_PeerId peerId);
	virtual void OnUnregister(ErrorCode err, GSY_StreamId sid);
	virtual void OnConnectResponds(ErrorCode err, GSY_PeerId otherPeer, GSY_StreamId sid);
	virtual void OnReceivePeerList(GSY_PeerData* list, uint32_t size, GSY_StreamId sid);
	virtual void OnPeerReqConnect(GSY_PeerData* peer, GSY_StreamId sid);
	virtual void OnError(GSY_ConnectionHwnd conn, GSY_StreamId sid, ErrorCode err, const char* msg, GSY_RequestId reqId);
	virtual void OnChangeToHelperResult(GSY_StreamId sid, ErrorCode err, GSY_PeerId peerId);
	virtual void OnAttemptConnect(GSY_StreamId sid, const char* ip, uint16_t port, uint32_t times, uint32_t verifyCode);
	virtual void OnHelperQuitResult(GSY_StreamId sid, const FP2PHelperResult& result);
	virtual void OnP2PServerReady(GSY_StreamId sid, GSY_PeerId peer, GSY_SessionHandle session);

private:
	static void Cb_OnRegistrationSuccess(ErrorCode err, GSY_StreamId sid, GSY_PeerId peerId);
	static void Cb_OnUnregister(ErrorCode err, GSY_StreamId sid);
	static void Cb_OnConnectResponds(ErrorCode err, GSY_PeerId otherPeer, GSY_StreamId sid);
	static void Cb_OnReceivePeerList(GSY_PeerData* list, uint32_t size, GSY_StreamId sid);
	static void Cb_OnPeerReqConnect(GSY_PeerData* peer, GSY_StreamId sid);
	static void Cb_OnError(GSY_ConnectionHwnd conn, GSY_StreamId sid, ErrorCode err, const char* msg, GSY_RequestId reqId);
	static void Cb_OnChangeToHelperResult(GSY_StreamId sid, ErrorCode err, GSY_PeerId peerId);
	static void Cb_OnAttemptConnect(GSY_StreamId sid, const char* ip, uint16_t port, uint32_t times, uint32_t verifyCode);
	static void Cb_OnHelperQuitResult(GSY_StreamId sid, GSY_HelperResult* result);
	static void Cb_OnP2PServerReady(GSY_StreamId sid, GSY_PeerId peer, GSY_SessionHandle session);

	static P2PHelperStream* GetInstance(GSY_StreamId sid);

	inline static std::unordered_map<GSY_StreamId, P2PHelperStream*> s_Registry;

	ContextType _Context = {};
	EventListenerManager<ILobbyEventListener> _EventListeners;
	GSY_PeerId _PeerId = INVALID_PID;
	bool _bIsRegistering:1 = false;
	bool _bIsRegistered:1 = false;

public:
	[[nodiscard]] bool IsRegistering() const { return _bIsRegistering; }
	[[nodiscard]] bool IsRegistered() const  { return _bIsRegistered; }
	[[nodiscard]] GSY_PeerId GetPeerId() const { return _PeerId; }

	ErrorCode FetchPeerList(GSY_RequestId RequestId = NONE_RID);
	ErrorCode RequestPeer(GSY_PeerId Peer, GSY_ServerPreference Preference = GSY_SERVER_NO_PREFERENCE, GSY_RequestId RequestId = NONE_RID);
	ErrorCode RespondPeer(GSY_PeerId Peer, bool Accept, GSY_ServerPreference Preference = GSY_SERVER_NO_PREFERENCE, GSY_RequestId RequestId = NONE_RID);
	DECLARE_MULTICAST_DELEGATE_TwoParams(FServerReady, GSY_PeerId, GSY_SessionHandle);
	FServerReady OnServerReady;
	ErrorCode NotifyP2PServerReady(GSY_SessionHandle Session);
};
