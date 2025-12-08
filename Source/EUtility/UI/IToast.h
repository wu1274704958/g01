// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IToast.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UIToast : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class EUTILITY_API IIToast
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Toast")
	void Show(const FString& Message);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Toast")
	void Hide();
};
