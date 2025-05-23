// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "RotationSpeedInitialProcessor.generated.h"

/**
 * 
 */
UCLASS()
class TESTMASS_API URotationSpeedInitialProcessor : public UMassProcessor
{
    GENERATED_BODY()
    URotationSpeedInitialProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
protected:
    FMassEntityQuery EntityQuery;
};
