#include "IUITransition.h"
#include "TimerManager.h"
#include "../Log.h"

void UUITransaction::Execute(UUserWidget* Widget, float Duration, std::function<void()> OnComplete)
{
    UWorld* World = Widget->GetWorld();
    if (!World || Duration <= 0.0f)
    {
        return;
    }
    if (!PreTransition(Widget, Duration,OnComplete))
        return;
    World->GetTimerManager().SetTimer(TimerHandle, this, &UUITransaction::Tick, IntervalPerFrame(), true);
}

bool UUITransaction::PreTransition(UUserWidget* Widget, float Duration, std::function<void()> OnComplete)
{
    if (!Widget)
    {
        UE_LOG(LogUIUtility, Error, TEXT("UFadeInTransaction::Execute - Widget is null!"));
        return false;
    }
    
    // Check if already executing
    if (IsExecuting())
    {
        UE_LOG(LogUIUtility, Warning, TEXT("UFadeInTransaction::Execute - Already executing, rejecting duplicate call!"));
        return false;
    }
    
    // Initialize member variables
    TargetWidget = Widget;
    TransitionDuration = Duration;
    ElapsedTime = 0.0f;
    LastElapsedTime = GetWorld()->GetTimeSeconds();
    
    CompletionCallback = std::move(OnComplete);
    SetState(EUITransactionState::Executing);
    if (TargetWidget)
        TargetWidget->SetVisibility(PreTransitionVisibility());
    SetState(EUITransactionState::Executing);
    ApplyInterpolation(IsReversed() ? 1.0f : 0.0f); // Initial tick to set starting state
    return true;
}

float UUITransaction::GetTickTimeDelta()
{
    const auto now = GetWorld()->GetTimeSeconds();
    const float delta = now - LastElapsedTime;
    //UE_LOG(LogUIUtility, Warning, TEXT("UUITransaction::GetTickTimeDelta - Delta Time: %f last: %f now: %f"), delta, LastElapsedTime, now);
    LastElapsedTime = now;
    return delta;
}

float UUITransaction::Interpolation(float In, float Start, float End) const
{
    return FMath::Lerp(Start, End, In);
}

void UUITransaction::Tick()
{
    if (!TargetWidget || !TargetWidget->IsValidLowLevel())
    {
        UE_LOG(LogUIUtility, Warning, TEXT("UFadeInTransaction::TickFadeIn - Widget invalid, stopping"));
        ClearTransitionTimer();
        SetState(EUITransactionState::Failed);
        return;
    }
    ApplyInterpolation(TickInternal());
     if (ElapsedTime >= TransitionDuration)
     {
         InvokeCompletionCallback();
     }
}


float UUITransaction::TickInternal()
{
    ElapsedTime += GetTickTimeDelta() * abs(ScaleFactor);
    auto Alpha = Interpolation(ElapsedTime,0.0f,TransitionDuration);
    if (IsReversed())
        Alpha = 1.0f - Alpha;
    return Alpha; 
}

void UUITransaction::SetState(EUITransactionState NewState)
{
    State = NewState;
}

void UUITransaction::InvokeCompletionCallback()
{
    ApplyInterpolation(IsReversed() ? 0.0f : 1.0f); // Initial tick to set end state
    ClearTransitionTimer();
    if (CompletionCallback)
    {
        CompletionCallback();
    }
    CompletionCallback = nullptr;
    if (TargetWidget)
        TargetWidget->SetVisibility(PostTransitionVisibility());
    this->ConditionalBeginDestroy();
    SetState(EUITransactionState::Completed);
}

void UUITransaction::ClearTransitionTimer()
{
    if (TargetWidget && TargetWidget->GetWorld())
    {
        UWorld* World = TargetWidget->GetWorld();
        if (TimerHandle.IsValid())
        {
            World->GetTimerManager().ClearTimer(TimerHandle);
            TimerHandle.Invalidate();
        }
    }
}

