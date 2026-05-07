// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWidget.h"
#include "UIViewConfig.h"
#include "UIViewController.generated.h"

/**
 * UI View Controller - MVC架构中的Controller层
 * 负责View的生命周期管理和业务逻辑
 */
UCLASS(Abstract, Blueprintable)
class EUTILITY_API UUIViewController : public UObject
{
	GENERATED_BODY()

public:
	UUIViewController();

	// 生命周期回调
	virtual void OnViewCreated(UBaseWidget* InView, const FUIViewConfig& InConfig);
	virtual void OnViewWillAppear();
	virtual void OnViewDidAppear();
	virtual void OnViewWillDisappear();
	virtual void OnViewDidDisappear();
	virtual void OnViewDestroyed();

	// Fade动画回调
	UFUNCTION(BlueprintNativeEvent, Category="UI|Transition")
	void OnFadeIn(float Duration);
	virtual void OnFadeIn_Implementation(float Duration);

	UFUNCTION(BlueprintNativeEvent, Category="UI|Transition")
	void OnFadeOut(float Duration);
	virtual void OnFadeOut_Implementation(float Duration);

	// 获取关联的View
	UFUNCTION(BlueprintCallable, Category="UI")
	UBaseWidget* GetView() const { return View; }

	// 设置模型数据
	UFUNCTION(BlueprintCallable, Category="UI")
	virtual void SetModel(UObject* InModel);

	UFUNCTION(BlueprintCallable, Category="UI")
	UObject* GetModel() const { return Model; }

	// 获取视图配置
	UFUNCTION(BlueprintCallable, Category="UI")
	const FUIViewConfig& GetViewConfig() const { return ViewConfig; }

protected:
	UPROPERTY()
	UBaseWidget* View;

	UPROPERTY()
	UObject* Model;

	UPROPERTY()
	FUIViewConfig ViewConfig;

	// 用于子类绑定UI事件
	UFUNCTION(BlueprintNativeEvent, Category="UI")
	void BindUIEvents();
	virtual void BindUIEvents_Implementation();

	// 用于子类更新UI显示
	UFUNCTION(BlueprintNativeEvent, Category="UI")
	void UpdateView();
	virtual void UpdateView_Implementation();
};
