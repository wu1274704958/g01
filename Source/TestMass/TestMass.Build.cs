using UnrealBuildTool;

public class TestMass : ModuleRules
{
    public TestMass(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "MassCommon",
                "MassEntity",
                "MassSpawner",
                //"MassGameplayDebug",
                //"MassGameplayEditor"
            }
        );
        
        CppStandard = CppStandardVersion.Cpp20;
    }
}