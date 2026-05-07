#include "LoadingModel.h"

ULoadingModel::ULoadingModel()
{
	Position = ELoadingPosition::Center;
	CustomPosition = FVector2D::ZeroVector;
	FrameInterval = 0.1f;
	HintText = TEXT("");
	bShowBackground = false;
	BackgroundColor = FLinearColor(0.f, 0.f, 0.f, 0.5f);
}
