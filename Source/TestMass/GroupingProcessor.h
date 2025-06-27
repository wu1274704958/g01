// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "UObject/Object.h"
#include "GroupingProcessor.generated.h"

/**
 * 
 */
UCLASS()
class TESTMASS_API UGroupingProcessor : public UMassProcessor
{
    GENERATED_BODY()
    public:
    UGroupingProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
    uint32_t GetGroupIdByTransform(const FTransform& Transform) const;

private:
    FMassEntityQuery EntityQuery;
};
