// Fill out your copyright notice in the Description page of Project Settings.


#include "RotationSpeedDataGenerator.h"

#include "MassSpawnLocationProcessor.h"
#include "RotationSpeedInitialProcessor.h"
#include "CommonDefs.h"
#include "GroupingProcessor.h"

void URotationSpeedDataGenerator::OnSpawnDataGenerationFinished(
    TArrayView<FMassEntitySpawnDataGeneratorResult> Results,
    FFinishedGeneratingSpawnDataSignature OriginDelegate) const
{
    for (FMassEntitySpawnDataGeneratorResult& Result : Results)
    {
        Result.PostSpawnProcessors.Push(URotationSpeedInitialProcessor::StaticClass());
        Result.PostSpawnProcessors.Push(UGroupingProcessor::StaticClass());

        Result.SpawnDataProcessor = UMassSpawnLocationProcessor::StaticClass();
        Result.SpawnData.InitializeAs<FMassTransformsSpawnData>();
        FMassTransformsSpawnData& Transforms = Result.SpawnData.GetMutable<FMassTransformsSpawnData>();

        Transforms.Transforms.Reserve(Result.NumEntities);
        for (int i = 0; i < Result.NumEntities; i++)
        {
            FTransform& Transform = Transforms.Transforms.AddDefaulted_GetRef();
            FVector Direction = FMath::VRand();
            Transform.SetLocation(Direction * UnitRadius + FVector(0,0,UnitYOffset));
        }
    }
    
    OriginDelegate.Execute(Results);
}

void URotationSpeedDataGenerator::Generate(UObject& QueryOwner, TConstArrayView<FMassSpawnedEntityType> EntityTypes,
                                           int32 Count, FFinishedGeneratingSpawnDataSignature& FinishedGeneratingSpawnPointsDelegate) const
{

    if (Count <= 0)
    {
        FinishedGeneratingSpawnPointsDelegate.Execute(TArray<FMassEntitySpawnDataGeneratorResult>());
        return;
    }

    TArray<FMassEntitySpawnDataGeneratorResult> Results;
    BuildResultsFromEntityTypes(Count, EntityTypes, Results);
    
    FGeneratingSpawnDataSignature Delegate = FGeneratingSpawnDataSignature::CreateUObject(this, &URotationSpeedDataGenerator::OnSpawnDataGenerationFinished,
        FinishedGeneratingSpawnPointsDelegate);
    Delegate.Execute(Results);
}
