// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseStream.h"

BaseStream::~BaseStream()
{
	_streamId = INVALID_SID;
}

void BaseStream::InitData(GSY_StreamId streamId, GSY_ConnectionHwnd hwnd)
{
	_streamId = streamId;
	_hwnd = hwnd;
	_isAlive = true;
}

void BaseStream::OnClose()
{
	_isAlive = false;
}
