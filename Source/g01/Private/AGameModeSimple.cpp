// Fill out your copyright notice in the Description page of Project Settings.


#include "AGameModeSimple.h"
#include "g01/T1Character.h"

AAGameModeSimple::AAGameModeSimple(const FObjectInitializer& ObjectInitializer) : AGameModeBase(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerCharacterClass(TEXT("/Game/Characters/BP_T1Character"));
	if(PlayerCharacterClass.Class)
	{
		DefaultPawnClass = PlayerCharacterClass.Class;
	}
}
