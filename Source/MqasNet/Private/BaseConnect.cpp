// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseConnect.h"
#include "Async/Async.h"


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
	OnConnectedDelegate.Broadcast(Code, GetHwnd());
}

void BaseConnect::OnDisconnect(int Code)
{
	Invalidate();
	OnDisconnectedDelegate.Broadcast(Code, GetHwnd());
	_Hwnd = InvalidConnection;
}

void BaseConnect::OnConnectError(int Code)
{
	Invalidate();
	OnConnectErrorDelegate.Broadcast(Code, GetHwnd());
	_Hwnd = InvalidConnection;
}

void BaseConnect::Invalidate()
{
	bIsConnected = false;
	auto Streams = std::move(_StreamMap);
	_StreamMap.clear();
	for (auto& [Id, Stream] : Streams)
		if (Stream) Stream->OnClose();
}

void BaseConnect::OnStreamOpen(GSY_StreamId sid, ErrorCode Code)
{
	
}

void BaseConnect::OnStreamClose(GSY_StreamId sid, ErrorCode Code)
{
	if (_StreamMap.contains(sid))
	{
		auto& stream = _StreamMap[sid];
		stream->OnClose();
		_StreamMap.erase(sid);
	}
}

void BaseConnect::Disconnect()
{
    const auto Handle = GetHwnd();
    if (Handle != InvalidConnection) GSY_disconnect(Handle);
}

namespace {
struct FPendingStreamCreation {
    std::weak_ptr<BaseConnect> Connection;
    std::shared_ptr<BaseStream> Stream;
    BaseConnect::FStreamCompletion Completion;
    BaseConnect::FStreamCompletion NativeCompletion;
};
}

ErrorCode BaseConnect::MakeStreamAsync(const std::shared_ptr<BaseStream>& Stream,
    const FStartStream& Start, FStreamCompletion Completion, FStreamCompletion NativeCompletion)
{
    check(IsInGameThread());
    const auto Handle = GetHwnd();
    if (!Stream || Stream->Valid() || !IsConnected() || Handle == InvalidConnection || weak_from_this().expired())
        return EC_WrongStreamState;

    auto* Pending = new FPendingStreamCreation{weak_from_this(), Stream, std::move(Completion), std::move(NativeCompletion)};
    const auto Code = Start(Handle, [](ErrorCode Result, GSY_StreamId Id, GSY_RequestId Rid, void* Opaque) {
        std::unique_ptr<FPendingStreamCreation> Request(static_cast<FPendingStreamCreation*>(Opaque));
        if (Request->NativeCompletion) Request->NativeCompletion(Result, Id, Rid);
        AsyncTask(ENamedThreads::GameThread, [Request = std::move(Request), Result, Id, Rid]() mutable {
            auto Connection = Request->Connection.lock();
            auto Code = Result;
            if (Code == EC_Ok) {
                if (!Connection || !Connection->IsConnected()) Code = EC_Disconnected;
                else if (!Connection->AttachStream(Id, Request->Stream)) Code = EC_InvalidStream;
            }
            if (Request->Completion) Request->Completion(Code, Code == EC_Ok ? Id : INVALID_SID, Rid);
        });
    }, Pending);
    if (Code != EC_Pending) delete Pending;
    return Code;
}

bool BaseConnect::AttachStream(GSY_StreamId Id, const std::shared_ptr<BaseStream>& Stream)
{
    check(IsInGameThread());
    if (Id == INVALID_SID || !_StreamMap.emplace(Id, Stream).second) return false;
    Stream->InitData(Id, GetHwnd());
    return true;
}

std::shared_ptr<BaseStream> BaseConnect::FindStream(GSY_StreamId Id) const
{
    check(IsInGameThread());
    const auto It = _StreamMap.find(Id);
    return It == _StreamMap.end() ? nullptr : It->second;
}

ErrorCode BaseConnect::BindIncomingStream(GSY_ConnectionHwnd Handle, GSY_StreamId Id,
    const std::shared_ptr<BaseStream>& Stream, const FBindIncomingStream& Binder)
{
    if (!Stream || !Binder || Id == INVALID_SID) return EC_InvalidStream;
    const auto Code = Binder(Handle, Id);
    if (Code == EC_Ok) {
        auto Connection = shared_from_this();
        AsyncTask(ENamedThreads::GameThread, [Connection, Stream, Handle, Id] {
            if (Connection->GetHwnd() == Handle) Connection->AttachStream(Id, Stream);
        });
    }
    return Code;
}
