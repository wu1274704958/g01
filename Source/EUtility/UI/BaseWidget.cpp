#include "BaseWidget.h"

#include "Blueprint/GameViewportSubsystem.h"
#include "Blueprint/WidgetLayoutLibrary.h"

FVector2D UBaseWidget::GetOffsetInViewport(bool bRemoveDPIScale) const
{
	FVector2D OutOffset(0, 0);
	if (UGameViewportSubsystem* Subsystem = UGameViewportSubsystem::Get(GetWorld()))
	{
		if (bIsManagedByGameViewportSubsystem)
		{
			auto scale = bRemoveDPIScale ? UWidgetLayoutLibrary::GetViewportScale(this) : 1.0f;
			FGameViewportWidgetSlot ViewportSlot = Subsystem->GetWidgetSlot(this);
			OutOffset = FVector2D(ViewportSlot.Offsets.Left, ViewportSlot.Offsets.Top) * scale;
		}
	}
	return OutOffset;
}

void UBaseWidget::SetOffsetInViewport(FVector2D InOffset,bool bRemoveDPIScale)
{
	if (UGameViewportSubsystem* Subsystem = UGameViewportSubsystem::Get(GetWorld()))
	{
		auto scale = bRemoveDPIScale ? UWidgetLayoutLibrary::GetViewportScale(this) : 1.0f;
		FGameViewportWidgetSlot ViewportSlot;
		if (bIsManagedByGameViewportSubsystem)
		{
			ViewportSlot = Subsystem->GetWidgetSlot(this);
		}
		ViewportSlot.Offsets.Left = InOffset.X / scale;
		ViewportSlot.Offsets.Top = InOffset.Y / scale;
		Subsystem->SetWidgetSlot(this, ViewportSlot);
	}
}
