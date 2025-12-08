// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <functional>

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LambdaWrapper.generated.h"

/**
 * 
 */
UCLASS()
class EUTILITY_API ULambdaWrapper : public UObject
{
	GENERATED_BODY()

public:
	ULambdaWrapper():Func(nullptr) {};
	void SetFunction(const std::function<void()>& InFunc)	
	{
		Func = InFunc;
	}
	void Execute() const;
	
private:
	std::function<void()> Func;
};
