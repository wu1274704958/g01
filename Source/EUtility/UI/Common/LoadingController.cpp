#include "LoadingController.h"

#include <cassert>

#include "LoadingView.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateBrush.h"
#include "TimerManager.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/World.h"

ULoadingController::ULoadingController()
{
	CurrentFrameIndex = 0;
	ImgLoading = nullptr;
	TbHint = nullptr;
	CanvasRoot = nullptr;
}

void ULoadingController::OnViewCreated(UBaseWidget* InView, const FUIViewConfig& InConfig)
{
	Super::OnViewCreated(InView, InConfig);
}

void ULoadingController::OnViewWillAppear()
{
	Super::OnViewWillAppear();
	UpdateView();
}

void ULoadingController::OnViewDidDisappear()
{
	// 停止帧动画定时器
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FrameTimerHandle);
	}
	CurrentFrameIndex = 0;

	Super::OnViewDidDisappear();
}

void ULoadingController::SetModel(UObject* InModel)
{
	Super::SetModel(InModel);
	ULoadingModel* LoadingModel = GetLoadingModel();
	if (!LoadingModel)
	{
		return;
	}
	// 从 View 读取并缓存精灵图布局参数
	CachedColumns     = FMath::Max(1, LoadingModel->Columns);
	CachedRows        = FMath::Max(1, LoadingModel->Rows);
	CachedTotalFrames = FMath::Max(1, LoadingModel->TotalFrames);
}

void ULoadingController::BindUIEvents_Implementation()
{
	Super::BindUIEvents_Implementation();

	if (!View) return;

	ULoadingView* LoadingView = Cast<ULoadingView>(View);
	if (!LoadingView) return;

	ImgLoading = LoadingView->img_loading;
	TbHint     = LoadingView->tb_hint;
	CanvasRoot = LoadingView->canvas_root;
}

void ULoadingController::UpdateView_Implementation()
{
	Super::UpdateView_Implementation();

	ULoadingModel* LoadingModel = GetLoadingModel();
	if (!LoadingModel)
	{
		return;
	}

	// 更新背景遮罩
	ULoadingView* LoadingView = Cast<ULoadingView>(View);
	if (LoadingView)
	{
		if (LoadingView->img_background)
		{
			if (LoadingModel->bShowBackground)
			{
				LoadingView->img_background->SetVisibility(ESlateVisibility::HitTestInvisible);
				LoadingView->SetBackgroundColor(LoadingModel->BackgroundColor);
			}
			else
			{
				LoadingView->img_background->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}

	// 更新提示文字
	if (TbHint)
	{
		if (LoadingModel->HintText.IsEmpty())
		{
			TbHint->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			TbHint->SetVisibility(ESlateVisibility::HitTestInvisible);
			TbHint->SetText(FText::FromString(LoadingModel->HintText));
		}
	}

	// 应用位置
	ApplyPosition();

	// 重启帧动画
	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearTimer(FrameTimerHandle);
	CurrentFrameIndex = 0;

	if (ImgLoading && LoadingModel->SpriteSheet)
	{
		// 先设置贴图和初始 UV（第 0 帧）
		FSlateBrush Brush;
		Brush.SetResourceObject(LoadingModel->SpriteSheet);
		const float UVW = 1.f / CachedColumns;
		const float UVH = 1.f / CachedRows;
		Brush.SetUVRegion(FBox2D(FVector2D::ZeroVector, FVector2D(UVW, UVH)));
		ImgLoading->SetBrush(Brush);

		if (CachedTotalFrames > 1)
		{
			const float Interval = FMath::Max(0.016f, LoadingModel->FrameInterval);
			World->GetTimerManager().SetTimer(
				FrameTimerHandle,
				this,
				&ULoadingController::OnFrameTick,
				Interval,
				true
			);
		}
	}
}

void ULoadingController::OnFrameTick()
{
	CurrentFrameIndex = (CurrentFrameIndex + 1) % CachedTotalFrames;
	ApplyFrameUV();
}

void ULoadingController::ApplyFrameUV()
{
	if (!ImgLoading) return;

	ULoadingModel* LoadingModel = GetLoadingModel();
	if (!LoadingModel || !LoadingModel->SpriteSheet) return;

	const int32 Col = CurrentFrameIndex % CachedColumns;
	const int32 Row = CurrentFrameIndex / CachedColumns;

	const float UVW = 1.f / CachedColumns;
	const float UVH = 1.f / CachedRows;

	const FVector2D UVMin(Col * UVW, Row * UVH);
	const FVector2D UVMax(UVMin.X + UVW, UVMin.Y + UVH);

	FSlateBrush Brush = ImgLoading->GetBrush();
	Brush.SetUVRegion(FBox2D(UVMin, UVMax));
	ImgLoading->SetBrush(Brush);
}

void ULoadingController::ApplyPosition()
{
	if (!View || !ImgLoading)
	{
		return;
	}

	ULoadingModel* LoadingModel = GetLoadingModel();
	if (!LoadingModel)
	{
		return;
	}
	const auto V = Cast<ULoadingView>(GetView());
	UCanvasPanelSlot* pSlot = Cast<UCanvasPanelSlot>(V->canvas_root->Slot);
	assert(V && pSlot);
	const FVector2D& Padding = LoadingModel->Padding;

	FVector2D TargetPos = FVector2D::ZeroVector;
	FAnchors Anchors(0.5, 0.5, 0.5, 0.5);

	switch (LoadingModel->Position)
	{
	case ELoadingPosition::Center:
		TargetPos = FVector2D::ZeroVector;
		break;

	case ELoadingPosition::BottomLeft:
		TargetPos = FVector2D(Padding.X,-Padding.Y);
		Anchors = FAnchors(0, 1, 0, 1);
		break;

	case ELoadingPosition::BottomRight:
		TargetPos = FVector2D(-Padding.X,-Padding.Y);
		Anchors = FAnchors(1, 1, 1, 1);
		break;

	case ELoadingPosition::Custom:
		TargetPos = LoadingModel->CustomPosition;
		break;
	}
	pSlot->SetAnchors(Anchors);
	V->canvas_root->SetRenderTranslation(TargetPos);
}

ULoadingModel* ULoadingController::GetLoadingModel() const
{
	return Cast<ULoadingModel>(GetModel());
}
