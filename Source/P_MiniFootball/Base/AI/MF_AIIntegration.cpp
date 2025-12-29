/*
 * @Author: Punal Manalan
 * @Description: Implementation of MF_AIIntegration
 *               Registers P_MiniFootball game actions with P_EAIS action registry
 * @Date: 29/12/2025
 */

#include "MF_AIIntegration.h"
#include "EAISSubsystem.h"
#include "AIAction.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

bool FMF_AIIntegration::bActionsRegistered = false;

void FMF_AIIntegration::RegisterActions()
{
    if (bActionsRegistered)
    {
        return;
    }

    // Note: Action registration happens in UEAISSubsystem::RegisterDefaultActions()
    // This function can be used for additional game-specific actions.
    
    // The following built-in actions are already registered by P_EAIS:
    // - MoveTo
    // - Kick
    // - AimAt
    // - SetLookTarget
    // - Wait
    // - SetBlackboardKey
    // - InjectInput
    // - PassToTeammate
    // - LookAround
    
    // Example of how to register a custom action:
    // UEAISSubsystem* Subsystem = UEAISSubsystem::Get(WorldContext);
    // if (Subsystem)
    // {
    //     Subsystem->RegisterAction(TEXT("CustomAction"), UMyCustomAction::StaticClass());
    // }

    UE_LOG(LogTemp, Log, TEXT("FMF_AIIntegration: P_MiniFootball AI actions ready"));
    bActionsRegistered = true;
}

void FMF_AIIntegration::UnregisterActions()
{
    // Actions are automatically cleaned up when the subsystem is deinitialized
    bActionsRegistered = false;
}
