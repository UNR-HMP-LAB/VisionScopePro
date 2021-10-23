// Copyright 2020 Fove, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class FoveVR : ModuleRules
{
#if WITH_FORWARDED_MODULE_RULES_CTOR // Engine versions >= 4.16
    public FoveVR(ReadOnlyTargetRules Target) : base(Target)
#else // Engine versions < 4.16
    public FoveVR(TargetInfo Target)
#endif
    {
        Type = ModuleType.External;

        // Compute various needed paths
        string FoveSDKVersion = "1_0_1";
        string SDKVersionPath = "FoveVR_SDK_" + FoveSDKVersion;
        string BaseDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", ".."));
        string BinariesDir = Path.Combine(BaseDirectory, "Binaries", "ThirdParty", "FoveVR", SDKVersionPath);
        string SdkBase = Path.GetFullPath(Path.Combine(ModuleDirectory, SDKVersionPath));

        // Add the fove include directory
        string IncludePath = Path.Combine(SdkBase, "include");
        PublicIncludePaths.Add(IncludePath);

#if UE_4_21_OR_LATER // To avoid: warning: Modules must specify an explicit precompiled header (eg. PrivatePCHHeaderFile = "Public/FoveHMD.h") from UE 4.21 onwards.
        PrivatePCHHeaderFile = Path.Combine(SdkBase, "FoveAPI.h");
#endif

        // Link the correct fove library
        string RuntimeDep = null;
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicAdditionalLibraries.Add(Path.Combine(BinariesDir, "x64", "FoveClient.lib"));
            PublicDelayLoadDLLs.Add("FoveClient.dll");
            RuntimeDep = Path.Combine(BinariesDir, "x64", "FoveClient.dll");
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            string path = Path.Combine(BinariesDir, "x64", "libFoveClient.dylib");
            PublicAdditionalLibraries.Add(path);
            RuntimeDep = Path.Combine("$(BinaryOutputDir)/libFoveClient.dylib", path);
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            string path = Path.Combine(BinariesDir, "x64", "libFoveClient.so");
            PublicAdditionalLibraries.Add(path);
            RuntimeDep = Path.Combine("$(BinaryOutputDir)/libFoveClient.so", path);
        }
#if UE_4_24_OR_LATER
        else if (Target.Platform == UnrealTargetPlatform.LinuxAArch64)
        {
            string path = Path.Combine(BinariesDir, "arm64", "libFoveClient.so");
            PublicAdditionalLibraries.Add(path);
            RuntimeDep = Path.Combine("$(BinaryOutputDir)/libFoveClient.so", path);
        }
#endif

        if (RuntimeDep != null)
        {
#if UE_4_20_OR_LATER
            RuntimeDependencies.Add(RuntimeDep);
#else
            RuntimeDependencies.Add(new RuntimeDependency(RuntimeDep));
#endif
        }
    }
}
