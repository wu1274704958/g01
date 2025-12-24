// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "example1/example1.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnConnectErrorDelegate,int,GSY_ConnectionHwnd);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnConnectedDelegate,int,GSY_ConnectionHwnd);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDisconnectedDelegate,int,GSY_ConnectionHwnd);

class AUMqasNetAgent;

/**
 * 
 */
class MQASNET_API BaseConnect
{
	friend AUMqasNetAgent; 
public:
	BaseConnect();
	~BaseConnect();
protected:
	void InitData(GSY_ConnectionHwnd Hwnd,GSY_EngineId EngineId, const FString& ConfigFile,
		const FString& Ip, short Port);
	void OnConnected(int Code);
	void OnDisconnect(int Code);
	void OnConnectError(int Code);
public: //public methods
	GSY_ConnectionHwnd GetHwnd() const { return _Hwnd; }
	GSY_EngineId GetEngineId() const { return _EngineId; }
	const FString& GetConfigFile() const { return _ConfigFile; }
	const FString& GetIp() const { return _Ip; }
	short GetPort() const { return _Port; }
	bool IsConnected() const { return bIsConnected; }
	
public:	//signal
	FOnConnectErrorDelegate OnConnectErrorDelegate;
	FOnConnectedDelegate OnConnectedDelegate;
	FOnDisconnectedDelegate OnDisconnectedDelegate;
protected:
	GSY_EngineId _EngineId;
	FString _ConfigFile;
	FString _Ip;
	short _Port;
	GSY_ConnectionHwnd _Hwnd = InvalidConnection;
	bool bIsConnected:1 = false;
};
