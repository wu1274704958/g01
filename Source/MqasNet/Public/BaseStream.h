// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "example1/example1.h"


class MQASNET_API BaseStream
{
	friend class BaseConnect;
public:
	BaseStream() = default; 
	virtual ~BaseStream();
	GSY_StreamId GetStreamId() const { return _streamId; }
	GSY_ConnectionHwnd GetHwnd() const { return _hwnd; }
	bool Valid() const { return _streamId != INVALID_SID; }
	virtual void InitData(GSY_StreamId streamId, GSY_ConnectionHwnd hwnd);
protected:
	virtual void OnClose();
	
protected:
	GSY_StreamId _streamId = INVALID_SID;
	GSY_ConnectionHwnd _hwnd = InvalidConnection;
	bool _isAlive:1 = false;
};

template<typename T,typename CT>
concept IsVaildStream = requires(T& stream)
{
	new T();
	requires std::is_base_of_v<BaseStream, T>;
	requires std::is_same_v<typename T::ContextType, CT>;
	requires std::is_same_v<decltype(stream.GetContext()),const CT*>;
};
