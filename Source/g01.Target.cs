// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class g01Target : TargetRules
{
	public g01Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("g01");
		ExtraModuleNames.Add("music_player");
		ExtraModuleNames.Add("MqasNet");
		ExtraModuleNames.Add("TestMass");
		ExtraModuleNames.Add("EUtility");
	}
}
