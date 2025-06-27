#include "SoundVisualProcessor.h"

#include "EngineUtils.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MusicPlayer/Actor/SoundVisualizationMgrActor.h"
#include "TestMass/GroupIdFragment.h"
#include "TestMass/CommonDefs.h"

void USoundVisualProcessor::onUnRegisted()
{
    SoundVisualizationListener::onUnRegisted();
    SoundVisualizationMgr.store(nullptr,std::memory_order_release);
}

void USoundVisualProcessor::onRegisted(eqd_mp::EFFTDataType type, size_t dataSize)
{
    UpdatingFFTData.store(true,std::memory_order_release);
    FFTData.resize(dataSize);
    LastProcessedData.resize(UnitGroupCount);
    IsRegistered = true;
    UpdatingFFTData.store(false,std::memory_order_release);
}

void USoundVisualProcessor::onVisualizationData(const std::span<float>& data)
{
    UpdatingFFTData.store(true,std::memory_order_release);
    std::memcpy(FFTData.data(), data.data(), data.size() * sizeof(float));
    UpdatingFFTData.store(false,std::memory_order_release);
}

USoundVisualProcessor::USoundVisualProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void USoundVisualProcessor::ConfigureQueries()
{
    
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FUnitGrouped>(EMassFragmentAccess::ReadWrite);
    EntityQuery.RegisterWithProcessor(*this);
}

void USoundVisualProcessor::RegisterSelf()
{
    RegistrationPending = true;
    ExecuteOnGameThread(TEXT("USoundVisualProcessor::RegisterSelf"),[this]()
    {
        auto Mgr = SoundVisualizationMgr.load(std::memory_order_acquire);
        if (Mgr != nullptr)
        {
            if (Mgr->GetInner())
                Mgr->GetInner()->registeListener(eqd_mp::EFFTDataType::FFT256,*this);
        }else
        {
            UWorld* World = GetWorld();
            if (World == nullptr)
                return;
            for (TActorIterator<ASoundVisualizationMgrActor> It(World, ASoundVisualizationMgrActor::StaticClass()); It; ++It)
            {
                ASoundVisualizationMgrActor* Actor = *It;
                SoundVisualizationMgr.store(Actor,std::memory_order_release);
                if (Actor->GetInner())
                    Actor->GetInner()->registeListener(eqd_mp::EFFTDataType::FFT256,*this);
                break;
            }
        }
        RegistrationPending = false;
    });
}

void USoundVisualProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    if (!IsRegistered)
    {
        if (!RegistrationPending)
            RegisterSelf();
        return;
    }
    if (FFTData.empty())
        return;
    while (UpdatingFFTData.load(std::memory_order_acquire))
        ;
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context) {
        const auto Count = Context.GetNumEntities();

        auto RotateFragment = Context.GetMutableFragmentView<FTransformFragment>();
        auto Groups = Context.GetMutableFragmentView<FUnitGrouped>();

        for (int32 i = 0; i < Count; ++i)
        {
            if (Groups[i].GroupId >= 0)
                UpdateUnit(Groups[i].GroupId,RotateFragment[i].GetMutableTransform(),Groups[i].Position);
        }   
    });
}

void USoundVisualProcessor::UpdateUnit(uint32_t ID, FTransform& Transform,FVector Pos)
{
    auto Value = GetValueByGroup(ID,UnitGroupCount);

    FVector Origin(0,0,UnitYOffset);

    auto Dir = (Pos - Origin).GetSafeNormal();
    Dir.Z = 0;

    auto NewPos = Pos + Dir * sqrtf(Value) * 3000.0;

    Transform.SetLocation(NewPos);
    
    LastProcessedData[ID] = Value;
}

float USoundVisualProcessor::GetValueByGroup(uint32_t Id,uint32_t Num) const
{
    auto BatchCount = FFTData.size() / Num;
    float Value = 0;
    for (int32 i = 0; i < BatchCount; ++i)
    {
        Value += FFTData[Id * BatchCount + i];
    }
    return Value / BatchCount;
}

void USoundVisualProcessor::BeginDestroy()
{
    Super::BeginDestroy();
    auto mgr = SoundVisualizationMgr.load(std::memory_order_acquire);
    if (mgr != nullptr && mgr->GetInner())
        mgr->GetInner()->unregisteListener(*this);
    SoundVisualizationMgr.store(nullptr,std::memory_order_release);
}
