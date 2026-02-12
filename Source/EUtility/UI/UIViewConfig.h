// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
 * UI过渡动画类型
 */
UENUM(BlueprintType)
enum class EUITransition : uint8
{
	None UMETA(DisplayName = "None"),
	Fade UMETA(DisplayName = "Fade"),
	Slide UMETA(DisplayName = "Slide"),
	Scale UMETA(DisplayName = "Scale"),
	Custom UMETA(DisplayName = "Custom")
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

	// 过渡动画类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	EUITransition TransitionType = EUITransition::Fade;

	// 淡入持续时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	float FadeInDuration = 0.3f;

	// 淡出持续时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	float FadeOutDuration = 0.3f;

	// 是否可以通过ESC键关闭
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	bool bClosableByEsc = true;

	FUIViewConfig()
		: ShowMode(EUIShowMode::Normal)
		, ZOrder(0)
		, bPauseGame(false)
		, bShowMouseCursor(true)
		, TransitionType(EUITransition::Fade)
		, FadeInDuration(0.3f)
		, FadeOutDuration(0.3f)
		, bClosableByEsc(true)
	{
	}
};

