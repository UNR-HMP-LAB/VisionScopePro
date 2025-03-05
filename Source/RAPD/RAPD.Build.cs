// Copyright University of Nevada, Reno. All rights reserved.

using System.IO;
using UnrealBuildTool;

public class RAPD : ModuleRules
{
    public RAPD(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "InputCore",
                "HeadMountedDisplay",
                "EyeTracker",
                "PICOXREyeTracker",
                "PICOXRHMD",
                "PICOXRInput",
                "PICOXRMR",
                "PICOXRMotionTracking"
            }
        );
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine"
            }
        );
        var publicIncludePath = Path.Combine(ModuleDirectory, "Public", "UEigen3");
        if (Directory.Exists(publicIncludePath)) // Check if the directory exists
        {
            PublicIncludePaths.Add(publicIncludePath);
        }
        else
        {
            // Log a warning if the directory does not exist
            System.Diagnostics.Debug.WriteLine($"Warning: Include path '{publicIncludePath}' does not exist.");
        }
        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
//The provided Unreal Engine module rules code for the `RAPD` module is generally well-structured. However, let's identify potential areas for improvement and bugs:
//1. **PublicDependencyModuleNames Order**: Ensure that you add dependencies in the correct order if there are any direct dependencies among them. This isn't causing a functional bug, but it's a best practice to maintain clarity.
//2. **PCH Usage settings**: You used `PCHUsageMode.UseExplicitOrSharedPCHs`. If any of the modules you are depending on (especially third-party ones) don't support this mode, it might lead to compile errors. Ensure that all your dependencies are compatible.
//3. **Commented sections**: The commented-out sections for Slate UI and Online features suggest you might need these functionalities later. It could be helpful to add a remark specifying when to uncomment them or under which circumstances they might become necessary.
//4. **Use of `Path.Combine`**: Ensure that the path you are trying to add through `PublicIncludePaths` actually exists. If the directory is not present, it may lead to an error during the build.
//5. **Add a check before adding PublicIncludePaths**: It's often a good practice to check if the directory exists before adding it to avoid build errors.
//Here's an updated version that includes these improvements:
//```csharp
//// Copyright University of Nevada, Reno. All rights reserved.
//using System.IO;
//using UnrealBuildTool;
//public class RAPD : ModuleRules
//{
//    public RAPD(ReadOnlyTargetRules Target) : base(Target)
//    {
//        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
//        PublicDependencyModuleNames.AddRange(
//            new string[]
//            {
//                "InputCore",
//                "HeadMountedDisplay",
//                "EyeTracker",
//                "PICOXREyeTracker",
//                "PICOXRHMD",
//                "PICOXRInput",
//                "PICOXRMR",
//                "PICOXRMotionTracking"
//            }
//        );
//        PrivateDependencyModuleNames.AddRange(
//            new string[]
//            {
//                "Core",
//                "CoreUObject",
//                "Engine"
//            }
//        );
//        var publicIncludePath = Path.Combine(ModuleDirectory, "Public", "UEigen3");
//        if (Directory.Exists(publicIncludePath)) // Check if the directory exists
//        {
//            PublicIncludePaths.Add(publicIncludePath);
//        }
//        else
//        {
//            // Log a warning if the directory does not exist
//            System.Diagnostics.Debug.WriteLine($"Warning: Include path '{publicIncludePath}' does not exist.");
//        }
//        // Uncomment if you are using Slate UI
//        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
//        // Uncomment if you are using online features
//        // PrivateDependencyModuleNames.Add("OnlineSubsystem");
//        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
//    }
//}
//```
//### Summary of Changes:
//- **Directory Existence Check**: Added a check for the existence of `PublicIncludePaths` to prevent build errors if the directory is missing.
//- **Console Warning**: Log a warning message if the path does not exist, aiding in troubleshooting during development. 
//These adjustments should help improve the robustness of the build script.