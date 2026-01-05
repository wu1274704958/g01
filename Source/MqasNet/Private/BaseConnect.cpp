// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseConnect.h"


BaseConnect::BaseConnect()
{
}

BaseConnect::~BaseConnect()
{
}

void BaseConnect::InitData(GSY_ConnectionHwnd Hwnd,GSY_EngineId EngineId, const FString& ConfigFile, const FString& Ip, short Port)
{
	this->_Hwnd = Hwnd;
	this->_EngineId = EngineId;
	this->_ConfigFile = std::move(ConfigFile);
	this->_Ip = std::move(Ip);
	this->_Port = Port;
}

void BaseConnect::OnConnected(int Code)
{
	bIsConnected = Code == EC_Ok ? true : false;
	OnConnectedDelegate.Broadcast(Code, _Hwnd);
}

void BaseConnect::OnDisconnect(int Code)
{
	bIsConnected = false;
	OnDisconnectedDelegate.Broadcast(Code, _Hwnd);
}

void BaseConnect::OnConnectError(int Code)
{
	bIsConnected = false;
	OnConnectErrorDelegate.Broadcast(Code, _Hwnd);
}

void BaseConnect::Disconnect()
{
	if (IsConnected())
	{
		if (GSY_disconnect(_Hwnd) == EC_Ok)
		{
			bIsConnected = false;
		}
	}
}

