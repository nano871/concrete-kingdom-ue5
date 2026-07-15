// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ConcreteKingdom : ModuleRules
{
	public ConcreteKingdom(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"ChaosVehicles",
			"PhysicsCore",
			"NavigationSystem",
			"AIModule",
			"GameplayTasks",
			"UMG",
			"Slate",
			"SlateCore",
			"HTTP",
			"Json",
			"JsonUtilities"
		});
	}
}
