// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseStream.h"

BaseStream::~BaseStream()
{
	_streamId = INVALID_SID;
}

void BaseStream::InitData(GSY_StreamId streamId)
{
	_streamId = streamId;
	_isAlive = true;
}

void BaseStream::OnClose()
{
	_isAlive = false;
}
