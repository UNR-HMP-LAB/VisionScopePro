// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class VisionScopeProTarget : TargetRules
{
    public VisionScopeProTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V4;

        ExtraModuleNames.AddRange(new string[] { "VisionScopePro", "RAPD", "PeriMapXR" });

        // Set the include order version
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

        // Set the Windows SDK version
        WindowsPlatform.WindowsSdkVersion = "10.0.20348.0";

        // Set the platform toolset
        WindowsPlatform.Compiler = WindowsCompiler.VisualStudio2022;

        // Set the exact toolset version
        WindowsPlatform.CompilerVersion = "14.37.32822";

        // Set the C++ standard to C++20
        CppStandard = CppStandardVersion.Cpp20;

        // Enable Link Time Code Generation (LTCG)
        bAllowLTCG = false;

        // Enable hot reload support
        bAllowHotReload = true;
    }
}