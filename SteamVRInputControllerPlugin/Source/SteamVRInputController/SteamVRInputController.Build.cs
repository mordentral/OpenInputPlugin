// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class SteamVRInputController : ModuleRules
{
    public SteamVRInputController(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivatePCHHeaderFile = "Public/ISteamVRInputPlugin.h";

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        PrivateIncludePathModuleNames.AddRange(new string[]
		{
			"TargetPlatform",
            "SteamVR"
		});

        PrivateDependencyModuleNames.AddRange(new string[]
        {
			"Core",
			"CoreUObject",
			"ApplicationCore",
			"Engine",
			"InputDevice",
            "InputCore",
			"HeadMountedDisplay",
            "SteamVR",
            "Json",
            "JsonUtilities",
            "SteamVRController"
        });

        if (Target.bBuildEditor == true)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }

        // Add the bindings folder to our packaging output
        RuntimeDependencies.Add(new RuntimeDependency("$(ProjectDir)/Config/SteamVRBindings/..."));


        if (Target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDependencyModuleNames.AddRange(
            new string[]
            {
               "OpenVRU"
            });

            //AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenVR");
            PrivateDependencyModuleNames.Add("D3D11RHI");     //@todo steamvr: multiplatform

            AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenGL");
            PrivateDependencyModuleNames.Add("OpenGLDrv");

            AddEngineThirdPartyPrivateStaticDependencies(Target, "Vulkan");
            PrivateDependencyModuleNames.Add("VulkanRHI");
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDependencyModuleNames.AddRange(
            new string[]
            {
               "OpenVRU"
            });

            PublicFrameworks.Add("IOSurface");
          //  AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenVR");
            PrivateDependencyModuleNames.AddRange(new string[] { "MetalRHI" });
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux && Target.Architecture.StartsWith("x86_64"))
        {

            PublicDependencyModuleNames.AddRange(
            new string[]
            {
              "OpenVRU"
            });
           // AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenVR");
            AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenGL");
            AddEngineThirdPartyPrivateStaticDependencies(Target, "Vulkan");
            PrivateDependencyModuleNames.Add("OpenGLDrv");
            PrivateDependencyModuleNames.Add("VulkanRHI");
        }

        /*if ( Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Win32 || (Target.Platform == UnrealTargetPlatform.Linux && Target.Architecture.StartsWith("x86_64")) )
        {
            AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenGL");
            PrivateDependencyModuleNames.Add("OpenGLDrv");
        }*/
    }
}
