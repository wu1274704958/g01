#pragma once

#include <vector>

#include "MassProcessor.h"
#include "soundVisualizationMgr/ISoundVisualizationMgr.h"
#include "SoundVisualProcessor.generated.h"

class ASoundVisualizationMgrActor;

UCLASS()
class USoundVisualProcessor: public UMassProcessor,public eqd_mp::SoundVisualizationListener
{
public:
    virtual void onUnRegisted() override;

    virtual void onRegisted(eqd_mp::EFFTDataType type, size_t dataSize) override;
    virtual void onVisualizationData(const std::span<float>& data) override;

private:
    GENERATED_BODY()

public:
    USoundVisualProcessor();

protected:
    virtual void ConfigureQueries() override;
    void RegisterSelf();
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
    void UpdateUnit(uint32_t Uint32,FTransform& Transform,FVector Pos);
    float GetValueByGroup(uint32_t Id,uint32_t Num) const;

public:
    virtual void BeginDestroy() override;

private:
    FMassEntityQuery EntityQuery;
    std::atomic_bool IsRegistered = false;
    std::atomic_bool RegistrationPending = false;
    std::atomic_bool UpdatingFFTData = false;
    std::vector<float> FFTData;
    std::vector<float> LastProcessedData;
    std::atomic<ASoundVisualizationMgrActor*> SoundVisualizationMgr = nullptr;
};
