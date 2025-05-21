// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundVisualizationMgrActor.h"
#include "../Components/SoundVisualizationComp.h"


// Sets default values
ASoundVisualizationMgrActor::ASoundVisualizationMgrActor(): musicPlayer(nullptr)
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASoundVisualizationMgrActor::BeginPlay()
{
    Super::BeginPlay();
    
}


void ASoundVisualizationMgrActor::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    InitVisualizationMgr();
}

void ASoundVisualizationMgrActor::BeginDestroy()
{
    Super::BeginDestroy();

    UninitVisualizationMgr();
}
// Called every frame
void ASoundVisualizationMgrActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (m_soundVisualizationMgr)
        m_soundVisualizationMgr->tick();
    // ...
}

void ASoundVisualizationMgrActor::InitVisualizationMgr()
{
    if (musicPlayer != nullptr && musicPlayer->GetPlayer() != nullptr)
    {
        m_soundVisualizationMgr = std::make_shared<SoundVisualizationMgr>(*const_cast<MusicPlayer*>(musicPlayer->GetPlayer().get()));
    }
}

void ASoundVisualizationMgrActor::UninitVisualizationMgr()
{
    if (m_soundVisualizationMgr)
    {
        m_soundVisualizationMgr.reset();
    }
        
}

bool ASoundVisualizationMgrActor::RegisterSoundVisualizationComp(eqd_mp::EFFTDataType type,
    USoundVisualizationComp& soundVisualizationComponent) const
{
    if (m_soundVisualizationMgr == nullptr)
        return false;
    return m_soundVisualizationMgr->registeListener(type,soundVisualizationComponent);
}

