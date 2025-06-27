// Fill out your copyright notice in the Description page of Project Settings.


#include "RotationSpeedTrait.h"

#include "GroupIdFragment.h"
#include "MassEntityTemplateRegistry.h"
#include "RotationSpeedFragment.h"

void URotationSpeedTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
    BuildContext.AddFragment<FRotationSpeedFragment>();
    BuildContext.AddFragment<FUnitGrouped>();
}
