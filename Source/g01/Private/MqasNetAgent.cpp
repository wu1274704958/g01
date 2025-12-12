// Fill out your copyright notice in the Description page of Project Settings.


#include "MqasNetAgent.h"

#include <string>

#include "UFilePreprocessor.h"
#include "MqasNet/example1/example1.h"

DEFINE_LOG_CATEGORY(LogMqasNet)



AUMqasNetAgent* AUMqasNetAgent::instance = nullptr;

inline void preprocess(const FString& file)
{
	auto path = FPaths::ProjectContentDir() / file;
	auto out_path = FPaths::ProjectSavedDir() / file;
	UFilePreprocessor::PreprocessFile(path,false,out_path);
}

bool AUMqasNetAgent::ConnectToHolePunchingServer(const FString& name, const FString& psd)
{
	if (!bInitialized || SelfId > 0)
		return false;
	auto ConfigPath = FPaths::ProjectSavedDir() / TEXT("NetConfig/hole_punching_config.txt");

	const auto& Path = StringCast<ANSICHAR>(*ConfigPath);
	const auto& Name = StringCast<ANSICHAR>(*name);
	const auto& Psd = StringCast<ANSICHAR>(*psd);
	
	const auto id = GSY_connect_hole_punching_server(Path.Get(),Name.Get(),Psd.Get(),OnConnectedCallbackGlobal,OnRequestCallbackGlobal);
	if (id > 0 && id < EC_ErrorBegin)
		SelfId = id;
	return SelfId > 0;
}

bool AUMqasNetAgent::DisconnectToHolePunchingServer()
{
	if (!bInitialized || SelfId == 0)
		return false;
	const auto ret = GSY_disconnect_hole_punching_server(SelfId);
	SelfId = 0;
	return ret == EC_Ok;
}

void AUMqasNetAgent::OnConnectedCallback(int code, unsigned int peer_id)
{
	OnConnected.Broadcast(code, peer_id);
}

void AUMqasNetAgent::OnRequestCallback(const FString& name, unsigned int id)
{
	OnRequest.Broadcast(name,id);
}

void AUMqasNetAgent::OnErrorCallback(const FString& msg, int code)
{
	OnError.Broadcast(msg,code);
}

void AUMqasNetAgent::OnConnectedCallbackGlobal(int code, unsigned int peer_id)
{
	if (instance != nullptr)
		instance->OnConnectedCallback(code, peer_id);
	UE_LOG(LogMqasNet, Warning, TEXT("UMqasNetAgent::OnConnectedCallback: code = %d,peer_id = %d"), code,peer_id);
}

void AUMqasNetAgent::OnRequestCallbackGlobal(struct PeerData* peer_data)
{
	const auto& Name = StringCast<WIDECHAR>(peer_data->name);
	if (instance != nullptr)
		instance->OnRequestCallback(Name.Get(), peer_data->id);
	UE_LOG(LogMqasNet, Warning, TEXT("UMqasNetAgent::OnRequestCallback: peer id = %d,name = %s"), peer_data->id,Name.Get());
}

void AUMqasNetAgent::OnErrorCallbackGlobal(const char* msg, int code)
{
	const auto& Name = StringCast<WIDECHAR>(msg);
	if (instance != nullptr)
		instance->OnErrorCallback(Name.Get(), code);
	UE_LOG(LogMqasNet, Warning, TEXT("UMqasNetAgent::OnErrorCallback: %hs,code = %d"), msg,code);
}

AUMqasNetAgent::AUMqasNetAgent()
{
	
}

void AUMqasNetAgent::BeginPlay()
{
	if (instance != nullptr && instance != this)
	{
		//error
		UE_LOG(LogTemp, Error, TEXT("MqasNetAgent instance already exists!"));
		return;
	}
	instance = this;
	preprocess(TEXT("NetConfig/hole_punching_config.txt"));
	bInitialized = GSY_initialize(0, OnErrorCallbackGlobal) == EC_Ok;
	UE_LOG(LogMqasNet, Verbose , TEXT("UMqasNetAgent initialization status = %s"), bInitialized ? TEXT("true") : TEXT("false"));
	Super::BeginPlay();
}

void AUMqasNetAgent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (bInitialized && (GSY_terminate() == EC_Ok))
	{
		bInitialized = false;
	}
	if (instance == this)
		instance = nullptr;
}
