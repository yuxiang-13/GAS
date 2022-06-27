// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class gas : ModuleRules
{
	public gas(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
		
		// 1 添加模块
		PrivateDependencyModuleNames.AddRange(new string[] {
			"GameplayAbilities",
		});
	}
}
