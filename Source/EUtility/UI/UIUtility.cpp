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
	if (!Class || !Class->ImplementsInterface(UIToast::StaticClass()))
	{
		UE_LOG(LogUIUtility, Error, TEXT("Toast must be implement IToast interface"));
		return;
	}
	auto* World = IsValid(obj) ? obj->GetWorld() : nullptr;
	if (!World || World->bIsTearingDown)
	{
		return;
	}

	auto playerCount = World->GetNumPlayerControllers();
	if (PlayerIndex < 0 || PlayerIndex >= playerCount)
	{
		UE_LOG(LogUIUtility, Error, TEXT("PlayerIndex %d is out of range,player count is %d"), PlayerIndex, playerCount);
		return;
	}
	// Get the player controller
	auto PlayerController = (World->GetPlayerControllerIterator() + PlayerIndex)->Get();
	if (!IsValid(PlayerController)) return;
	
	auto Widget = CreateWidget(World,Class);
	if (!IsValid(Widget)) return;

	int32 width = 0, height = 0;
	PlayerController->GetViewportSize(width, height);
	
	//Widget->SetPositionInViewport(FVector2D(0, 0));
	Widget->SetAnchorsInViewport(FAnchors(0.5,0.5));
	Widget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
	Widget->SetPositionInViewport(FVector2D(XOffset * width, YOffset * height));
	
	IIToast::Execute_Show(Widget, Message);
	Widget->AddToViewport(CurrentToastZOrder++);
	// UWorld uses its GameInstance's timer manager, which survives level travel.
	// Neither the toast nor its original world is guaranteed to survive it.
	const TWeakObjectPtr<UUserWidget> WeakWidget(Widget);
	const TWeakObjectPtr<UWorld> WeakWorld(World);
	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(TimerHandle, [WeakWidget, WeakWorld]()
		{
			UUserWidget* Toast = WeakWidget.Get();
			UWorld* ToastWorld = WeakWorld.Get();
			if (IsValid(Toast) && IsValid(ToastWorld) && !ToastWorld->bIsTearingDown &&
				Toast->GetWorld() == ToastWorld && Toast->GetClass()->ImplementsInterface(UIToast::StaticClass()))
			{
				IIToast::Execute_Hide(Toast);
			}
		}, Duration, false);
}
