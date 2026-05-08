#pragma once

#include "CoreMinimal.h"
#include "EUtility/UI/BaseWidget.h"
#include "LobbyModel.h"
#include "LobbyView.generated.h"

/**
 * P2P Lobby 视图
 * 纯UI Widget，不包含业务逻辑
 * 业务逻辑由 LobbyController 处理
 */
UCLASS()
class G01_API ULobbyView : public UBaseWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidget))
	class UButton* btn_request_connect;


	UPROPERTY(meta=(BindWidget))
	class UButton* btn_refresh_list;

	// 列表项 Widget 类（在蓝图 Class Defaults 中指定 WBP_PeerListItem）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	TSubclassOf<class UPeerListItemWidget> PeerListItemClass;
	
	// 连接按钮点击回调类型
	DECLARE_DELEGATE_OneParam(FOnConnectClickedDelegate, int64 /*PeerId*/);
	
	// Peer 列表滚动容器
	UPROPERTY(meta=(BindWidget))
	class UScrollBox* scroll_peer_list;

	/**
	 * 用 PeerList 数据刷新 scroll_peer_list
	 * 由 LobbyController 调用
	 */
	void RefreshPeerList(const TArray<FPeerInfo>& PeerList,
	                     FOnConnectClickedDelegate OnConnectClicked);
};
