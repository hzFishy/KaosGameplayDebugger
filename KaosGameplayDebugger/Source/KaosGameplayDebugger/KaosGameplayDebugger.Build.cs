// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class KaosGameplayDebugger : ModuleRules
{
	public KaosGameplayDebugger(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"DeveloperSettings", 
				"SlateIM", 
				"Engine",
				"InputCore", 
				"GameplayTags",
				
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		if (Target.bBuildEditor)
		{
			PublicDependencyModuleNames.Add("ToolMenus");
			PublicDependencyModuleNames.Add("UnrealEd");
		}
		
		bool bEnableKaosDebugger =
			Target.bCompileAgainstEngine && 
			Target.Configuration != UnrealTargetConfiguration.Shipping;

		PublicDefinitions.Add("WITH_KAOS_GAMEPLAYDEBUGGER=" + (bEnableKaosDebugger ? "1" : "0"));

	}
}
