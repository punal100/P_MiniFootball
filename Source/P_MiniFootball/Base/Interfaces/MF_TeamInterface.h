/*
 * @Author: Punal Manalan
 * @Description: MF_TeamInterface - Blueprint Interface for Team Operations
 *               Implemented by GameMode to handle team join/leave requests
 * @Date: 09/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/MF_Types.h"
#include "MF_TeamInterface.generated.h"

// Forward declarations
class APlayerController;

/**
 * Interface for team management operations
 * Implemented by AMF_GameMode (server-only)
 * 
 * This interface provides Blueprint-implementable functions for:
 * - Handling team join/leave requests from players
 * - Validating team balance rules
 * - Querying team state
 */
UINTERFACE(BlueprintType, Blueprintable, MinimalAPI)
class UMF_TeamInterface : public UInterface
{
    GENERATED_BODY()
};

class IMF_TeamInterface
{
    GENERATED_BODY()

public:
    // ==================== Team Join/Leave Operations ====================

    /**
     * Handle a request from a player to join a team
     * Called when player clicks "Join Team" button in widget
     * 
     * @param RequestingPC - The player controller requesting to join
     * @param RequestedTeam - The team they want to join
     * @return FMF_TeamAssignmentResult - Success/failure with team ID or error message
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MF|Team")
    FMF_TeamAssignmentResult HandleJoinTeamRequest(APlayerController* RequestingPC, EMF_TeamID RequestedTeam);

    /**
     * Handle a request from a player to leave their current team
     * Called when player clicks "Leave Team" or disconnects
     * 
     * @param RequestingPC - The player controller requesting to leave
     * @return bool - True if successfully left team
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MF|Team")
    bool HandleLeaveTeamRequest(APlayerController* RequestingPC);

    // ==================== Team Validation ====================

    /**
     * Check if a player can join a specific team
     * Validates team balance rules, capacity, and match state
     * 
     * @param PC - The player controller to check
     * @param TeamID - The team to check
     * @return bool - True if player can join the team
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MF|Team")
    bool CanPlayerJoinTeam(APlayerController* PC, EMF_TeamID TeamID);

    /**
     * Check if a specific team is at maximum capacity
     * 
     * @param TeamID - The team to check
     * @return bool - True if team is full
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MF|Team")
    bool IsTeamFull(EMF_TeamID TeamID);

    // ==================== Team Queries ====================

    /**
     * Get the current number of players on a team
     * 
     * @param TeamID - The team to query
     * @return int32 - Number of players on the team
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MF|Team")
    int32 GetTeamPlayerCount(EMF_TeamID TeamID);

    /**
     * Get list of teams the player can currently join
     * Based on team balance rules and capacity
     * 
     * @param PC - The player controller to check
     * @return TArray<EMF_TeamID> - List of joinable teams
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MF|Team")
    TArray<EMF_TeamID> GetAvailableTeams(APlayerController* PC);

    /**
     * Get maximum players allowed per team
     * 
     * @return int32 - Max players per team
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MF|Team")
    int32 GetMaxPlayersPerTeam();

    /**
     * Check if mid-match joining is allowed
     * 
     * @return bool - True if players can join during active match
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MF|Team")
    bool IsMidMatchJoinAllowed();
};
