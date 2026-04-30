// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EUtility/UI/UIViewController.h"
#include "MqasNetModel.h"
#include "MqasNetAgent.h"
#include "MqasNetController.generated.h"

class BaseConnect;

/**
 * Mqas网络连接控制器
 * 负责处理网络连接的业务逻辑
 */
UCLASS()
class G01_API UMqasNetController : public UUIViewController
{
	GENERATED_BODY()

public:
	UMqasNetController();

	virtual void OnViewCreated(UBaseWidget* InView) override;
	virtual void OnViewWillAppear() override;
	virtual void OnViewDidDisappear() override;

protected:
	virtual void BindUIEvents_Implementation() override;
	virtual void UpdateView_Implementation() override;

	// 获取Agent（由蓝图实现）
	UFUNCTION(BlueprintCallable, Category="MqasNet")
	AUMqasNetAgent* GetMqasNetAgent();

private:
	// 按钮点击事件
	UFUNCTION()
	void OnConnectButtonClicked();

	// 网络连接回调
	void OnConnectError(int Code, GSY_ConnectionHwnd Hwnd);
	void OnConnected(int Code, GSY_ConnectionHwnd Hwnd);
	void OnDisconnected(int Code, GSY_ConnectionHwnd Hwnd);

	// 显示Toast提示
	void ShowToast(const FString& Message);

	// 验证输入
	bool ValidateInput();

	// 获取Model
	UMqasNetModel* GetMqasModel() const;

private:
	UPROPERTY()
	AUMqasNetAgent* AgentNet;

	std::weak_ptr<BaseConnect> CurrentConnect;

	// UI组件引用
	UPROPERTY()
	class UButton* BtnConnect;

	UPROPERTY()
	class UEditableText* EtIp;

	UPROPERTY()
	class UEditableText* EtPort;

	UPROPERTY()
	class UEditableText* EtName;

	UPROPERTY()
	class UEditableText* EtPsd;

	UPROPERTY()
	class UTextBlock* TbStatus;

	UPROPERTY()
	class UTextBlock* TbBtnConnect;
};

