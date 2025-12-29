/*
 * @Author: Punal Manalan
 * @Description: MF_AIIntegration - Registers P_MiniFootball actions with P_EAIS
 *               This file hooks into the module startup to register game-specific actions
 * @Date: 29/12/2025
 */

#pragma once

#include "CoreMinimal.h"

/**
 * Static class to handle AI integration between P_MiniFootball and P_EAIS.
 * Call RegisterActions() during module startup.
 */
class P_MINIFOOTBALL_API FMF_AIIntegration
{
public:
    /** Register all P_MiniFootball actions with the P_EAIS subsystem */
    static void RegisterActions();

    /** Unregister actions (call during module shutdown) */
    static void UnregisterActions();

private:
    static bool bActionsRegistered;
};
