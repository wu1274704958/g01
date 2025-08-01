﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSpawner/Public/MassSpawner.h"
#include "RotateCubeSpawner.generated.h"

UCLASS()
class TESTMASS_API ARotateCubeSpawner : public AMassSpawner
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ARotateCubeSpawner();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;
};
