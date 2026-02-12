// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MqasNetView.generated.h"

/**
 * Mqas网络连接视图
 * 纯UI Widget，不包含业务逻辑
 * 业务逻辑由 MqasNetController 处理
 */
UCLASS()
class G01_API UMqasNetView : public UUserWidget
{
	GENERATED_BODY()

public:
	// UI控件（通过蓝图绑定）
	UPROPERTY(meta=(BindWidget))
	class UButton* btn_connect;

	UPROPERTY(meta=(BindWidget))
	class UEditableText* et_ip;

	UPROPERTY(meta=(BindWidget))
	class UEditableText* et_port;

	UPROPERTY(meta=(BindWidget))
	class UEditableText* et_name;

	UPROPERTY(meta=(BindWidget))
	class UEditableText* et_psd;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* tb_status;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* tb_btn_connect;
};

