// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Actor/SoundVisualizationMgrActor.h"
#include "Components/ActorComponent.h"
#include "music_player/music_player.h"
#include "soundVisualizationMgr/ISoundVisualizationMgr.h"
#include "SoundVisualizationComp.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class G01_API USoundVisualizationComp : public UActorComponent,public eqd_mp::SoundVisualizationListener
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USoundVisualizationComp();

protected:
    void RegisterSelf();
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    virtual void onRegisted(eqd_mp::EFFTDataType type, size_t dataSize) override;
    virtual void onUnRegisted() override;
    virtual void onVisualizationData(const std::span<float>& data) override;
    virtual eqd_mp::EFFTDataType GetFFTDataType() const;

protected:
    UPROPERTY(EditAnywhere,Category="Sound Visualization")
    ASoundVisualizationMgrActor* m_soundVisualizationMgr;

    UPROPERTY(EditAnywhere,Category="Sound Visualization")
    bool AutoRegister;
};
