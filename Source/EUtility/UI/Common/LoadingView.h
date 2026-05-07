#pragma once

#include "CoreMinimal.h"
#include "EUtility/UI/BaseWidget.h"
#include "LoadingView.generated.h"

/**
 * Loading 弹窗视图
 * 纯UI Widget，不包含业务逻辑
 * 业务逻辑由 LoadingController 处理
 */
UCLASS()
class EUTILITY_API ULoadingView : public UBaseWidget
{
	GENERATED_BODY()

public:
	// 全屏背景遮罩（可选，不绑定时不显示背景）
	UPROPERTY(meta=(BindWidgetOptional))
	class UImage* img_background;

	// 帧动画图片显示控件
	UPROPERTY(meta=(BindWidget))
	class UImage* img_loading;

	// 可选提示文字
	UPROPERTY(meta=(BindWidgetOptional))
	class UTextBlock* tb_hint;

	// 外层容器（用于定位）
	UPROPERTY(meta=(BindWidget))
	class UCanvasPanel* canvas_root;

	/**
	 * 设置背景颜色和透明度
	 * @param Color  背景颜色（RGB），Alpha 决定透明度，0=完全透明，1=完全不透明
	 */
	UFUNCTION(BlueprintCallable, Category="Loading")
	void SetBackgroundColor(FLinearColor Color);
};
