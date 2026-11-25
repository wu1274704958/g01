#pragma once
#include "Common.h"


template<typename ST,typename CT,typename... Args>
requires IsVaildStream<ST,CT>
std::shared_ptr<ST> BaseConnect::MakeStream(GSY_StreamId(* req_func)(GSY_ConnectionHwnd,Args...,CT*) ,Args&&... args)
{
	const auto Hwnd = GetHwnd();
	if (!IsConnected() || Hwnd == InvalidConnection)
	{
		UE_LOG(LogMqasNet,Warning,TEXT("MakeStream failed: not connected, hwnd=%u"), Hwnd);
		return nullptr;
	}
	const auto stream = std::make_shared<ST>();
	const CT* context = stream->GetContext();
	GSY_StreamId stream_id = req_func(Hwnd,std::forward<Args>(args)...,const_cast<CT*>(context));
	if (stream_id == INVALID_SID)
	{
		UE_LOG(LogMqasNet,Warning,TEXT("MakeStream failed: request stream failed, hwnd=%u"), _Hwnd);
		return nullptr;
	}
	return AttachStream(stream_id, stream) ? stream : nullptr;
}


