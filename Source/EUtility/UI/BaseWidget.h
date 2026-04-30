#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseWidget.generated.h"

/**
 * Base widget class for all UI widgets managed by UIManager.
 * Inherit from this instead of UUserWidget.
 */
UCLASS(Abstract, Blueprintable)
class EUTILITY_API UBaseWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Called by UIManager when the view is about to appear */
    UFUNCTION(BlueprintNativeEvent, Category="UI|Lifecycle")
    void OnViewWillAppear();
    virtual void OnViewWillAppear_Implementation() {}

    /** Called by UIManager after the view has appeared (fade-in complete) */
    UFUNCTION(BlueprintNativeEvent, Category="UI|Lifecycle")
    void OnViewDidAppear();
    virtual void OnViewDidAppear_Implementation() {}

    /** Called by UIManager when the view is about to disappear */
    UFUNCTION(BlueprintNativeEvent, Category="UI|Lifecycle")
    void OnViewWillDisappear();
    virtual void OnViewWillDisappear_Implementation() {}

    /** Called by UIManager after the view has disappeared (fade-out complete) */
    UFUNCTION(BlueprintNativeEvent, Category="UI|Lifecycle")
    void OnViewDidDisappear();
    virtual void OnViewDidDisappear_Implementation() {}

    FVector2D GetOffsetInViewport(bool bRemoveDPIScale) const;
    void SetOffsetInViewport(FVector2D InOffset,bool bRemoveDPIScale);
};

