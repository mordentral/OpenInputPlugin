// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class OpenVRU : ModuleRules
{
	public OpenVRU(ReadOnlyTargetRules Target) : base(Target)
	{
		/** Mark the current version of the OpenVR SDK */
		string OpenVRVersion = "v1_5_17";
		Type = ModuleType.External;

		string SdkBase = /*Target.UEThirdPartySourceDirectory +*/ ModuleDirectory + "/OpenVR" + OpenVRVersion;
		if (!Directory.Exists(SdkBase))
		{
			string Err = string.Format("OpenVR SDK not found in {0}", SdkBase);
			System.Console.WriteLine(Err);
			throw new BuildException(Err);
		}

		PublicIncludePaths.Add(SdkBase + "/headers");

		string LibraryPath = SdkBase + "/lib/";

		if(Target.Platform == UnrealTargetPlatform.Win32)
		{
			PublicLibraryPaths.Add(LibraryPath + "win32");
			PublicAdditionalLibraries.Add("openvr_api.lib");
			PublicDelayLoadDLLs.Add("openvr_api.dll");

			string OpenVRBinariesDir = String.Format(ModuleDirectory + "/OpenVR{0}/bin/Win32/", OpenVRVersion);//String.Format("$(EngineDir)/Binaries/ThirdParty/OpenVR/OpenVR{0}/Win32/", OpenVRVersion);
			RuntimeDependencies.Add(OpenVRBinariesDir + "openvr_api.dll");
		}
		else if(Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicLibraryPaths.Add(LibraryPath + "win64");
			PublicAdditionalLibraries.Add("openvr_api.lib");
			PublicDelayLoadDLLs.Add("openvr_api.dll");

			string OpenVRBinariesDir = String.Format(ModuleDirectory + "/OpenVR{0}/bin/Win64/", OpenVRVersion);
			RuntimeDependencies.Add(OpenVRBinariesDir + "openvr_api.dll");
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			string DylibPath = /*Target.UEThirdPartyBinariesDirectory + */ ModuleDirectory + "/OpenVR" + OpenVRVersion + "/bin/osx32/libopenvr_api.dylib";
			PublicDelayLoadDLLs.Add(DylibPath);
			PublicAdditionalShadowFiles.Add(DylibPath);
			RuntimeDependencies.Add(DylibPath);
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux && Target.Architecture.StartsWith("x86_64"))
		{
			PublicLibraryPaths.Add(LibraryPath + "linux64");
			PublicAdditionalLibraries.Add("openvr_api");

			string DylibPath = /*Target.UEThirdPartyBinariesDirectory + */ ModuleDirectory + "/OpenVR" + OpenVRVersion + "/bin/linux64/libopenvr_api.so";
			PublicDelayLoadDLLs.Add(DylibPath);
			RuntimeDependencies.Add(DylibPath);
		}
	}
}
