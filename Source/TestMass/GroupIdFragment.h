#pragma once
#include "MassEntityTypes.h"
#include "GroupIdFragment.generated.h"


USTRUCT()
struct TESTMASS_API FUnitGrouped : public FMassFragment
{
    GENERATED_BODY()
    int32_t GroupId = -1;
    FVector Position = FVector::ZeroVector;
};
