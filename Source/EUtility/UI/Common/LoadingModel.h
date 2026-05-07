#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LoadingModel.generated.h"

/**
 * Loading 动画位置
 */
UENUM(BlueprintType)
enum class ELoadingPosition : uint8
{
	Center       UMETA(DisplayName = "Center"),
	BottomLeft   UMETA(DisplayName = "Bottom Left"),
	BottomRight  UMETA(DisplayName = "Bottom Right"),
	Custom       UMETA(DisplayName = "Custom"),
};

/**
 * Loading 弹窗数据模型
 */
UCLASS(Blueprintable)
class EUTILITY_API ULoadingModel : public UObject
{
	GENERATED_BODY()

public:
	ULoadingModel();

	// Loading 位置
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Loading")
	ELoadingPosition Position = ELoadingPosition::Center;

	// 自定义位置（Position == Custom 时生效）
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Loading")
	FVector2D CustomPosition = FVector2D::ZeroVector;

	// 精灵图（Sprite Sheet），所有帧合并在一张贴图中
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Loading")
	UTexture2D* SpriteSheet = nullptr;

	// 每帧间隔（秒）
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Loading")
	float FrameInterval = 0.1f;

	// 提示文字（可选）
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Loading")
	FString HintText;

	// 是否显示全屏背景遮罩
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Loading|Background")
	bool bShowBackground = false;

	// 背景颜色及透明度（Alpha=0 完全透明，Alpha=1 完全不透明）
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Loading|Background")
	FLinearColor BackgroundColor = FLinearColor(0.f, 0.f, 0.f, 0.5f);

	// 精灵图横向列数（每行有多少帧）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Loading|SpriteSheet")
	int32 Columns = 1;

	// 精灵图纵向行数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Loading|SpriteSheet")
	int32 Rows = 1;

	// 总帧数（<= Columns * Rows，用于排除空白格子）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Loading|SpriteSheet")
	int32 TotalFrames = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Loading")
	FVector2D Padding = FVector2D(30,30);
};
