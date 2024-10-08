// PeriMapXR.Build.cs

using UnrealBuildTool;

public class PeriMapXR : ModuleRules
{
	public PeriMapXR(ReadOnlyTargetRules Target) : base(Target)
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

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
