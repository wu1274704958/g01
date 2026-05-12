// Fill out your copyright notice in the Description page of Project Settings.


#include "MqasNetAgent.h"
#include "EUtility/Public/UFilePreprocessor.h"
#include "MqasNet/example1/lobby.h"

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

std::weak_ptr<BaseConnect> AUMqasNetAgent::ConnectTo(GSY_EngineId EngineId, const FString& ConfigFile,
	const FString& Ip, short Port)
{
	const FString RealConfigPath = FPaths::ProjectSavedDir() / ConfigFile;
	const auto hwnd = GSY_connect(EngineId, TCHAR_TO_UTF8(*RealConfigPath), TCHAR_TO_UTF8(*Ip), Port, &ConnectionContext);
	if (hwnd == InvalidConnection)
	{
		UE_LOG(LogMqasNet, Warning, TEXT("ConnectTo failed: EngineId=%d, ConfigFile=%s, Ip=%s, Port=%d"),
			(int)EngineId, *ConfigFile, *Ip, Port);
		return {};
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

void AUMqasNetAgent::OnStreamOpenCallback(GSY_ConnectionHwnd hwnd, GSY_StreamId sid, ErrorCode code)
{
	if (ConnectionMap.contains(hwnd))
		ConnectionMap[hwnd]->OnStreamOpen(sid,code);
}

void AUMqasNetAgent::OnStreamCloseCallback(GSY_ConnectionHwnd hwnd, GSY_StreamId sid, ErrorCode code)
{
	if (ConnectionMap.contains(hwnd))
		ConnectionMap[hwnd]->OnStreamClose(sid, code);
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
	{
		if (IsInGameThread())
			instance->OnConnectCallback(code, hwnd);
		else
		{
			AsyncTask(ENamedThreads::GameThread, [code,hwnd]()
			{
				if (instance)
					instance->OnConnectCallback(code, hwnd);
			});
		}
	}
	UE_LOG(LogMqasNet, Verbose, TEXT("UMqasNetAgent::OnConnectCallback: handle = %u,code = %d"), hwnd,code);
}

void AUMqasNetAgent::OnDisconnectCallbackGlobal(int code, GSY_ConnectionHwnd hwnd)
{
	if (instance != nullptr)
	{
		if (IsInGameThread())
			instance->OnDisconnectCallback(code, hwnd);
		else
		{
			AsyncTask(ENamedThreads::GameThread, [code,hwnd]()
			{
				if (instance)
					instance->OnDisconnectCallback(code, hwnd);
			});
		}
	}
	UE_LOG(LogMqasNet, Verbose, TEXT("UMqasNetAgent::OnDisconnectCallback: handle = %u,code = %d"), hwnd,code);
}

void AUMqasNetAgent::OnConnectErrorCallbackGlobal(int code, GSY_ConnectionHwnd hwnd)
{
	if (instance != nullptr)
	{
		if (IsInGameThread())
			instance->OnConnectErrorCallback(code, hwnd);
		else
		{
			AsyncTask(ENamedThreads::GameThread, [code,hwnd]()
			{
				if (instance)
					instance->OnConnectErrorCallback(code, hwnd);
			});
		}
	}
	UE_LOG(LogMqasNet, Warning, TEXT("UMqasNetAgent::OnConnectErrorCallback: handle = %u,code = %d"), hwnd,code);
}

void AUMqasNetAgent::OnStreamOpenCallbackGlobal(GSY_ConnectionHwnd hwnd, GSY_StreamId sid, ErrorCode code)
{
	if (instance != nullptr)
	{
		if (IsInGameThread())
			instance->OnStreamOpenCallback(hwnd, sid, code);
		else
		{
			AsyncTask(ENamedThreads::GameThread, [hwnd,sid,code]()
			{
				if (instance)
					instance->OnStreamOpenCallback(hwnd, sid, code);
			});
		}
	}
	UE_LOG(LogMqasNet, Verbose, TEXT("UMqasNetAgent::OnStreamOpenCallback: handle = %u,stream id = %u,code = %d"), hwnd,sid,code);
}

void AUMqasNetAgent::OnStreamCloseCallbackGlobal(GSY_ConnectionHwnd hwnd, GSY_StreamId sid, ErrorCode code)
{
	if (instance != nullptr)
	{
		if (IsInGameThread())
			instance->OnStreamCloseCallback(hwnd, sid, code);
		else
		{
			AsyncTask(ENamedThreads::GameThread, [hwnd,sid,code]()
			{
				if (instance)
					instance->OnStreamCloseCallback(hwnd, sid, code);
			});
		}
	}
	UE_LOG(LogMqasNet, Verbose, TEXT("UMqasNetAgent::OnStreamCloseCallback: handle = %u,stream id = %u,code = %d"), hwnd,sid,code);
}

AUMqasNetAgent::AUMqasNetAgent()
{
	Context.on_error = OnErrorCallbackGlobal;

	ConnectionContext.on_error = OnConnectErrorCallbackGlobal;
	ConnectionContext.on_connect = OnConnectCallbackGlobal;
	ConnectionContext.on_disconnect = OnDisconnectCallbackGlobal;
	ConnectionContext.on_stream_open = OnStreamOpenCallbackGlobal;
	ConnectionContext.on_stream_close = OnStreamCloseCallbackGlobal;
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
