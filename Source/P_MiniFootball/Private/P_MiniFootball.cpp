/*
 * @Author: Punal Manalan
 * @Description: Mini Football System - Module Implementation
 * @Date: 07/12/2025
 */

#include "P_MiniFootball.h"
#include "MF_AIIntegration.h"

#define LOCTEXT_NAMESPACE "FP_MiniFootballModule"

void FP_MiniFootballModule::StartupModule()
{
    // This code will execute after your module is loaded into memory
    // The exact timing is specified in the .uplugin file per-module
    UE_LOG(LogTemp, Log, TEXT("P_MiniFootball: Module Started"));
    
    // Register P_MiniFootball gameplay actions with P_EAIS
    // Note: Actual registration happens when subsystem is available
    // See MF_AIIntegration.cpp for details
    FMF_AIIntegration::RegisterActions();
}

void FP_MiniFootballModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.
    // For modules that support dynamic reloading, we call this function before unloading the module.
    UE_LOG(LogTemp, Log, TEXT("P_MiniFootball: Module Shutdown"));
    
    // Unregister AI actions
    FMF_AIIntegration::UnregisterActions();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FP_MiniFootballModule, P_MiniFootball)

