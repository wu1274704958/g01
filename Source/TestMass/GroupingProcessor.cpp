// Fill out your copyright notice in the Description page of Project Settings.


#include "GroupingProcessor.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "RotationSpeedFragment.h"
#include "CommonDefs.h"
#include "GroupIdFragment.h"

struct FTransformFragment;

UGroupingProcessor::UGroupingProcessor()
{
    bAutoRegisterWithProcessingPhases = false;
    //ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
    //ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UGroupingProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FUnitGrouped>(EMassFragmentAccess::ReadWrite);
    EntityQuery.RegisterWithProcessor(*this);
}

void UGroupingProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context) {
        const auto count = Context.GetNumEntities();

        auto Transforms = Context.GetFragmentView<FTransformFragment>();
        auto Groups = Context.GetMutableFragmentView<FUnitGrouped>();

        for (int32 i = 0; i < count; ++i)
        {
            auto Transform = Transforms[i].GetTransform();
            auto& Group = Groups[i];

            Group.GroupId = GetGroupIdByTransform(Transform);
            Group.Position = Transform.GetLocation();
        }
    });
}

uint32_t UGroupingProcessor::GetGroupIdByTransform(const FTransform& Transform) const
{
    const auto& pos = Transform.GetLocation();
    auto y = pos.Z - UnitYOffset;
    auto id =  FMath::FloorToInt32(FMath::Abs(y) / UnitSpace);
   
    if (id < 0 || id >= UnitGroupCount)
        id = 0;
    return id;
}


