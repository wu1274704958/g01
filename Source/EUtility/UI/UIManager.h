// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UIViewController.h"
#include "UIViewConfig.h"
#include "Blueprint/UserWidget.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include <functional>
#include "UIManager.generated.h"

class UUIViewController;

/**
 * UI视图信息
 */
USTRUCT()
struct FUIViewInfo
{
	GENERATED_BODY()

	UPROPERTY()
	UUserWidget* Widget = nullptr;

	UPROPERTY()
	UUIViewController* Controller = nullptr;

	UPROPERTY()
	FUIViewConfig Config;

	UPROPERTY()
	FName ViewName;

	bool bIsShowing = false;
	bool bIsTransitioning = false;

	FUIViewInfo()
		: Widget(nullptr)
		, Controller(nullptr)
		, ViewName(NAME_None)
		, bIsShowing(false)
		, bIsTransitioning(false)
	{
	}
};

/**
 * UI Manager - 基于MVC架构的UI管理器
 * 支持独占模式、弹窗模式和淡入淡出动画
 */
UCLASS()
class EUTILITY_API UUIManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 获取UI Manager实例
	UFUNCTION(BlueprintCallable, Category="UI", meta=(WorldContext="WorldContextObject"))
	static UUIManager* Get(const UObject* WorldContextObject);

	// 显示UI
	UFUNCTION(BlueprintCallable, Category="UI")
	void ShowUI(FName ViewName, 
		TSubclassOf<UUserWidget> WidgetClass,
		TSubclassOf<UUIViewController> ControllerClass,
		const FUIViewConfig& Config,
		UObject* Model = nullptr);

	// 隐藏UI
	UFUNCTION(BlueprintCallable, Category="UI")
	void HideUI(FName ViewName, bool bDestroy = false);

	// 隐藏所有UI
	UFUNCTION(BlueprintCallable, Category="UI")
	void HideAllUI(bool bDestroy = false);

	// 检查UI是否正在显示
	UFUNCTION(BlueprintCallable, Category="UI")
	bool IsUIShowing(FName ViewName) const;

	// 获取UI Widget
	UFUNCTION(BlueprintCallable, Category="UI")
	UUserWidget* GetUIWidget(FName ViewName) const;

	// 获取UI Controller
	UFUNCTION(BlueprintCallable, Category="UI")
	UUIViewController* GetUIController(FName ViewName) const;

	// 更新UI模型数据
	UFUNCTION(BlueprintCallable, Category="UI")
	void UpdateUIModel(FName ViewName, UObject* Model);

	// 设置淡入淡出回调（C++使用）
	void SetFadeInCallback(FName ViewName, std::function<void()> Callback);
	void SetFadeOutCallback(FName ViewName, std::function<void()> Callback);

protected:
	// 创建UI视图
	FUIViewInfo* CreateUIView(FName ViewName,
		TSubclassOf<UUserWidget> WidgetClass,
		TSubclassOf<UUIViewController> ControllerClass,
		const FUIViewConfig& Config);

	// 执行显示逻辑
	void PerformShow(FUIViewInfo* ViewInfo, UObject* Model);

	// 执行隐藏逻辑
	void PerformHide(FUIViewInfo* ViewInfo, bool bDestroy);

	// 处理独占模式
	void HandleExclusiveMode(FUIViewInfo* ViewInfo);

	// 执行淡入动画
	void ExecuteFadeIn(FUIViewInfo* ViewInfo);

	// 执行淡出动画
	void ExecuteFadeOut(FUIViewInfo* ViewInfo, bool bDestroy);

	// 淡入完成回调
	void OnFadeInComplete(FName ViewName);

	// 淡出完成回调
	void OnFadeOutComplete(FName ViewName, bool bDestroy);

	// 应用UI输入模式
	void ApplyInputMode(FUIViewInfo* ViewInfo, bool bShow);

private:
	// 所有UI视图的映射表
	UPROPERTY()
	TMap<FName, FUIViewInfo> UIViews;

	// 当前独占的UI视图
	UPROPERTY()
	TArray<FName> ExclusiveUIStack;

	// 淡入淡出回调映射
	TMap<FName, std::function<void()>> FadeInCallbacks;
	TMap<FName, std::function<void()>> FadeOutCallbacks;

	// 获取玩家控制器
	APlayerController* GetPlayerController() const;
};

