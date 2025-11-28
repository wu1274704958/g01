#pragma once

#include "CoreMinimal.h"
#include "Misc/Paths.h"
#include "UFilePreprocessor.generated.h"

UCLASS()
class EUTILITY_API UFilePreprocessor : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * 预处理文件中的占位符变量
	 * 
	 * @param SourceFilePath 源文件路径
	 * @param bOverwriteSource 是否覆盖源文件
	 * @param OutputFilePath 输出文件路径（当 bOverwriteSource 为 false 时有效）
	 * @return 预处理后的文件内容
	 */
	UFUNCTION(BlueprintCallable, Category = "File Utilities")
	static FString PreprocessFile(const FString& SourceFilePath, bool bOverwriteSource = true, const FString& OutputFilePath = TEXT(""));

	/**
	 * 预处理字符串中的占位符变量
	 * 
	 * @param Content 要处理的内容
	 * @return 处理后的内容
	 */
	UFUNCTION(BlueprintCallable, Category = "File Utilities")
	static FString PreprocessString(const FString& Content);

private:
	// 替换单个占位符变量
	static void ReplacePlaceholder(FString& Content, const FString& Placeholder, const FString& Replacement);
    
	// 所有支持的占位符映射
	static TMap<FString, FString> GetPlaceholderMap();
};