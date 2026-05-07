#pragma once

#include <memory>

#include "CoreMinimal.h"
#include "EUtility/UI/UIViewController.h"
#include "LobbyModel.h"
#include "P2PHelperStream.h"
#include "EUtility/UI/Common/LoadingModel.h"
#include "LobbyController.generated.h"

class P2PHelperStream;

UCLASS()
class G01_API ULobbyController : public UUIViewController, public ILobbyEventListener
{
	GENERATED_BODY()

public:
	ULobbyController();
	
	void SetStream(std::weak_ptr<P2PHelperStream> stream);
public:
	virtual void OnViewCreated(UBaseWidget* InView, const FUIViewConfig& InConfig) override;
	virtual void OnViewWillAppear() override;
	virtual void OnViewDidDisappear() override;

protected:
	virtual void BindUIEvents_Implementation() override;
	virtual void UpdateView_Implementation() override;

private:
	ULobbyModel* GetLobbyModel() const;
	
	std::weak_ptr<P2PHelperStream> LobbyStream;

	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UUserWidget> _ToastClass;

	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UBaseWidget> _LoadingViewClass;

	UPROPERTY(EditAnywhere, Category="UI")
	FUIViewConfig _LoadingViewConfig;

	UPROPERTY(EditAnywhere, Category="UI")
	FName _LoadingName = "Lobby_Loading";
	
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<ULoadingModel> _LoadingModelClass;

	ULoadingModel* _LoadingModelInstance = nullptr;

	ULoadingModel* GetLoadingModel();

	// ILobbyEventListener
	virtual void OnRegistrationSuccess(ErrorCode err, GSY_StreamId sid, GSY_PeerId peerId) override;
	virtual void OnUnregister(ErrorCode err, GSY_StreamId sid) override;
	virtual void OnConnectResponds(ErrorCode err, GSY_PeerId otherPeer, GSY_StreamId sid) override;
	virtual void OnReceivePeerList(GSY_PeerData* list, uint32_t size, GSY_StreamId sid) override;
	virtual void OnPeerReqConnect(GSY_PeerData* peer, GSY_StreamId sid) override;
	virtual void OnError(GSY_ConnectionHwnd conn, GSY_StreamId sid, ErrorCode err, const char* msg, GSY_RequestId reqId) override;
	virtual void OnChangeToHelperResult(GSY_StreamId sid, ErrorCode err, GSY_PeerId peerId) override;
	virtual void OnAttemptConnect(GSY_StreamId sid, const char* ip, uint16_t port, uint32_t times, uint32_t verifyCode) override;
	virtual void OnHelperQuitResult(GSY_StreamId sid, GSY_HelperResult* result) override;
};
