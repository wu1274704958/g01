#pragma once
#include "IUITransition.h"
#include "EUtility/UI/BaseWidget.h"
#include "SimpleFadeTransition.generated.h"

UENUM(BlueprintType)
enum class ESlideDirection : uint8
{
    Left,
    Right,
    Top,
    Bottom
};
UCLASS()
class EUTILITY_API UFadeInTransaction : public UUITransaction
{
    GENERATED_BODY()
protected:
    virtual void ApplyInterpolation(float Alpha) override;
};
UCLASS()
class EUTILITY_API UFadeOutTransaction : public UUITransaction
{
    GENERATED_BODY()
public:
    virtual bool PreTransition(UBaseWidget* Widget, float Duration, std::function<void()> OnComplete) override;
protected:
    virtual void ApplyInterpolation(float Alpha) override;
};
/** Slide In: widget slides FROM offscreen TO RestingPosition */
UCLASS()
class EUTILITY_API USlideInTransaction : public UUITransaction
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transaction")
    ESlideDirection Direction = ESlideDirection::Top;
protected:
    virtual bool PreTransition(UBaseWidget* Widget, float Duration, std::function<void()> OnComplete) override;
    virtual void ApplyInterpolation(float Alpha) override;
    virtual float Interpolation(float In, float Start, float End) const override;
private:
    FVector2D StartPosition;
    FVector2D TargetPosition;
};
/** Slide Out: widget slides FROM RestingPosition TO offscreen */
UCLASS()
class EUTILITY_API USlideOutTransaction : public UUITransaction
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transaction")
    ESlideDirection Direction = ESlideDirection::Bottom;
protected:
    virtual bool PreTransition(UBaseWidget* Widget, float Duration, std::function<void()> OnComplete) override;
    virtual void ApplyInterpolation(float Alpha) override;
    virtual float Interpolation(float In, float Start, float End) const override;
    virtual ESlateVisibility PostTransitionVisibility() const override { return ESlateVisibility::Collapsed; }
private:
    FVector2D StartPosition;
    FVector2D TargetPosition;
};
UCLASS()
class EUTILITY_API UScaleInTransaction : public UUITransaction
{
    GENERATED_BODY()
protected:
    virtual void ApplyInterpolation(float Alpha) override;
    virtual float Interpolation(float In, float Start, float End) const override;
};
UCLASS()
class EUTILITY_API UScaleOutTransaction : public UUITransaction
{
    GENERATED_BODY()
protected:
    virtual bool PreTransition(UBaseWidget* Widget, float Duration, std::function<void()> OnComplete) override;
    virtual void ApplyInterpolation(float Alpha) override;
    virtual float Interpolation(float In, float Start, float End) const override;
    virtual ESlateVisibility PostTransitionVisibility() const override { return ESlateVisibility::Collapsed; }
};
UCLASS()
class EUTILITY_API UNoTransaction : public UUITransaction
{
    GENERATED_BODY()
public:
    virtual void Execute(UBaseWidget* Widget, float Duration, std::function<void()> OnComplete) override;
};
