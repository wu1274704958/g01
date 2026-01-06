// Fill out your copyright notice in the Description page of Project Settings.


#include "UIUtility.h"

#include "IToast.h"
#include "Log.h"
#include "ScreenPass.h"

#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanelSlot.h"

int UIUtility::CurrentToastZOrder = UIUtility::ToastZOrder;

void UIUtility::ShowToast(UObject* obj,int PlayerIndex,const TSubclassOf<UUserWidget> Class, const FString& Message,
                           float Duration, float XOffset, float YOffset)
{
	if (!Class->ImplementsInterface(UIToast::StaticClass()))
	{
		UE_LOG(LogUIUtility, Error, TEXT("Toast must be implement IToast interface"));
		return;
	}
	auto World = obj->GetWorld();

	auto playerCount = World->GetNumPlayerControllers();
	if (PlayerIndex >= playerCount)
	{
		UE_LOG(LogUIUtility, Error, TEXT("PlayerIndex %d is out of range,player count is %d"), PlayerIndex, playerCount);
		return;
	}
	// Get the player controller
	auto PlayerController = (World->GetPlayerControllerIterator() + PlayerIndex)->Get();
	
	auto Widget = CreateWidget(World,Class);

	int32 width = 0, height = 0;
	PlayerController->GetViewportSize(width, height);
	
	//Widget->SetPositionInViewport(FVector2D(0, 0));
	Widget->SetAnchorsInViewport(FAnchors(0.5,0.5));
	Widget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
	Widget->SetPositionInViewport(FVector2D(XOffset * width, YOffset * height));
	
	IIToast::Execute_Show(Widget, Message);
	Widget->AddToViewport(CurrentToastZOrder++);
	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(TimerHandle, [Widget]()
		{
			if (Widget)
			{
				IIToast::Execute_Hide(Widget);
			}
		}, Duration, false);
}
