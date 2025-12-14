/*
 * @Author: Punal Manalan
 * @Description: Mini Football System - Build.cs.
 * @Date: 07/12/2025
 */

using UnrealBuildTool;
using System.IO;

// Build configuration for the P_MiniFootball runtime module
public class P_MiniFootball : ModuleRules
{
        public P_MiniFootball(ReadOnlyTargetRules Target) : base(Target)
        {
                // Use explicit precompiled headers for better performance
                PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

                // Get the module directory path
                string ModulePath = ModuleDirectory;

                // Public include paths - these are exposed to other modules
                PublicIncludePaths.AddRange(
                new string[] {
Path.Combine(ModulePath, "Public"),
Path.Combine(ModulePath, "Base"),
Path.Combine(ModulePath, "Base", "Interfaces"),
Path.Combine(ModulePath, "Base", "UI"),
                }
                );

                // Private include paths - internal to this module only
                PrivateIncludePaths.AddRange(
                new string[] {
Path.Combine(ModulePath, "Private"),
Path.Combine(ModulePath, "Base"),
Path.Combine(ModulePath, "Base", "Interfaces"),
Path.Combine(ModulePath, "Base", "UI"),
                }
                );

                // Public dependencies - modules that are required by public headers
                PublicDependencyModuleNames.AddRange(
                new string[]
                {
"Core",
"CoreUObject",
"Engine",
"DeveloperSettings",
"InputCore",
"EnhancedInput",     // UE5 Enhanced Input System
"P_MEIS",            // Modular Enhanced Input System plugin
"NetCore",           // Core networking
"GameplayTags",      // Gameplay tags support
                }
                );

                // Private dependencies - modules only needed for implementation
                PrivateDependencyModuleNames.AddRange(
                new string[]
                {
"Slate",
"SlateCore",
"Json",
"JsonUtilities",
"UMG",               // Unreal Motion Graphics (Widget system)
"AIModule",          // AI support (deferred but included)
"NavigationSystem",  // Navigation for AI (deferred)
                }
                );

                // Dynamically loaded modules - loaded at runtime when needed
                DynamicallyLoadedModuleNames.AddRange(
                new string[]
                {
                }
                );
        }
}
