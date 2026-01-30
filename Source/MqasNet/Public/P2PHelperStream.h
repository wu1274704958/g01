#pragma once
#include "BaseStream.h"

class P2PHelperStream : public BaseStream
{
	using ContextType = GSY_LobbyStreamContext;
public:
	P2PHelperStream():BaseStream(){}
	const ContextType* GetContext() const {return &_Context;}
protected:
	ContextType _Context = {};
};
