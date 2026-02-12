// Fill out your copyright notice in the Description page of Project Settings.

#include "MqasNetModel.h"

#include "Blueprint/UserWidget.h"


UMqasNetModel::UMqasNetModel()
{
	IpAddress = TEXT("127.0.0.1");
	Port = 8084;
	UserName = TEXT("");
	Password = TEXT("");
	bIsConnected = false;
	bIsConnecting = false;
	StatusText = TEXT("Not Connected");
	ButtonText = TEXT("Connect");
	ConnectionHandle = 0;

	// 加载默认Toast类
	static ConstructorHelpers::FClassFinder<UUserWidget> DefaultToastClass(TEXT("/Game/UI/Toast/UW_ToastDef"));
	if(DefaultToastClass.Class)
	{
		ToastClass = DefaultToastClass.Class;
	}
}

void UMqasNetModel::ResetStatus()
{
	bIsConnected = false;
	bIsConnecting = false;
	StatusText = TEXT("Not Connected");
	ButtonText = TEXT("Connect");
	ConnectionHandle = 0;
}

void UMqasNetModel::SetDefaults(const FString& DefaultIp, int32 DefaultPort)
{
	IpAddress = DefaultIp;
	Port = DefaultPort;
}

