#include "TestGroupProcessor.h"

#include "CommonDefs.h"
#include "GroupIdFragment.h"
#include "MassCommonTypes.h"
#include "MassDebuggerSubsystem.h"
#include "MassExecutionContext.h"
#include "MassGameplayDebugTypes.h"
#include "RotationSpeedFragment.h"

UTestGroupProcessor::UTestGroupProcessor()
{
    bAutoRegisterWithProcessingPhases = RunTestGroup;
    ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UTestGroupProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FRotationSpeedFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FUnitGrouped>(EMassFragmentAccess::ReadWrite);
    EntityQuery.RegisterWithProcessor(*this);
}

void UTestGroupProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context) {
        const auto count = Context.GetNumEntities();

        auto RotateFragment = Context.GetMutableFragmentView<FRotationSpeedFragment>();
        auto Groups = Context.GetMutableFragmentView<FUnitGrouped>();

        for (int32 i = 0; i < count; ++i)
        {
            if (Groups[i].GroupId == 0)
            {
                RotateFragment[i].Active = false;
            }
        }   
       
    });
}

