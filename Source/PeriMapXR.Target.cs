// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class PeriMapXRTarget : TargetRules
{
	public PeriMapXRTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V4;

        // Set the include order version
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

        // Set the Windows SDK version
        WindowsPlatform.WindowsSdkVersion = "10.0.26100.0";

        // Set the platform toolset
        WindowsPlatform.Compiler = WindowsCompiler.VisualStudio2022;

        // Set the exact toolset version
        WindowsPlatform.CompilerVersion = "14.41.34120";

        // Set the C++ standard to C++20
        CppStandard = CppStandardVersion.Cpp20;

        // Enable Link Time Code Generation (LTCG)
        bAllowLTCG = false;

        // Enable hot reload support
        bAllowHotReload = true;

        ExtraModuleNames.AddRange( new string[] { "PeriMapXR" } );
	}
}
