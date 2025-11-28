// Fill out your copyright notice in the Description page of Project Settings.


#include "MqasNetAgent.h"

#include <string>

#include "UFilePreprocessor.h"
#include "MqasNet/example1/example1.h"

UMqasNetAgent* UMqasNetAgent::instance = nullptr;

inline void preprocess(const FString& file)
{
	auto path = FPaths::ProjectContentDir() / file;
	auto out_path = FPaths::ProjectSavedDir() / file;
	UFilePreprocessor::PreprocessFile(path,false,out_path);
}

void UMqasNetAgent::PostInitProperties()
{
	UObject::PostInitProperties();
	preprocess(TEXT("NetConfig/hole_punching_config.txt"));
	
	bInitialized = GSY_initialize(0, OnErrorCallback) == EC_Ok;
}

bool UMqasNetAgent::ConnectToHolePunchingServer(const FString& name, const FString& psd)
{
	if (!bInitialized || SelfId > 0)
		return false;
	auto ConfigPath = FPaths::ProjectSavedDir() / TEXT("NetConfig/hole_punching_config.txt");

	const auto& Path = StringCast<ANSICHAR>(*ConfigPath);
	const auto& Name = StringCast<ANSICHAR>(*name);
	const auto& Psd = StringCast<ANSICHAR>(*psd);
	
	const auto id = GSY_connect_hole_punching_server(Path.Get(),Name.Get(),Psd.Get(),OnConnectedCallback,OnRequestCallback);
	if (id > 0 && id < EC_ErrorBegin)
		SelfId = id;
	return SelfId > 0;
}

void UMqasNetAgent::OnConnectedCallback(int code, unsigned int peer_id)
{
	UE_LOG(LogTemp, Warning, TEXT("UMqasNetAgent::OnConnectedCallback: code = %d,peer_id = %d"), code,peer_id);
}

void UMqasNetAgent::OnRequestCallback(struct PeerData* peer_data)
{
	const auto& Name = StringCast<WIDECHAR>(peer_data->name);
	UE_LOG(LogTemp, Warning, TEXT("UMqasNetAgent::OnRequestCallback: peer id = %d,name = %s"), peer_data->id,Name.Get());
}

void UMqasNetAgent::OnErrorCallback(const char* msg, int code)
{
	UE_LOG(LogTemp, Warning, TEXT("UMqasNetAgent::OnErrorCallback: %hs,code = %d"), msg,code);
}

UMqasNetAgent::UMqasNetAgent()
{
	instance = this;
}

void UMqasNetAgent::BeginDestroy()
{
	UObject::BeginDestroy();
	if (bInitialized && (GSY_terminate() == EC_Ok))
	{
		bInitialized = false;
	}
}
