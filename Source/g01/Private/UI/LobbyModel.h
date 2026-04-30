#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LobbyModel.generated.h"

USTRUCT(BlueprintType)
struct FPeerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Lobby")
	int64 PeerId = 0;

	UPROPERTY(BlueprintReadWrite, Category="Lobby")
	FString Name;
};

/**
 * P2P Lobby 数据模型
 */
UCLASS(BlueprintType)
class G01_API ULobbyModel : public UObject
{
	GENERATED_BODY()

public:
	// 注册信息
	UPROPERTY(BlueprintReadWrite, Category="Lobby")
	FString UserName;

	UPROPERTY(BlueprintReadWrite, Category="Lobby")
	FString Password;

	// 注册状态
	UPROPERTY(BlueprintReadWrite, Category="Lobby")
	bool bIsRegistered = false;

	UPROPERTY(BlueprintReadWrite, Category="Lobby")
	bool bIsRegistering = false;

	// 本机 PeerId（注册成功后赋值）
	UPROPERTY(BlueprintReadWrite, Category="Lobby")
	int64 LocalPeerId = 0;

	// Peer 列表
	UPROPERTY(BlueprintReadWrite, Category="Lobby")
	TArray<FPeerInfo> PeerList;

	// 状态文本（UI 显示用）
	UPROPERTY(BlueprintReadWrite, Category="Lobby")
	FString StatusText;

	// 待连接的目标 PeerId
	UPROPERTY(BlueprintReadWrite, Category="Lobby")
	int64 TargetPeerId = 0;

	void Reset();
};

