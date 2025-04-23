// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.Collections.Generic;
using UnrealBuildTool;

public class music_player : ModuleRules
{
	public music_player(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Core" });


        PublicIncludePaths.AddRange(new string[] { 
            "music_player/include",
            "music_player/dep/win32/bass24/c" ,
            "music_player/dep/win32/bassflac24/c",
            "music_player/dep/mt/include"
        });

        PrivateIncludePaths.AddRange(new string[] { 
            "music_player/include" ,
            "music_player/dep/win32/bass24/c" ,
            "music_player/dep/win32/bassflac24/c",
            "music_player/dep/mt/include"
        });

        bEnableExceptions = true;

        PrivateDefinitions.AddRange(new string[] { "PF_WIN32", "EQD_MP_API" });
        PublicDefinitions.AddRange(new string[] { "PF_WIN32", "EQD_MP_API" });


        PublicSystemLibraryPaths.AddRange(new string[] { "music_player/dep/win32/bass24/c/x64", "music_player/dep/win32/bassflac24/c/x64" });

        PublicSystemLibraries.AddRange(new string[] { 
            "bass.lib",
            "bassflac.lib" 
        });

        RuntimeDependencies.Add("$(BinaryOutputDir)/bass.dll", $"{ModuleDirectory}/dep/win32/bass24/x64/bass.dll");
        RuntimeDependencies.Add("$(BinaryOutputDir)/bassflac.dll", $"{ModuleDirectory}/dep/win32/bassflac24/x64/bassflac.dll");

        RuntimeDependencies.Add("$(TargetOutputDir)/bass.dll", $"{ModuleDirectory}/dep/win32/bass24/x64/bass.dll");
        RuntimeDependencies.Add("$(TargetOutputDir)/bassflac.dll", $"{ModuleDirectory}/dep/win32/bassflac24/x64/bassflac.dll");

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
