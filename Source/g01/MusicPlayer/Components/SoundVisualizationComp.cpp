// Fill out your copyright notice in the Description page of Project Settings.


#include "MusicPlayer/Components/SoundVisualizationComp.h"
#include "../Actor/SoundVisualizationMgrActor.h"

// Sets default values for this component's properties
USoundVisualizationComp::USoundVisualizationComp(): m_soundVisualizationMgr(nullptr), AutoRegister(false)
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = false;

    // ...
}

void USoundVisualizationComp::RegisterSelf()
{
    if (m_soundVisualizationMgr != nullptr)
    {
        m_soundVisualizationMgr->RegisterSoundVisualizationComp(GetFFTDataType(),*this);
    }
}


// Called when the game starts
void USoundVisualizationComp::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if (AutoRegister)
	    RegisterSelf();
}


// Called every frame
void USoundVisualizationComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USoundVisualizationComp::onRegisted(eqd_mp::EFFTDataType type, size_t dataSize)
{
    SoundVisualizationListener::onRegisted(type, dataSize);
    
}

void USoundVisualizationComp::onUnRegisted()
{
    SoundVisualizationListener::onUnRegisted();
    
}

void USoundVisualizationComp::onVisualizationData(const std::span<float>& data)
{
    SoundVisualizationListener::onVisualizationData(data);
    
}

eqd_mp::EFFTDataType USoundVisualizationComp::GetFFTDataType() const
{
    return eqd_mp::EFFTDataType::FFT256;
}

