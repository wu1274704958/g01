// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityEQSSpawnPointsGenerator.h"
#include "RotationSpeedDataGenerator.generated.h"

/**
 * 
 */
UCLASS()
class TESTMASS_API URotationSpeedDataGenerator : public UMassEntityEQSSpawnPointsGenerator
{
    GENERATED_BODY()

public:
    void OnSpawnDataGenerationFinished(TArrayView<const FMassEntitySpawnDataGeneratorResult> Results,
                                                    FFinishedGeneratingSpawnDataSignature OriginDelegate) const;
    virtual void Generate(UObject& QueryOwner, TConstArrayView<FMassSpawnedEntityType> EntityTypes, int32 Count,
                          FFinishedGeneratingSpawnDataSignature& FinishedGeneratingSpawnPointsDelegate) const override;
};
