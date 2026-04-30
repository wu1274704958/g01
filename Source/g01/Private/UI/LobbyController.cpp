#include "LobbyController.h"
#include "BaseConnect.h"
#include "EUtility/UI/UIUtility.h"

// ---------------------------------------------------------------------------
// ULobbyController
// ---------------------------------------------------------------------------
ULobbyController::ULobbyController(std::weak_ptr<P2PHelperStream> stream)
	: LobbyStream(std::move(stream))
{
	static ConstructorHelpers::FClassFinder<UUserWidget> DefaultToastClass(TEXT("/Game/UI/Toast/UW_ToastDef"));
	if(DefaultToastClass.Class)
	{
		_ToastClass = DefaultToastClass.Class;
	}
	if (auto streamPtr = LobbyStream.lock())
	{
		streamPtr->AddListener(this);
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("LobbyController: P2PHelperStream is invalid."));
	}
}

void ULobbyController::OnViewCreated(UBaseWidget* InView)
{
	Super::OnViewCreated(InView);
}

void ULobbyController::OnViewWillAppear()
{
	Super::OnViewWillAppear();
	UpdateView();
}

void ULobbyController::OnViewDidDisappear()
{
	Super::OnViewDidDisappear();
	LobbyStream.reset();
}

// ---------------------------------------------------------------------------
// Bind UI
// ---------------------------------------------------------------------------
void ULobbyController::BindUIEvents_Implementation()
{
	Super::BindUIEvents_Implementation();
}

// ---------------------------------------------------------------------------
// Update UI
// ---------------------------------------------------------------------------
void ULobbyController::UpdateView_Implementation()
{
	Super::UpdateView_Implementation();
	ULobbyModel* M = GetLobbyModel();
	if (!M) return;
	
}

ULobbyModel* ULobbyController::GetLobbyModel() const
{
	return Cast<ULobbyModel>(Model);
}

// ---------------------------------------------------------------------------
// ILobbyEventListener
// ---------------------------------------------------------------------------
void ULobbyController::OnRegistrationSuccess(ErrorCode err, GSY_StreamId sid, GSY_PeerId peerId)
{
	UE_LOG(LogTemp, Log, TEXT("LobbyController::OnRegistrationSuccess - err=%d, peerId=%llu"), (int)err, (uint64)peerId);
	if (err == EC_Ok)
	{
		FString Msg = FString::Printf(TEXT("Register success\nPeerID: %llu\nStreamID: %llu"), (uint64)peerId, (uint64)sid);
		UIUtility::ShowToast(this, 0, _ToastClass, Msg, 3.0f);
	}
	else
	{
		FString Msg = FString::Printf(TEXT("Register failed\nErrorCode: %d\nStreamID: %llu"), (int)err, (uint64)sid);
		UIUtility::ShowToast(this, 0, _ToastClass, Msg, 3.0f);
	}
}

void ULobbyController::OnUnregister(ErrorCode err, GSY_StreamId sid)
{
	UE_LOG(LogTemp, Log, TEXT("LobbyController::OnUnregister - err=%d"), (int)err);
}

void ULobbyController::OnConnectResponds(ErrorCode err, GSY_PeerId otherPeer, GSY_StreamId sid)
{
	UE_LOG(LogTemp, Log, TEXT("LobbyController::OnConnectResponds - err=%d, otherPeer=%llu"), (int)err, (uint64)otherPeer);
}

void ULobbyController::OnReceivePeerList(GSY_PeerData* list, uint32_t size, GSY_StreamId sid)
{
	UE_LOG(LogTemp, Log, TEXT("LobbyController::OnReceivePeerList - count=%u"), size);
}

void ULobbyController::OnPeerReqConnect(GSY_PeerData* peer, GSY_StreamId sid)
{
	UE_LOG(LogTemp, Log, TEXT("LobbyController::OnPeerReqConnect"));
}

void ULobbyController::OnError(GSY_ConnectionHwnd conn, GSY_StreamId sid, ErrorCode err, const char* msg, GSY_RequestId reqId)
{
	UE_LOG(LogTemp, Error, TEXT("LobbyController::OnError - err=%d, msg=%hs"), (int)err, UTF8_TO_TCHAR(msg));
}

void ULobbyController::OnChangeToHelperResult(GSY_StreamId sid, ErrorCode err, GSY_PeerId peerId)
{
	UE_LOG(LogTemp, Log, TEXT("LobbyController::OnChangeToHelperResult - err=%d, peerId=%llu"), (int)err, (uint64)peerId);
}

void ULobbyController::OnAttemptConnect(GSY_StreamId sid, const char* ip, uint16_t port, uint32_t times, uint32_t verifyCode)
{
	UE_LOG(LogTemp, Log, TEXT("LobbyController::OnAttemptConnect - ip=%hs, port=%u, times=%u"), UTF8_TO_TCHAR(ip), port, times);
}

void ULobbyController::OnHelperQuitResult(GSY_StreamId sid, GSY_HelperResult* result)
{
	UE_LOG(LogTemp, Log, TEXT("LobbyController::OnHelperQuitResult"));
}
