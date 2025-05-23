// Fill out your copyright notice in the Description page of Project Settings.


#include "RotationSpeedTrait.h"

#include "MassEntityTemplateRegistry.h"
#include "RotationSpeed.h"

void URotationSpeedTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
    BuildContext.AddFragment<FRotationSpeed>();	
}
