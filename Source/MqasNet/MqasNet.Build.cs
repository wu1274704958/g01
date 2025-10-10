using UnrealBuildTool;
using System.IO;

public class MqasNet : ModuleRules
{
    public MqasNet(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        // Add public dependency modules
        PublicDependencyModuleNames.AddRange(
            [
                "Core"
            ]
        );

        // Add private dependency modules
        PrivateDependencyModuleNames.AddRange(
            [
                "CoreUObject",
                "Engine"
            ]
        );
        
        // Set C++ standard
        CppStandard = CppStandardVersion.Cpp20;
        
        // Get module directory path
        string ModuleDir = ModuleDirectory;
        
        // 1. Configure header file folders
        // Add public include paths
        PublicIncludePaths.AddRange(
            [
                Path.Combine(ModuleDir, "include"),
                ModuleDir
            ]
        );
        
        // 2. Configure dependency libraries based on system and build type
        // Attempt to configure library files for different platforms
        TryConfigureLibrariesForPlatform(Target, ModuleDir);
       
    }
    
    private void TryConfigureLibrariesForPlatform(ReadOnlyTargetRules Target, string ModuleDir)
    {
        string PlatformDir = "";
        string Architecture = "";
        string ConfigDir = "";
        string StaticLibExt = "";
        string SharedLibExt = "";
        string SharedLibPattern = "";
        
        // Determine platform directory and file extensions
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PlatformDir = "windows";
            Architecture = "x64";
            StaticLibExt = ".lib";
            SharedLibExt = ".dll";
            SharedLibPattern = "*.dll";
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            PlatformDir = "linux";
            Architecture = "x64";
            StaticLibExt = ".a";
            SharedLibExt = ".so";
            SharedLibPattern = "*.so*"; // Include versioned libraries
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PlatformDir = "mac";
            Architecture = Target.Architecture == UnrealArch.Arm64 ? "arm64" : "x64";
            StaticLibExt = ".a";
            SharedLibExt = ".dylib";
            SharedLibPattern = "*.dylib";
        }
        else
        {
            System.Console.WriteLine($"Warning: Unsupported target platform {Target.Platform}");
            return;
        }
        
        // Determine build configuration directory
        if (Target.Configuration == UnrealTargetConfiguration.Debug ||
            Target.Configuration == UnrealTargetConfiguration.DebugGame)
        {
            ConfigDir = "debug";
        }
        else
        {
            ConfigDir = "release";
        }
        
        // Attempt to set library directory
        string LibDir = Path.Combine(ModuleDir, "output", PlatformDir, Architecture, ConfigDir, "lib");
        if (Directory.Exists(LibDir))
        {
            PublicSystemLibraryPaths.Add(LibDir);
            System.Console.WriteLine($"Adding library directory: {LibDir}");
            
            // Add all static library files
            var staticLibs = Directory.GetFiles(LibDir, $"*{StaticLibExt}");
            foreach (string lib in staticLibs)
            {
                string libName = Path.GetFileName(lib);
                PublicSystemLibraries.Add(libName);
                System.Console.WriteLine($"Adding static library: {libName}");
            }
            
            // Output warning if no libraries found
            if (staticLibs.Length == 0)
            {
                System.Console.WriteLine($"Warning: No {StaticLibExt} files found in {LibDir}");
            }
        }
        else
        {
            System.Console.WriteLine($"Warning: Library directory does not exist - {LibDir}");
        }
        
        // Attempt to set runtime dependencies for shared libraries
        string BinDir = Path.Combine(ModuleDir, "output", PlatformDir, Architecture, ConfigDir, "bin");
        if (Directory.Exists(BinDir))
        {
            // Add all shared library files
            var sharedLibs = Directory.GetFiles(BinDir, SharedLibPattern);
            foreach (string lib in sharedLibs)
            {
                RuntimeDependencies.Add($"$(BinaryOutputDir)/{Path.GetFileName(lib)}", lib);
                System.Console.WriteLine($"Adding runtime dependency: $(BinaryOutputDir)/{Path.GetFileName(lib)}:{lib}");
            }
            
            // Output warning if no libraries found
            if (sharedLibs.Length == 0)
            {
                System.Console.WriteLine($"Warning: No {SharedLibPattern} files found in {BinDir}");
            }
        }
        else
        {
            System.Console.WriteLine($"Warning: Binary directory does not exist - {BinDir}");
        }
    }
}