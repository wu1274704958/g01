// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalMusicPlayerActor.h"
#include "GameFramework/Actor.h"
#include "music_player/music_player.h"
#include "soundVisualizationMgr/ISoundVisualizationMgr.h"
#include "SoundVisualizationMgrActor.generated.h"

UCLASS()
class G01_API ASoundVisualizationMgrActor : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ASoundVisualizationMgrActor();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    virtual void BeginDestroy() override;

    virtual void PostInitializeComponents() override;

    // Called every frame
    virtual void Tick(float DeltaTime) override;
    bool RegisterSoundVisualizationComp(eqd_mp::EFFTDataType type,class USoundVisualizationComp& soundVisualizationComponent) const;
    std::shared_ptr<SoundVisualizationMgr> GetInner() const;
protected:
    void InitVisualizationMgr();
    void UninitVisualizationMgr();

protected:
    UPROPERTY(EditAnywhere,Category="Sound Visualization")
    AGlobalMusicPlayerActor* musicPlayer;
    std::shared_ptr<SoundVisualizationMgr> m_soundVisualizationMgr;
};
