#pragma once

#include "IUITransition.h"
#include "Components/CanvasPanelSlot.h"
#include "SimpleFadeTransition.generated.h"

/**
 * Slide Direction Enum
 */
UENUM(BlueprintType)
enum class ESlideDirection : uint8
{
    Left,
    Right,
    Top,
    Bottom
};

/**
 * Fade In Transaction - Animates widget from transparent to opaque
 */
UCLASS()
class EUTILITY_API UFadeInTransaction : public UUITransaction
{
    GENERATED_BODY()

protected:
    virtual void ApplyInterpolation(float Alpha) override;
};

/**
 * Fade Out Transaction - Animates widget from opaque to transparent
 */
UCLASS()
class EUTILITY_API UFadeOutTransaction : public UUITransaction
{
    GENERATED_BODY()
    
protected:
    virtual void ApplyInterpolation(float Alpha) override;

public:
    virtual bool PreTransition(UUserWidget* Widget, float Duration, std::function<void()> OnComplete) override;
};

/**
 * Slide In Transaction - Slides widget in from a direction
 */
UCLASS()
class EUTILITY_API USlideInTransaction : public UUITransaction
{
    GENERATED_BODY()
    
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transaction")
    ESlideDirection Direction = ESlideDirection::Bottom;
    
protected:
    virtual bool PreTransition(UUserWidget* Widget, float Duration, std::function<void()> OnComplete) override;
    virtual void ApplyInterpolation(float Alpha) override;
    virtual float Interpolation(float In, float Start, float End) const override;
    
private:
    FVector2D StartPosition;
    FVector2D TargetPosition;
    UCanvasPanelSlot* CanvasSlot = nullptr;
};

/**
 * Slide Out Transaction - Slides widget out to a direction
 */
UCLASS()
class EUTILITY_API USlideOutTransaction : public UUITransaction
{
    GENERATED_BODY()
    
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transaction")
    ESlideDirection Direction = ESlideDirection::Bottom;
    
protected:
    virtual bool PreTransition(UUserWidget* Widget, float Duration, std::function<void()> OnComplete) override;
    virtual void ApplyInterpolation(float Alpha) override;
    virtual float Interpolation(float In, float Start, float End) const override;
    virtual ESlateVisibility PostTransitionVisibility() const override { return ESlateVisibility::Collapsed; }
    
private:
    FVector2D StartPosition;
    FVector2D TargetPosition;
    UCanvasPanelSlot* CanvasSlot = nullptr;
};

/**
 * Scale In Transaction - Scales widget from small to normal size
 */
UCLASS()
class EUTILITY_API UScaleInTransaction : public UUITransaction
{
    GENERATED_BODY()
    
protected:
    virtual void ApplyInterpolation(float Alpha) override;
    virtual float Interpolation(float In, float Start, float End) const override;
};

/**
 * Scale Out Transaction - Scales widget from normal to small size
 */
UCLASS()
class EUTILITY_API UScaleOutTransaction : public UUITransaction
{
    GENERATED_BODY()
    
protected:
    virtual bool PreTransition(UUserWidget* Widget, float Duration, std::function<void()> OnComplete) override;
    virtual void ApplyInterpolation(float Alpha) override;
    virtual float Interpolation(float In, float Start, float End) const override;
    virtual ESlateVisibility PostTransitionVisibility() const override { return ESlateVisibility::Collapsed; }
};

/**
 * No Transaction - Instant show/hide with no animation
 */
UCLASS()
class EUTILITY_API UNoTransaction : public UUITransaction
{
    GENERATED_BODY()
    
public:
    virtual void Execute(UUserWidget* Widget, float Duration, std::function<void()> OnComplete) override;
};
