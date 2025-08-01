// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class g01 : ModuleRules
{
    public g01(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

        PrivateDependencyModuleNames.AddRange(new string[] { "MassEntity", "MassEntity" });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "music_player","TestMass" ,
            
            "MassCommon",
            "MassEntity",
            "MassSpawner",
            "StructUtils", 
            "MassGameplayDebug",
        });

        PrivateIncludePaths.AddRange(new string[] { "g01" });
        PublicIncludePaths.AddRange(new string[] { "g01" });

        CppStandard = CppStandardVersion.Cpp20;
        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
