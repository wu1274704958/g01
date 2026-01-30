#pragma once
#include "CoreMinimal.h"
#include "Logging/LogVerbosity.h"


struct MQASNET_API FLogCategoryLogMqasNet : public FLogCategory<ELogVerbosity::Log, ELogVerbosity::All>
{
	__forceinline FLogCategoryLogMqasNet() : FLogCategory(L"LogMqasNet")
	{
	}
};

extern MQASNET_API FLogCategoryLogMqasNet LogMqasNet;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnErrorDelegate,const FString&,int);