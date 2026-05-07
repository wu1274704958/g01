#include "LoadingView.h"
#include "Components/Image.h"

void ULoadingView::SetBackgroundColor(FLinearColor Color)
{
	if (img_background)
	{
		img_background->SetColorAndOpacity(Color);
	}
}
