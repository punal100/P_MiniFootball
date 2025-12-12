/*
 * @Author: Punal Manalan
 * @Description: Mini Football System - Editor module Build.cs.
 * @Date: 11/12/2025
 */

using UnrealBuildTool;
using System.IO;

// Editor-only module for tools and Editor Utility Widgets.
public class P_MiniFootballEditor : ModuleRules
{
    public P_MiniFootballEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        // NOTE:
        // This module is declared as an "Editor" module in P_MiniFootball.uplugin,
        // so Unreal will only build it for editor targets. No need to manually
        // disable it for non-editor targets here.

        string ModulePath = ModuleDirectory;

        PublicIncludePaths.AddRange(
            new string[]
            {
                Path.Combine(ModulePath, "Public"),
                Path.Combine(ModulePath, "Base"),
                Path.Combine(ModulePath, "Base", "UI"),
            }
        );

        PrivateIncludePaths.AddRange(
            new string[]
            {
                Path.Combine(ModulePath, "Private"),
                Path.Combine(ModulePath, "Base"),
                Path.Combine(ModulePath, "Base", "UI"),
            }
        );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "UMG",
                "Slate",
                "SlateCore",
                "Blutility",      // Editor Utility Widgets support
                "UMGEditor",      // UMG editor utilities
                "UnrealEd",       // General editor utilities
                "P_MiniFootball", // Runtime plugin module
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "EditorSubsystem",
                "LevelEditor",
            }
        );
    }
}
