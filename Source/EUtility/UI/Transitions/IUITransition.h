#pragma once

#include "CoreMinimal.h"
#include "EUtility/UI/BaseWidget.h"
#include <functional>
#include "IUITransition.generated.h"

/**
 * UI Transaction State
 */
UENUM(BlueprintType)
enum class EUITransactionState : uint8
{
    Idle,
    Executing,
    Completed,
    Failed
};

/**
 * UI Transaction Base Class
 * Each transaction represents a single UI animation/transition effect
 */
UCLASS(Abstract, Blueprintable)
class EUTILITY_API UUITransaction : public UObject
{
    GENERATED_BODY()
    
public:
    virtual ~UUITransaction() = default;
    
    /**
     * Execute the transaction
     * @param Widget - The widget to apply the transition to
     * @param Duration - How long the transition should take
     * @param OnComplete - Callback when the transition completes
     */
    virtual void Execute(UBaseWidget* Widget, float Duration, std::function<void()> OnComplete);
    
    /**
     * Get current transaction state
     */
    UFUNCTION(BlueprintCallable, Category = "UI Transaction")
    EUITransactionState GetState() const { return State; }
    
    /**
     * Check if transaction is currently executing
     */
    UFUNCTION(BlueprintCallable, Category = "UI Transaction")
    bool IsExecuting() const { return State == EUITransactionState::Executing; }

    virtual ESlateVisibility PreTransitionVisibility() const { return ESlateVisibility::HitTestInvisible; }

    virtual ESlateVisibility PostTransitionVisibility() const { return ESlateVisibility::Visible; }

    virtual bool PreTransition(UBaseWidget* Widget, float Duration, std::function<void()> OnComplete);

    float GetTickTimeDelta();

    bool IsReversed() const { return ScaleFactor < 0.0f; }
protected:
    
    virtual float Interpolation(float In, float Start, float End) const;

    virtual void ApplyInterpolation(float Alpha) { }

    virtual void Tick();

    float IntervalPerFrame() const { return 0.016f; } // Approx 60 FPS

protected:
    float TickInternal();

protected:
    // Current state of the transaction
    UPROPERTY(BlueprintReadOnly, Category = "UI Transaction")
    EUITransactionState State = EUITransactionState::Idle;
    
    // Timer handle for the animation
    FTimerHandle TimerHandle;
    
    // Target widget
    UPROPERTY()
    TObjectPtr<UBaseWidget> TargetWidget = nullptr;
    
    // Duration of the transaction
    float TransitionDuration = 0.0f;
    
    // Elapsed time
    float ElapsedTime = 0.0f;

    float ScaleFactor = 1.0f;

    double LastElapsedTime = 0.0f;
    
    // Completion callback
    std::function<void()> CompletionCallback;
    
    /**
     * Helper to set state and log if needed
     */
    void SetState(EUITransactionState NewState);
    
    /**
     * Helper to safely invoke completion callback
     */
    void InvokeCompletionCallback();
    
    /**
     * Helper to clear timer
     */
    void ClearTransitionTimer();
};
