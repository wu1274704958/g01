// Fill out your copyright notice in the Description page of Project Settings.


#include "RotationProcessor.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "RotationSpeed.h"

URotationProcessor::URotationProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void URotationProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite); // 位置
    EntityQuery.AddRequirement<FRotationSpeed>(EMassFragmentAccess::ReadOnly); // 旋转速度
    EntityQuery.RegisterWithProcessor(*this);
}

void URotationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& Context) {
        const int32 NumEntities = Context.GetNumEntities();

        // 获取组件数组
        const TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<const FRotationSpeed> RotationSpeeds = Context.GetFragmentView<FRotationSpeed>();

        // 遍历每个实体
        for (int32 i = 0; i < NumEntities; ++i)
        {
            FTransform& Transform = Transforms[i].GetMutableTransform();
            const FVector Speed = RotationSpeeds[i].Speed;

            // 更新旋转（绕Z轴）
            Transform.ConcatenateRotation(FRotator(Speed.X,Speed.Y,Speed.Z).Quaternion());
            Transform.NormalizeRotation();
        }
    });
}

