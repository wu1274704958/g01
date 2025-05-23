// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "RotationProcessor.generated.h"

/**
 * 
 */
UCLASS()
class TESTMASS_API URotationProcessor : public UMassProcessor
{
    GENERATED_BODY()
public:
    URotationProcessor();
protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
};
