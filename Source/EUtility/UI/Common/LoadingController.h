#pragma once

#include "CoreMinimal.h"
#include "EUtility/UI/UIViewController.h"
#include "LoadingModel.h"
#include "LoadingController.generated.h"

/**
 * Loading 弹窗控制器
 * 负责帧动画播放、位置布局等逻辑
 */
UCLASS()
class EUTILITY_API ULoadingController : public UUIViewController
{
	GENERATED_BODY()

public:
	ULoadingController();

	virtual void OnViewCreated(UBaseWidget* InView, const FUIViewConfig& InConfig) override;
	virtual void OnViewWillAppear() override;
	virtual void OnViewDidDisappear() override;
	virtual void SetModel(UObject* InModel) override;

protected:
	virtual void BindUIEvents_Implementation() override;
	virtual void UpdateView_Implementation() override;

private:
	// 帧动画定时器回调
	UFUNCTION()
	void OnFrameTick();

	// 根据当前帧索引更新 img_loading 的 UV
	void ApplyFrameUV();

	// 应用位置到 Widget
	void ApplyPosition();

	// 获取类型安全的 Model
	ULoadingModel* GetLoadingModel() const;

private:
	int32 CurrentFrameIndex = 0;
	FTimerHandle FrameTimerHandle;

	// Sprite Sheet 布局（从 View 读取，缓存避免重复 Cast）
	int32 CachedColumns    = 1;
	int32 CachedRows       = 1;
	int32 CachedTotalFrames = 1;

	UPROPERTY()
	class UImage* ImgLoading;

	UPROPERTY()
	class UTextBlock* TbHint;

	UPROPERTY()
	class UCanvasPanel* CanvasRoot;
};
