// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "RotationSpeedTrait.generated.h"

/**
 * 
 */
UCLASS()
class TESTMASS_API URotationSpeedTrait : public UMassEntityTraitBase
{
    GENERATED_BODY()

public:
    virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
