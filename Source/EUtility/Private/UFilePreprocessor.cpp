#include "UFilePreprocessor.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "GenericPlatform/GenericPlatformFile.h"

FString UFilePreprocessor::PreprocessFile(const FString& SourceFilePath, bool bOverwriteSource, const FString& OutputFilePath)
{
    // 检查文件是否存在
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.FileExists(*SourceFilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("File not found: %s"), *SourceFilePath);
        return TEXT("");
    }
    
    // 读取文件内容
    FString FileContent;
    if (!FFileHelper::LoadFileToString(FileContent, *SourceFilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to read file: %s"), *SourceFilePath);
        return TEXT("");
    }

    // 预处理内容
    FString ProcessedContent = PreprocessString(FileContent);

    // 确定输出路径
    FString FinalOutputPath = bOverwriteSource ? SourceFilePath : 
        (OutputFilePath.IsEmpty() ? SourceFilePath + TEXT(".processed") : OutputFilePath);

    // 写入处理后的内容
    if (!FFileHelper::SaveStringToFile(ProcessedContent, *FinalOutputPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to write processed file: %s"), *FinalOutputPath);
        return TEXT("");
    }

    return ProcessedContent;
}

FString UFilePreprocessor::PreprocessString(const FString& Content)
{
    FString ProcessedContent = Content;
    
    // 获取所有占位符映射
    TMap<FString, FString> PlaceholderMap = GetPlaceholderMap();
    
    // 替换所有占位符
    for (const auto& Pair : PlaceholderMap)
    {
        ReplacePlaceholder(ProcessedContent, Pair.Key, Pair.Value);
    }
    
    return ProcessedContent;
}

void UFilePreprocessor::ReplacePlaceholder(FString& Content, const FString& Placeholder, const FString& Replacement)
{
    // 使用正则表达式确保只替换完整匹配的占位符
    FString Pattern = FString::Printf(TEXT("\\%s/\\b"), *Placeholder);
    FRegexPattern RegexPattern(Pattern);
    FRegexMatcher Matcher(RegexPattern, Content);
    
    while (Matcher.FindNext())
    {
        int32 Start = Matcher.GetMatchBeginning();
        int32 End = Matcher.GetMatchEnding();
        Content = Content.Left(Start) + Replacement + Content.Mid(End);
        
        // 重置匹配器以处理新位置
        Matcher = FRegexMatcher(RegexPattern, Content);
        Matcher.SetLimits(Start + Replacement.Len(), Content.Len());
    }
}

TMap<FString, FString> UFilePreprocessor::GetPlaceholderMap()
{
    TMap<FString, FString> Placeholders;
    
    // 项目路径相关
    Placeholders.Add(TEXT("$ProjectDir"), FPaths::ProjectDir());
    Placeholders.Add(TEXT("$ProjectContentDir"), FPaths::ProjectContentDir());
    Placeholders.Add(TEXT("$ProjectSavedDir"), FPaths::ProjectSavedDir());
    Placeholders.Add(TEXT("$ProjectConfigDir"), FPaths::ProjectConfigDir());
    Placeholders.Add(TEXT("$ProjectPluginsDir"), FPaths::ProjectPluginsDir());
    
    // 引擎路径相关
    Placeholders.Add(TEXT("$EngineDir"), FPaths::EngineDir());
    Placeholders.Add(TEXT("$EngineContentDir"), FPaths::EngineContentDir());
    Placeholders.Add(TEXT("$EngineSavedDir"), FPaths::EngineSavedDir());
    Placeholders.Add(TEXT("$EngineConfigDir"), FPaths::EngineConfigDir());
    Placeholders.Add(TEXT("$EnginePluginsDir"), FPaths::EnginePluginsDir());
    
    // 用户目录
    Placeholders.Add(TEXT("$UserDir"), FPlatformProcess::UserDir());
    Placeholders.Add(TEXT("$UserSettingsDir"), FPlatformProcess::UserSettingsDir());
    
    // 其他常用路径
    Placeholders.Add(TEXT("$RootDir"), FPaths::RootDir());
    Placeholders.Add(TEXT("$LaunchDir"), FPaths::LaunchDir());
    Placeholders.Add(TEXT("$GameAgnosticSavedDir"), FPaths::GameAgnosticSavedDir());
    
    return Placeholders;
}