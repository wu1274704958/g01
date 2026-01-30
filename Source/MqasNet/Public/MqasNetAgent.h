// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <memory>
#include <unordered_map>

#include "CoreMinimal.h"
#include "BaseConnect.h"
#include "example1/example1.h"
#include "UObject/Object.h"
#include "MqasNet/Public/Common.h"
#include "MqasNetAgent.generated.h"

/**
 * 
 */
UCLASS()
class MQASNET_API AUMqasNetAgent : public AActor
{
	GENERATED_BODY()

public:
	AUMqasNetAgent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintCallable,Category="MqasNetAgent")
	bool IsInitialized() const { return bInitialized; }
	bool IsConnected(GSY_ConnectionHwnd hwnd);
	std::weak_ptr<BaseConnect> ConnectTo(GSY_EngineId EngineId,const FString& ConfigFile,
		const FString& Ip,short Port);
private:
	void OnErrorCallback(const FString& msg,int code);
	void OnConnectCallback(int code,GSY_ConnectionHwnd hwnd);
	void OnDisconnectCallback(int code,GSY_ConnectionHwnd hwnd);
	void OnConnectErrorCallback(int code,GSY_ConnectionHwnd hwnd);
	void OnStreamOpenCallback(GSY_ConnectionHwnd,GSY_StreamId,ErrorCode);
	void OnStreamCloseCallback(GSY_ConnectionHwnd,GSY_StreamId,ErrorCode);
private:// static members
	static AUMqasNetAgent* instance;
	static void OnErrorCallbackGlobal(const char* msg,int code);
	static void OnConnectCallbackGlobal(int code,GSY_ConnectionHwnd hwnd);
	static void OnDisconnectCallbackGlobal(int code,GSY_ConnectionHwnd hwnd);
	static void OnConnectErrorCallbackGlobal(int code,GSY_ConnectionHwnd hwnd);
	static void OnStreamOpenCallbackGlobal(GSY_ConnectionHwnd,GSY_StreamId,ErrorCode);
	static void OnStreamCloseCallbackGlobal(GSY_ConnectionHwnd,GSY_StreamId,ErrorCode);
public:
	FOnErrorDelegate OnError;
	
private:
	std::unordered_map<GSY_ConnectionHwnd,std::shared_ptr<BaseConnect>> ConnectionMap;
	GSY_Context Context = {};
	GSY_BaseConnectionContext ConnectionContext = {};
	bool bInitialized:1 = false;
};
