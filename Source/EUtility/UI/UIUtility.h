// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <map>
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "IToast.h"
#include "EUtility/LambdaWrapper.h"
#include "UIUtility.generated.h"

/**
 * 
 */
UCLASS()
class EUTILITY_API UUIUtility : public UObject
{
	GENERATED_BODY()
public:
	static constexpr int ToastZOrder = 1000;
	static int CurrentToastZOrder;
	UFUNCTION(BlueprintCallable, Category="UI")
	static void ShowToast(UObject* obj,int PlayerIndex,const TSubclassOf<class UUserWidget> Class,const FString& Message,
		float Duration = 2.0f,float XOffset = 0.5f,float YOffset = 0.7f);
};
