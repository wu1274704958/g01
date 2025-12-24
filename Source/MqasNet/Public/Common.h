#pragma once
#include "CoreMinimal.h"
#include "Logging/LogVerbosity.h"


DECLARE_LOG_CATEGORY_EXTERN(LogMqasNet, Log, All);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnErrorDelegate,const FString&,int);