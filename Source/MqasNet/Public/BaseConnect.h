// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include <memory>
#include <unordered_map>
#include <functional>
#include <type_traits>
#include "CoreMinimal.h"
#include "BaseStream.h"
#include "gsync/core/connection.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnConnectErrorDelegate,int,GSY_ConnectionHwnd);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnConnectedDelegate,int,GSY_ConnectionHwnd);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDisconnectedDelegate,int,GSY_ConnectionHwnd);

class AUMqasNetAgent;
class FMqasAgent;

/**
 * 
 */
class MQASNET_API BaseConnect : public std::enable_shared_from_this<BaseConnect>
{
	friend AUMqasNetAgent;
	friend class FMqasAgent;
	friend struct FMqasServerState;
public:
	BaseConnect();
	virtual ~BaseConnect();
protected:
	void InitData(GSY_ConnectionHwnd Hwnd,GSY_EngineId EngineId, const FString& ConfigFile,
		const FString& Ip, short Port);
	void OnConnected(int Code);
	void OnDisconnect(int Code);
	void OnConnectError(int Code);
	void OnStreamOpen(GSY_StreamId sid,ErrorCode Code);
	void OnStreamClose(GSY_StreamId sid,ErrorCode Code);
	void Invalidate();
    // Synchronous SDK callbacks. Overrides must not access game-thread state.
    virtual void OnNativeStreamOpen(GSY_ConnectionHwnd, GSY_StreamId, ErrorCode) {}
    virtual void OnNativeStreamClose(GSY_ConnectionHwnd, GSY_StreamId, ErrorCode) {}
    bool AttachStream(GSY_StreamId, const std::shared_ptr<BaseStream>&);
public: //public methods
	virtual GSY_ConnectionHwnd GetHwnd() const { return _Hwnd; }
	// Endpoint metadata is populated for connections created with GSY_connect;
	// specialized connections can have no corresponding engine or endpoint.
	GSY_EngineId GetEngineId() const { return _EngineId; }
	const FString& GetConfigFile() const { return _ConfigFile; }
	const FString& GetIp() const { return _Ip; }
	short GetPort() const { return _Port; }
	virtual bool IsConnected() const { return bIsConnected; }
	virtual void Disconnect();
	template<typename ST,typename CT,typename... Args>
	requires IsVaildStream<ST,CT>
	std::shared_ptr<ST> MakeStream(GSY_StreamId(* req_func)(GSY_ConnectionHwnd,Args...,CT*),Args&&... args);
	
    using FStreamCreated = void(*)(ErrorCode, GSY_StreamId, GSY_RequestId, void*);
    using FStreamCompletion = std::function<void(ErrorCode, GSY_StreamId, GSY_RequestId)>;
    // The starter must complete exactly once on EC_Pending, even on cancellation.
    // The stream owns its native Context. Completion is delivered on the game thread.
    template<class ST, class Start>
    requires std::is_base_of_v<BaseStream, ST>
    ErrorCode MakeStream(const std::shared_ptr<ST>& Stream, Start&& Starter, FStreamCompletion Completion, FStreamCompletion NativeCompletion = {})
    {
        return MakeStreamAsync(Stream, [&](GSY_ConnectionHwnd Handle, FStreamCreated Created, void* User) {
            return std::invoke(Starter, Handle, Stream->GetContext(), Created, User);
        }, std::move(Completion), std::move(NativeCompletion));
    }
    std::shared_ptr<BaseStream> FindStream(GSY_StreamId Id) const;
    // Called synchronously by the native stream-open callback before protocol
    // dispatch. Binder installs the stream's native Context on the I/O thread.
    using FBindIncomingStream = std::function<ErrorCode(GSY_ConnectionHwnd, GSY_StreamId)>;
    ErrorCode BindIncomingStream(GSY_ConnectionHwnd Handle, GSY_StreamId Id,
        const std::shared_ptr<BaseStream>& Stream, const FBindIncomingStream& Binder);
private:
    using FStartStream = std::function<ErrorCode(GSY_ConnectionHwnd, FStreamCreated, void*)>;
    ErrorCode MakeStreamAsync(const std::shared_ptr<BaseStream>&, const FStartStream&, FStreamCompletion, FStreamCompletion);

public:	//signal
	FOnConnectErrorDelegate OnConnectErrorDelegate;
	FOnConnectedDelegate OnConnectedDelegate;
	FOnDisconnectedDelegate OnDisconnectedDelegate;
protected:
	GSY_EngineId _EngineId = 0;
	FString _ConfigFile;
	FString _Ip;
	short _Port = 0;
	GSY_ConnectionHwnd _Hwnd = InvalidConnection;
	std::unordered_map<GSY_StreamId,std::shared_ptr<BaseStream>> _StreamMap;
	bool bIsConnected:1 = false;
};

#include "BaseConnect.hpp"
