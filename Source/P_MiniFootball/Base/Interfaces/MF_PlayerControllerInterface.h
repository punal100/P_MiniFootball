/*
 * @Author: Punal Manalan
 * @Description: MF_PlayerControllerInterface - Blueprint Interface for Player Controller Callbacks
 *               Implemented by PlayerController to receive team assignment notifications
 * @Date: 09/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/MF_Types.h"
#include "MF_PlayerControllerInterface.generated.h"

// Forward declarations
class APawn;

/**
 * Interface for player controller team callbacks
 * Implemented by AMF_PlayerController
 * 
 * This interface provides Blueprint-implementable functions for:
 * - Receiving team assignment responses (success/failure)
 * - Handling state changes (spectator to playing, etc.)
 * - Responding to possession changes
 */
UINTERFACE(BlueprintType, Blueprintable, MinimalAPI)
class UMF_PlayerControllerInterface : public UInterface
{
    GENERATED_BODY()
};

class IMF_PlayerControllerInterface
{
    GENERATED_BODY()

public:
    // ==================== Team Assignment Callbacks ====================

    /**
     * Called when server responds to a team assignment request
     * Use this to update UI (close team select, show error, etc.)
     * 
     * @param bSuccess - Whether the assignment was successful
     * @param AssignedTeam - The team assigned to (None if failed)
     * @param ErrorMessage - Error message if assignment failed
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MF|Team")
    void OnTeamAssignmentResponse(bool bSuccess, EMF_TeamID AssignedTeam, const FString& ErrorMessage);

    /**
     * Called when the player's team or spectator state changes
     * Use this to update HUD team indicator
     * 
     * @param NewTeamID - The new team (None if spectating)
     * @param NewState - The new spectator state
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MF|Team")
    void OnTeamStateChanged(EMF_TeamID NewTeamID, EMF_SpectatorState NewState);

    /**
     * Called when the possessed pawn changes
     * Use this to setup character-specific UI
     * 
     * @param NewPawn - The newly possessed pawn (can be null)
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MF|Team")
    void OnPossessedPawnChanged(APawn* NewPawn);

    // ==================== State Queries ====================

    /**
     * Get the current team ID
     * 
     * @return EMF_TeamID - Current team (None if spectating)
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MF|Team")
    EMF_TeamID GetCurrentTeamID();

    /**
     * Get the current spectator state
     * 
     * @return EMF_SpectatorState - Current state (Spectating, Playing, Transitioning)
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MF|Team")
    EMF_SpectatorState GetCurrentSpectatorState();

    /**
     * Check if currently spectating (not on a team)
     * 
     * @return bool - True if spectating
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MF|Team")
    bool IsSpectating();
};
