// Fill out your copyright notice in the Description page of Project Settings.


#include "RotationSpeedInitialProcessor.h"

#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "RotationSpeedFragment.h"

URotationSpeedInitialProcessor::URotationSpeedInitialProcessor()
{
    bAutoRegisterWithProcessingPhases = false;
}

void URotationSpeedInitialProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FRotationSpeedFragment>(EMassFragmentAccess::ReadWrite); // 旋转速度
    EntityQuery.RegisterWithProcessor(*this);
}

void URotationSpeedInitialProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& Context) {
        const int32 NumEntities = Context.GetNumEntities();

        TArrayView<FRotationSpeedFragment> RotationSpeeds = Context.GetMutableFragmentView<FRotationSpeedFragment>();
        
        for (int32 i = 0; i < NumEntities; ++i)
        {
            RotationSpeeds[i].Speed = FVector( FMath::RandRange(0.f,360.0f),FMath::RandRange(0.f,360.0f),FMath::RandRange(0.f,360.0f));
        }
    });

    
}
