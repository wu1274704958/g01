#pragma once
#include "gsync/example1/lobby.h"

enum class RID : GSY_RequestId
{
	FetchPeerList  = 1,
	RequestConnect = 2,
};
