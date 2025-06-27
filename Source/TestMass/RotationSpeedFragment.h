// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "RotationSpeedFragment.generated.h"

/**
 * Represents the rotational speed of an entity within the mass framework.
 * This struct is used as a fragment in the mass system to store and manage rotational speed data.
 * The rotational speed is represented as a vector, indicating the speed along each axis.
 */
USTRUCT()
struct TESTMASS_API FRotationSpeedFragment : public FMassFragment
{
    GENERATED_BODY()
    FVector Speed;
    bool Active = true;
};

