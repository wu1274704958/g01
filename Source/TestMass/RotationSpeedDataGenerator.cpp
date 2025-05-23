// Fill out your copyright notice in the Description page of Project Settings.


#include "RotationSpeedDataGenerator.h"

#include "RotationSpeedInitialProcessor.h"

void URotationSpeedDataGenerator::OnSpawnDataGenerationFinished(
    TArrayView<const FMassEntitySpawnDataGeneratorResult> Results,
    FFinishedGeneratingSpawnDataSignature OriginDelegate) const
{
    for (int32 i = 0; i < Results.Num(); i++)
    {
        const_cast<TArray<TSubclassOf<UMassProcessor>>*>(&Results[i].PostSpawnProcessors)->Push(URotationSpeedInitialProcessor::StaticClass());
    }
    OriginDelegate.Execute(Results);
}

void URotationSpeedDataGenerator::Generate(UObject& QueryOwner, TConstArrayView<FMassSpawnedEntityType> EntityTypes,
                                           int32 Count, FFinishedGeneratingSpawnDataSignature& FinishedGeneratingSpawnPointsDelegate) const
{
    FFinishedGeneratingSpawnDataSignature Delegate = FFinishedGeneratingSpawnDataSignature::CreateUObject(this, &URotationSpeedDataGenerator::OnSpawnDataGenerationFinished,
        FinishedGeneratingSpawnPointsDelegate);
    UMassEntityEQSSpawnPointsGenerator::Generate(QueryOwner, EntityTypes, Count, Delegate);
}
