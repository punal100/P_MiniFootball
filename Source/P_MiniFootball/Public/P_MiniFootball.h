/*
 * @Author: Punal Manalan
 * @Description: Mini Football System - Public Module Header
 * @Date: 07/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * P_MiniFootball Module Interface
 * Mini Football System plugin module
 */
class FP_MiniFootballModule : public IModuleInterface
{
public:

/** IModuleInterface implementation */
virtual void StartupModule() override;
virtual void ShutdownModule() override;

/**
 * Singleton-like access to this module's interface.
 * @return Returns singleton instance, loading the module on demand if needed.
 */
static inline FP_MiniFootballModule& Get()
{
return FModuleManager::LoadModuleChecked<FP_MiniFootballModule>("P_MiniFootball");
}

/**
 * Checks to see if this module is loaded and ready.
 * @return True if the module is loaded and ready to use.
 */
static inline bool IsAvailable()
{
return FModuleManager::Get().IsModuleLoaded("P_MiniFootball");
}
};
