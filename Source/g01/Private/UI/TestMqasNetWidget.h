// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MqasNetAgent.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "TestMqasNetWidget.generated.h"

/**
 * 
 */
UCLASS()
class UTestMqasNetWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTestMqasNetWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	UFUNCTION()
	void OnBtnTestClick();
	
	UFUNCTION(BlueprintImplementableEvent)
	AUMqasNetAgent* GetAUMqasNetAgent();

	void OnConnectError(int code,GSY_ConnectionHwnd hwnd);
	void OnConnected(int code,GSY_ConnectionHwnd hwnd);
	void OnDisconnected(int code,GSY_ConnectionHwnd hwnd);
protected:

	UPROPERTY(EditAnywhere, Category="UI")
	TSoftObjectPtr<AUMqasNetAgent> _AgentNet;
	UPROPERTY(EditAnywhere, Category="UI")
	UButton* _BtnConnect;
	UPROPERTY(EditAnywhere, Category="UI")
	UEditableText* _EtIp;
	UPROPERTY(EditAnywhere, Category="UI")
	UEditableText* _EtPort;
	UPROPERTY(EditAnywhere, Category="UI")
	UEditableText* _EtName;
	UPROPERTY(EditAnywhere, Category="UI")
	UEditableText* _EtPsd;
	UPROPERTY(EditAnywhere, Category="UI")
	UTextBlock* _TbStatus;
	UPROPERTY(EditAnywhere, Category="UI")
	UTextBlock* _TbBtnConnect;
	
	UPROPERTY(EditAnywhere, Category="UI")
	FString _DefaultIp = "127.0.0.1";
	UPROPERTY(EditAnywhere, Category="UI")
	int32 _DefaultPort = 8084;

	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UUserWidget> _ToastClass;

	std::weak_ptr<BaseConnect> _CurrentConnect;
};
