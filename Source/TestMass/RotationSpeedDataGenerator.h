// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntitySpawnDataGeneratorBase.h"
#include "RotationSpeedDataGenerator.generated.h"


DECLARE_DELEGATE_OneParam(FGeneratingSpawnDataSignature, TArrayView<FMassEntitySpawnDataGeneratorResult>);
/**
 * 
 */
UCLASS()
class TESTMASS_API URotationSpeedDataGenerator : public UMassEntitySpawnDataGeneratorBase
{
    GENERATED_BODY()

public:
    void OnSpawnDataGenerationFinished(TArrayView<FMassEntitySpawnDataGeneratorResult> Results,
                                                    FFinishedGeneratingSpawnDataSignature OriginDelegate) const;
    virtual void Generate(UObject& QueryOwner, TConstArrayView<FMassSpawnedEntityType> EntityTypes, int32 Count,
                          FFinishedGeneratingSpawnDataSignature& FinishedGeneratingSpawnPointsDelegate) const override;
};
