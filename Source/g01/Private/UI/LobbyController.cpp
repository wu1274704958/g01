#include "LobbyController.h"

#include <cassert>

#include "BaseConnect.h"
#include "EUtility/UI/UIUtility.h"
#include "Common.h"
#include "LobbyView.h"
#include "Components/Button.h"
#include "EUtility/UI/UIManager.h"
#include "EUtility/UI/Common/LoadingController.h"
#include "EUtility/UI/Common/LoadingView.h"

// ---------------------------------------------------------------------------
// ULobbyController
// ---------------------------------------------------------------------------
ULobbyController::ULobbyController()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> DefaultToastClass(TEXT("/Game/UI/Toast/UW_ToastDef"));
	if(DefaultToastClass.Class)
	{
		_ToastClass = DefaultToastClass.Class;
	}
	_LoadingViewClass = ULoadingView::StaticClass();
}

void ULobbyController::SetStream(std::weak_ptr<P2PHelperStream> stream)
{
	LobbyStream = std::move(stream);
	if (auto streamPtr = LobbyStream.lock())
	{
		if (auto model = GetLobbyModel())
		{
			model->bIsRegistered = streamPtr->IsRegistered();
			model->bIsRegistering = streamPtr->IsRegistering();
			model->LocalPeerId = streamPtr->GetPeerId();
			if (model->bIsRegistered)
			{
				OnRegistrationSuccess(EC_Ok, streamPtr->GetStreamId(), model->LocalPeerId);	
			}else
			{
				UUIManager::Get(this)->ShowUI(_LoadingName, _LoadingViewClass, ULoadingController::StaticClass(),
					_LoadingViewConfig,GetLoadingModel());
			}
		}
		else
		{
			UE_LOG(LogMqasNet, Warning, TEXT("LobbyController: Failed to get LobbyModel."));
		}
		streamPtr->AddListener(this);
		
	}else
	{
		UE_LOG(LogMqasNet, Warning, TEXT("LobbyController: P2PHelperStream is invalid."));
	}
}

void ULobbyController::OnViewCreated(UBaseWidget* InView, const FUIViewConfig& InConfig)
{
	Super::OnViewCreated(InView,InConfig);
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
	ULobbyView* V = Cast<ULobbyView>(View);
	if (!M || !V) return;

	V->btn_refresh_list->SetVisibility(M->bIsRegistered ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	V->btn_request_connect->SetVisibility(M->bIsRegistered ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	
}

ULobbyModel* ULobbyController::GetLobbyModel() const
{
	return Cast<ULobbyModel>(Model);
}

ULoadingModel* ULobbyController::GetLoadingModel()
{
	if (_LoadingModelInstance)
		return _LoadingModelInstance;
	if (_LoadingModelClass == nullptr)
		UE_LOG(LogMqasNet, Error, TEXT("LobbyController: LoadingModelClass is not set!"));
	return _LoadingModelInstance = NewObject<ULoadingModel>(this, _LoadingModelClass);
}

// ---------------------------------------------------------------------------
// ILobbyEventListener
// ---------------------------------------------------------------------------
void ULobbyController::OnRegistrationSuccess(ErrorCode err, GSY_StreamId sid, GSY_PeerId peerId)
{
	UUIManager::Get(this)->HideUI(_LoadingName);
	UE_LOG(LogMqasNet, Log, TEXT("LobbyController::OnRegistrationSuccess - err=%d, peerId=%llu"), (int)err, (uint64)peerId);
	if (err == EC_Ok)
	{
		FString Msg = FString::Printf(TEXT("Register success\nPeerID: %llu\nStreamID: %llu"), (uint64)peerId, (uint64)sid);
		UIUtility::ShowToast(this, 0, _ToastClass, Msg, 3.0f);
		ULobbyModel* M = GetLobbyModel();
		assert(M != nullptr);
		M->bIsRegistered = true;
		M->bIsRegistering = false;
		M->LocalPeerId = peerId;
		UpdateView_Implementation();
		//!todo 这里可以直接请求一次PeerList
	}
	else
	{
		FString Msg = FString::Printf(TEXT("Register failed\nErrorCode: %d\nStreamID: %llu"), (int)err, (uint64)sid);
		UIUtility::ShowToast(this, 0, _ToastClass, Msg, 3.0f);
		HideSelf();
	}
}

void ULobbyController::OnUnregister(ErrorCode err, GSY_StreamId sid)
{
	UE_LOG(LogMqasNet, Log, TEXT("LobbyController::OnUnregister - err=%d"), (int)err);
}

void ULobbyController::OnConnectResponds(ErrorCode err, GSY_PeerId otherPeer, GSY_StreamId sid)
{
	UE_LOG(LogMqasNet, Log, TEXT("LobbyController::OnConnectResponds - err=%d, otherPeer=%llu"), (int)err, (uint64)otherPeer);
}

void ULobbyController::OnReceivePeerList(GSY_PeerData* list, uint32_t size, GSY_StreamId sid)
{
	UE_LOG(LogMqasNet, Log, TEXT("LobbyController::OnReceivePeerList - count=%u"), size);
}

void ULobbyController::OnPeerReqConnect(GSY_PeerData* peer, GSY_StreamId sid)
{
	UE_LOG(LogMqasNet, Log, TEXT("LobbyController::OnPeerReqConnect"));
}

void ULobbyController::OnError(GSY_ConnectionHwnd conn, GSY_StreamId sid, ErrorCode err, const char* msg, GSY_RequestId reqId)
{
	UE_LOG(LogMqasNet, Error, TEXT("LobbyController::OnError - err=%d, msg=%s"), (int)err, UTF8_TO_TCHAR(msg));
}

void ULobbyController::OnChangeToHelperResult(GSY_StreamId sid, ErrorCode err, GSY_PeerId peerId)
{
	UE_LOG(LogMqasNet, Log, TEXT("LobbyController::OnChangeToHelperResult - err=%d, peerId=%llu"), (int)err, (uint64)peerId);
}

void ULobbyController::OnAttemptConnect(GSY_StreamId sid, const char* ip, uint16_t port, uint32_t times, uint32_t verifyCode)
{
	UE_LOG(LogMqasNet, Log, TEXT("LobbyController::OnAttemptConnect - ip=%s, port=%u, times=%u"), UTF8_TO_TCHAR(ip), port, times);
}

void ULobbyController::OnHelperQuitResult(GSY_StreamId sid, GSY_HelperResult* result)
{
	UE_LOG(LogMqasNet, Log, TEXT("LobbyController::OnHelperQuitResult"));
}
