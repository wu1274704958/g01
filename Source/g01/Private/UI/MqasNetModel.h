// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MqasNetModel.generated.h"

class AUMqasNetAgent;
/**
 * Mqas网络连接数据模型
 */
UCLASS(BlueprintType)
class G01_API UMqasNetModel : public UObject
{
	GENERATED_BODY()

public:
	UMqasNetModel();

	// 连接配置
	UPROPERTY(BlueprintReadWrite, Category="MqasNet")
	FString IpAddress;

	UPROPERTY(BlueprintReadWrite, Category="MqasNet")
	int32 Port;

	UPROPERTY(BlueprintReadWrite, Category="MqasNet")
	FString UserName;

	UPROPERTY(BlueprintReadWrite, Category="MqasNet")
	FString Password;

	// 连接状态
	UPROPERTY(BlueprintReadWrite, Category="MqasNet")
	bool bIsConnected;

	UPROPERTY(BlueprintReadWrite, Category="MqasNet")
	bool bIsConnecting;

	UPROPERTY(BlueprintReadWrite, Category="MqasNet")
	FString StatusText;

	UPROPERTY(BlueprintReadWrite, Category="MqasNet")
	FString ButtonText;

	UPROPERTY(BlueprintReadWrite, Category="MqasNet")
	int ConnectionHandle;

	// Toast配置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	TSubclassOf<UUserWidget> ToastClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MqasNet")
	TObjectPtr<AUMqasNetAgent> NetAgent;

	// 重置状态
	UFUNCTION(BlueprintCallable, Category="MqasNet")
	void ResetStatus();

	// 设置默认值
	void SetDefaults(const FString& DefaultIp = TEXT("127.0.0.1"), int32 DefaultPort = 8084);
};

