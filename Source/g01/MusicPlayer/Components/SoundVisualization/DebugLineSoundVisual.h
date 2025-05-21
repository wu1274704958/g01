// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MusicPlayer/Components/SoundVisualizationComp.h"
#include "DebugLineSoundVisual.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class G01_API UDebugLineSoundVisual : public USoundVisualizationComp
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UDebugLineSoundVisual();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:
    virtual void onVisualizationData(const std::span<float>& data) override;
    virtual void onRegisted(eqd_mp::EFFTDataType type, size_t dataSize) override;

    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;
    virtual eqd_mp::EFFTDataType GetFFTDataType() const override;

protected:
    std::vector<FVector> points;
    UPROPERTY(EditAnywhere)
    float BaseSize = 1.0f;
    UPROPERTY(EditAnywhere)
    float SizeScale = 1.0f;
    UPROPERTY(EditAnywhere)
    FColor BaseColor = FColor::Cyan;
    UPROPERTY(EditAnywhere)
    float BaseDuration = 1.0f;
    UPROPERTY(EditAnywhere)
    float SphereRadius = 10.0f;
};
