#pragma once

#include "CoreMinimal.h"
#include "EUtility/UI/BaseWidget.h"
#include "LobbyModel.h"
#include "PeerListItemWidget.generated.h"

/**
 * Peer 列表单项 Widget
 * 显示单个 Peer 的 ID 和名称，以及连接按钮
 */
UCLASS()
class G01_API UPeerListItemWidget : public UBaseWidget
{
	GENERATED_BODY()

public:
	// 显示 Peer 名称
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* tb_peer_name;

	// 显示 PeerID
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* tb_peer_id;

	// 连接按钮
	UPROPERTY(meta=(BindWidget))
	class UButton* btn_connect;

	// 填充数据
	UFUNCTION(BlueprintCallable, Category="Lobby")
	void SetPeerInfo(const FPeerInfo& InPeerInfo);

	// 点击连接时通知外部的委托（非 Dynamic，支持 AddLambda）
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnConnectClicked, int64 /*PeerId*/);
	FOnConnectClicked OnConnectClicked;

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleConnectClicked();

	FPeerInfo PeerInfo;
};
