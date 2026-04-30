#include "SimpleFadeTransition.h"
#include "EUtility/UI/BaseWidget.h"
#include "TimerManager.h"
#include "EUtility/UI/Log.h"
// --- FadeIn Transaction ---
void UFadeInTransaction::ApplyInterpolation(float Alpha)
{
    if (TargetWidget && TargetWidget->IsValidLowLevel())
    {
        TargetWidget->SetRenderOpacity(Alpha);
    }
}
// --- FadeOut Transaction ---
void UFadeOutTransaction::ApplyInterpolation(float Alpha)
{
    if (TargetWidget && TargetWidget->IsValidLowLevel())
    {
        TargetWidget->SetRenderOpacity(Alpha);
    }
}
bool UFadeOutTransaction::PreTransition(UBaseWidget* Widget, float Duration, std::function<void()> OnComplete)
{
    ScaleFactor = -1.0f;
    return Super::PreTransition(Widget, Duration, OnComplete);
}
// --- SlideIn Transaction ---
bool USlideInTransaction::PreTransition(UBaseWidget* Widget, float Duration, std::function<void()> OnComplete)
{
    if (!Widget)
    {
        UE_LOG(LogUIUtility, Error, TEXT("USlideInTransaction::PreTransition - Widget is null!"));
        return false;
    }
    TargetWidget = Widget;
    FVector2D ViewportSize = FVector2D(1920, 1080);
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->GetViewportSize(ViewportSize);
    }
    StartPosition = TargetWidget->GetOffsetInViewport(false);
    TargetPosition = StartPosition;
    switch (Direction)
    {
    case ESlideDirection::Left:
        StartPosition.X -= ViewportSize.X;
        break;
    case ESlideDirection::Right:
        StartPosition.X += ViewportSize.X;
        break;
    case ESlideDirection::Top:
        StartPosition.Y -= ViewportSize.Y;
        break;
    case ESlideDirection::Bottom:
        StartPosition.Y += ViewportSize.Y;
        break;
    }
    TargetWidget->SetOffsetInViewport(StartPosition, false);
    if (!Super::PreTransition(Widget, Duration, OnComplete))
        return false;
    return true;
}
void USlideInTransaction::ApplyInterpolation(float Alpha)
{
    if (TargetWidget && TargetWidget->IsValidLowLevel())
    {
        FVector2D CurrentPosition = FMath::Lerp(StartPosition, TargetPosition, Alpha);
        TargetWidget->SetOffsetInViewport(CurrentPosition, false);
    }
}
float USlideInTransaction::Interpolation(float In, float Start, float End) const
{
    float Alpha = FMath::Clamp(In / End, 0.0f, 1.0f);
    return FMath::InterpEaseOut(0.0f, 1.0f, Alpha, 2.0f);
}
// --- SlideOut Transaction ---
bool USlideOutTransaction::PreTransition(UBaseWidget* Widget, float Duration, std::function<void()> OnComplete)
{
    if (!Widget)
    {
        UE_LOG(LogUIUtility, Error, TEXT("USlideOutTransaction::PreTransition - Widget is null!"));
        return false;
    }
    TargetWidget = Widget;
    FVector2D ViewportSize = FVector2D(1920, 1080);
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->GetViewportSize(ViewportSize);
    }
    TargetPosition = TargetWidget->GetOffsetInViewport(false);
    StartPosition = TargetPosition;
    switch (Direction)
    {
    case ESlideDirection::Left:
        TargetPosition.X -= ViewportSize.X;
        break;
    case ESlideDirection::Right:
        TargetPosition.X += ViewportSize.X;
        break;
    case ESlideDirection::Top:
        TargetPosition.Y -= ViewportSize.Y;
        break;
    case ESlideDirection::Bottom:
        TargetPosition.Y += ViewportSize.Y;
        break;
    }
    TargetWidget->SetOffsetInViewport(StartPosition, false);
    if (!Super::PreTransition(Widget, Duration, OnComplete))
        return false;
    return true;
}
void USlideOutTransaction::ApplyInterpolation(float Alpha)
{
    if (TargetWidget && TargetWidget->IsValidLowLevel())
    {
        FVector2D CurrentPosition = FMath::Lerp(StartPosition, TargetPosition, Alpha);
        TargetWidget->SetOffsetInViewport(CurrentPosition, false);
    }
}
float USlideOutTransaction::Interpolation(float In, float Start, float End) const
{
    float Alpha = FMath::Clamp(In / End, 0.0f, 1.0f);
    return FMath::InterpEaseIn(0.0f, 1.0f, Alpha, 2.0f);
}
// --- ScaleIn Transaction ---
void UScaleInTransaction::ApplyInterpolation(float Alpha)
{
    if (TargetWidget && TargetWidget->IsValidLowLevel())
    {
        TargetWidget->SetRenderScale(FVector2D(Alpha, Alpha));
    }
}
float UScaleInTransaction::Interpolation(float In, float Start, float End) const
{
    float Alpha = FMath::Clamp(In / End, 0.0f, 1.0f);
    return FMath::InterpEaseOut(0.0f, 1.0f, Alpha, 2.0f);
}
// --- ScaleOut Transaction ---
bool UScaleOutTransaction::PreTransition(UBaseWidget* Widget, float Duration, std::function<void()> OnComplete)
{
    ScaleFactor = -1.0f;
    return Super::PreTransition(Widget, Duration, OnComplete);
}
void UScaleOutTransaction::ApplyInterpolation(float Alpha)
{
    if (TargetWidget && TargetWidget->IsValidLowLevel())
    {
        TargetWidget->SetRenderScale(FVector2D(Alpha, Alpha));
    }
}
float UScaleOutTransaction::Interpolation(float In, float Start, float End) const
{
    float Alpha = FMath::Clamp(In / End, 0.0f, 1.0f);
    return FMath::InterpEaseIn(0.0f, 1.0f, Alpha, 2.0f);
}
// --- No Transaction ---
void UNoTransaction::Execute(UBaseWidget* Widget, float Duration, std::function<void()> OnComplete)
{
    if (!Widget)
    {
        UE_LOG(LogUIUtility, Error, TEXT("UNoTransaction::Execute - Widget is null!"));
        return;
    }
    InvokeCompletionCallback();
}
