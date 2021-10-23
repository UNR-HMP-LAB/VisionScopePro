// Copyright 2020 Fove, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class FoveHMD : ModuleRules
{
#if WITH_FORWARDED_MODULE_RULES_CTOR // Engine versions >= 4.16
    public FoveHMD(ReadOnlyTargetRules Target) : base(Target)
#else // Engine versions < 4.16
    public FoveHMD(TargetInfo Target)
#endif
    {
#if WITH_FORWARDED_MODULE_RULES_CTOR
        string EnginePath = Path.GetFullPath(Target.RelativeEnginePath);
#else
        string EnginePath = Path.GetFullPath(BuildConfiguration.RelativeEnginePath);
#endif

#if UE_4_21_OR_LATER // To avoid: warning: Modules must specify an explicit precompiled header (eg. PrivatePCHHeaderFile = "Public/FoveHMD.h") from UE 4.21 onwards.
        PrivatePCHHeaderFile = "Public/FoveHMD.h";
#endif

        PrivateIncludePaths.AddRange(new string[] {
            "FoveHMD/Private",
            EnginePath + "Source/Runtime/Renderer/Private", // Needed for FRenderingCompositePassContext
        });

        // Add our public dependencies. Anything using FoveHMD will automatically get these
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "FoveVR"
        });

        // Add our private dependencies. Anything we need internally to compile & link
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "HeadMountedDisplay",
            "Projects",
            "RHI",
            "RenderCore",
            "Renderer",
#if !UE_4_22_OR_LATER // ShaderCore seems to have joined RenderCore in 4.22
            "ShaderCore",
#endif
            "Slate",
            "SlateCore"
        });

        // Depend on the editor module if in the editor
#if WITH_FORWARDED_MODULE_RULES_CTOR
        if (Target.bBuildEditor == true)
#else
        if (UEBuildConfiguration.bBuildEditor == true)
#endif
            PrivateDependencyModuleNames.Add("UnrealEd");

        // Add Vulkan support
        if (Target.Platform != UnrealTargetPlatform.Mac)
        {
            // Depend on Vulkan RHI
            PrivateDependencyModuleNames.Add("VulkanRHI");

            // Add header paths to vulkan RHI
            PrivateIncludePaths.Add(Path.Combine(EnginePath, "Source/Runtime/VulkanRHI/Private"));
            if (Target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Win64)
                PrivateIncludePaths.Add(Path.Combine(EnginePath, "Source/Runtime/VulkanRHI/Private/Windows"));
            else if (Target.Platform != UnrealTargetPlatform.Mac)
                PrivateIncludePaths.Add(Path.Combine(EnginePath, "Source/Runtime/VulkanRHI/Private/Linux"));

            // Allow including of Vulkan headers
            AddEngineThirdPartyPrivateStaticDependencies(Target, "Vulkan");
        }
    }
}
