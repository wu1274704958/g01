// Fill out your copyright notice in the Description page of Project Settings.


#include "LambdaWrapper.h"

void ULambdaWrapper::Execute() const
{
	if (Func)
	{
		Func();
	}
}
