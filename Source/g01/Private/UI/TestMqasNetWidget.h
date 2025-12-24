// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MqasNetAgent.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "TestMqasNetWidget.generated.h"

/**
 * 
 */
UCLASS()
class UTestMqasNetWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	void OnBtnTestClick();
	
	UFUNCTION(BlueprintImplementableEvent)
	AUMqasNetAgent* GetAUMqasNetAgent();

	UPROPERTY(EditAnywhere, Category="UI")
	TSoftObjectPtr<AUMqasNetAgent> _AgentNet;
	UPROPERTY(EditAnywhere, Category="UI")
	UButton* _ButtonConnect;
	UPROPERTY(EditAnywhere, Category="UI")
	UEditableText* _EtIp;
	UPROPERTY(EditAnywhere, Category="UI")
	UEditableText* _EtPort;
	
	
};
