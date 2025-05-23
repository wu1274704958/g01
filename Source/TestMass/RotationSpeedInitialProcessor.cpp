// Fill out your copyright notice in the Description page of Project Settings.


#include "RotationSpeedInitialProcessor.h"

#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "RotationSpeed.h"

URotationSpeedInitialProcessor::URotationSpeedInitialProcessor()
{
    bAutoRegisterWithProcessingPhases = false;
}

void URotationSpeedInitialProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FRotationSpeed>(EMassFragmentAccess::ReadWrite); // 旋转速度
    EntityQuery.RegisterWithProcessor(*this);
}

void URotationSpeedInitialProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& Context) {
        const int32 NumEntities = Context.GetNumEntities();

        TArrayView<FRotationSpeed> RotationSpeeds = Context.GetMutableFragmentView<FRotationSpeed>();
        
        for (int32 i = 0; i < NumEntities; ++i)
        {
            RotationSpeeds[i].Speed = FMath::VRand();
        }
    });

    
}
