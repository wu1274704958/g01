#pragma once

#include "CoreMinimal.h"
#include "EUtility/UI/BaseWidget.h"
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
};

