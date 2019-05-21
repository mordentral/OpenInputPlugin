// Some copyright should be here...
using System.IO;
using UnrealBuildTool;

public class OpenInputPlugin : ModuleRules
{
    private string PluginsPath
    {
        get { return Path.GetFullPath(Target.RelativeEnginePath) + "Plugins/Runtime/"; }
    }

    public OpenInputPlugin(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        //bEnforceIWYU = true;

        bool bCompileWithVRExpansion = false;

        if (bCompileWithVRExpansion)
        {
            PrivateDefinitions.Add("USE_WITH_VR_EXPANSION=1");
        }
        else
        {
            PrivateDefinitions.Add("USE_WITH_VR_EXPANSION=0");
        }
		
        PublicIncludePaths.AddRange(
			new string[] {
				//"OpenVRExpansionPlugin/Public",
                //"HeadMountedDisplay/Public",
                //"Runtime/Engine/Private/PhysicsEngine"
				
				// ... add public include paths required here ...
			}
			);
	

        PublicDependencyModuleNames.AddRange(
            new string[] 
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "HeadMountedDisplay",
               // "HeadMountedDisplay",
                "RHI",
                "RenderCore",
               //"ShaderCore",
				"AnimGraphRuntime"
            });

        if(bCompileWithVRExpansion)
        {
            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                "VRExpansionPlugin"
                });
        }



        if (Target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Win64 || 
		(Target.Platform == UnrealTargetPlatform.Linux && Target.Architecture.StartsWith("x86_64")))
		{
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
			//"SteamVR",
			"OpenVRU",
			//"SteamVRController"
			});
		}

    }
}
