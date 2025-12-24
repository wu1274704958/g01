// Fill out your copyright notice in the Description page of Project Settings.


#include "MqasNetAgent.h"
#include "EUtility/Public/UFilePreprocessor.h"
#include "MqasNet/example1/example1.h"

AUMqasNetAgent* AUMqasNetAgent::instance = nullptr;

inline void preprocess(const FString& file)
{
	auto path = FPaths::ProjectContentDir() / file;
	auto out_path = FPaths::ProjectSavedDir() / file;
	UFilePreprocessor::PreprocessFile(path,false,out_path);
}

bool AUMqasNetAgent::IsConnected(GSY_ConnectionHwnd hwnd)
{
	return GSY_is_connected(hwnd) != 0;
}

std::shared_ptr<BaseConnect> AUMqasNetAgent::ConnectTo(GSY_EngineId EngineId, const FString& ConfigFile,
	const FString& Ip, short Port)
{
	const auto hwnd = GSY_connect(EngineId, TCHAR_TO_UTF8(*ConfigFile), TCHAR_TO_UTF8(*Ip), Port, &ConnectionContext);
	if (hwnd == InvalidConnection)
	{
		UE_LOG(LogMqasNet, Warning, TEXT("ConnectTo failed: EngineId=%d, ConfigFile=%s, Ip=%s, Port=%d"),
			(int)EngineId, *ConfigFile, *Ip, Port);
		return nullptr;
	}
	auto conn = std::make_shared<BaseConnect>();
	conn->InitData(hwnd,EngineId,std::move(ConfigFile),std::move(Ip), Port);
	ConnectionMap.insert({hwnd,conn});
	return conn;
}

void AUMqasNetAgent::OnErrorCallback(const FString& msg, int code)
{
	OnError.Broadcast(msg,code);
}

void AUMqasNetAgent::OnConnectCallback(int code, GSY_ConnectionHwnd hwnd)
{
	if (!ConnectionMap.contains(hwnd))
	{
		UE_LOG(LogMqasNet,Warning,TEXT("OnConnectCallback : unknown handle = %u,code = %d"), hwnd,code);
		return;
	}
	auto conn = ConnectionMap[hwnd];
	conn->OnConnected(code);
}

void AUMqasNetAgent::OnDisconnectCallback(int code, GSY_ConnectionHwnd hwnd)
{
	if (!ConnectionMap.contains(hwnd))
	{
		UE_LOG(LogMqasNet,Warning,TEXT("OnDisconnectCallback : unknown handle = %u,code = %d"), hwnd,code);
		return;
	}
	auto conn = ConnectionMap[hwnd];
	conn->OnDisconnect(code);
	ConnectionMap.erase(hwnd);
}

void AUMqasNetAgent::OnConnectErrorCallback(int code, GSY_ConnectionHwnd hwnd)
{
	if (!ConnectionMap.contains(hwnd))
	{
		UE_LOG(LogMqasNet,Warning,TEXT("OnConnectErrorCallback : unknown handle = %u,code = %d"), hwnd,code);
		return;
	}
	auto conn = ConnectionMap[hwnd];
	conn->OnConnectError(code);
	ConnectionMap.erase(hwnd);
}

void AUMqasNetAgent::OnErrorCallbackGlobal(const char* msg, int code)
{
	const auto& Name = StringCast<WIDECHAR>(msg);
	if (instance != nullptr)
		instance->OnErrorCallback(Name.Get(), code);
	UE_LOG(LogMqasNet, Warning, TEXT("UMqasNetAgent::OnErrorCallback: %hs,code = %d"), msg,code);
}

void AUMqasNetAgent::OnConnectCallbackGlobal(int code, GSY_ConnectionHwnd hwnd)
{
	if (instance != nullptr)
		instance->OnConnectCallback(code, hwnd);
	UE_LOG(LogMqasNet, Verbose, TEXT("UMqasNetAgent::OnConnectCallback: handle = %u,code = %d"), hwnd,code);
}

void AUMqasNetAgent::OnDisconnectCallbackGlobal(int code, GSY_ConnectionHwnd hwnd)
{
	if (instance != nullptr)
		instance->OnDisconnectCallback(code, hwnd);
	UE_LOG(LogMqasNet, Verbose, TEXT("UMqasNetAgent::OnDisconnectCallback: handle = %u,code = %d"), hwnd,code);
}

void AUMqasNetAgent::OnConnectErrorCallbackGlobal(int code, GSY_ConnectionHwnd hwnd)
{
	if (instance != nullptr)
		instance->OnConnectErrorCallback(code, hwnd);
	UE_LOG(LogMqasNet, Warning, TEXT("UMqasNetAgent::OnConnectErrorCallback: handle = %u,code = %d"), hwnd,code);
}

AUMqasNetAgent::AUMqasNetAgent()
{
	Context.on_error = OnErrorCallbackGlobal;

	ConnectionContext.on_error = OnConnectErrorCallbackGlobal;
	ConnectionContext.on_connect = OnConnectCallbackGlobal;
	ConnectionContext.on_disconnect = OnDisconnectCallbackGlobal;
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
	bInitialized = GSY_initialize(0, &Context) == EC_Ok;
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
