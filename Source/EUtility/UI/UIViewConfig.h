// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Transitions/IUITransition.h"
#include "UIViewConfig.generated.h"

/**
 * UI显示模式
 */
UENUM(BlueprintType)
enum class EUIShowMode : uint8
{
	// 独占模式 - 显示时隐藏其他所有UI
	Exclusive UMETA(DisplayName = "Exclusive"),
	
	// 弹窗模式 - 可以叠加显示
	Popup UMETA(DisplayName = "Popup"),
	
	// 普通模式 - 正常显示，不影响其他UI
	Normal UMETA(DisplayName = "Normal")
};

/**
 * UI视图配置
 */
USTRUCT(BlueprintType)
struct EUTILITY_API FUIViewConfig
{
	GENERATED_BODY()

	// UI显示模式
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	EUIShowMode ShowMode = EUIShowMode::Normal;

	// UI层级（ZOrder）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	int32 ZOrder = 0;

	// 是否在显示时暂停游戏
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	bool bPauseGame = false;

	// 是否显示鼠标光标
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	bool bShowMouseCursor = true;

	// 显示时的过渡效果类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|Transaction")
	TSubclassOf<UUITransaction> ShowTransactionClass;

	// 隐藏时的过渡效果类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|Transaction")
	TSubclassOf<UUITransaction> HideTransactionClass;

	// 显示过渡持续时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|Transaction")
	float FadeInDuration = 0.3f;

	// 隐藏过渡持续时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|Transaction")
	float FadeOutDuration = 0.3f;

	// 是否可以通过ESC键关闭
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	bool bClosableByEsc = true;

	FUIViewConfig()
		: ShowMode(EUIShowMode::Normal)
		, ZOrder(0)
		, bPauseGame(false)
		, bShowMouseCursor(true)
		, ShowTransactionClass(nullptr)
		, HideTransactionClass(nullptr)
		, FadeInDuration(0.3f)
		, FadeOutDuration(0.3f)
		, bClosableByEsc(true)
	{
	}
};
