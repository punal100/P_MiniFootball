/*
 * @Author: Punal Manalan
 * @Description: MF_PlayerController - Networked Player Controller Header
 *               Manages player possession, team assignment, and character switching
 *               Works with both Listen Server and Dedicated Server
 *               Implements spectator system and team request RPCs
 * @Date: 07/12/2025
 * @Updated: 09/12/2025 - Added spectator state and team request RPCs
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Core/MF_Types.h"
#include "Interfaces/MF_PlayerControllerInterface.h"
#include "MF_PlayerController.generated.h"

class AMF_PlayerCharacter;
class AMF_Spectator;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnControlledCharacterChanged, AMF_PlayerController *, Controller, AMF_PlayerCharacter *, NewCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeamAssigned, AMF_PlayerController *, Controller, EMF_TeamID, Team);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTeamAssignmentResponseDelegate, bool, bSuccess, EMF_TeamID, Team, const FString &, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpectatorStateChanged, AMF_PlayerController *, Controller, EMF_SpectatorState, NewState);

/**
 * MF_PlayerController - Networked Player Controller for Mini Football
 *
 * Responsibilities:
 * - Manages which character the player is currently controlling
 * - Handles player switching between team characters
 * - Receives team assignment from server
 * - Supports both Listen Server and Dedicated Server configurations
 * - Implements spectator system (players start as spectators)
 * - Provides Server RPCs for team join/leave requests (called from widgets)
 */
UCLASS()
class P_MINIFOOTBALL_API AMF_PlayerController : public APlayerController, public IMF_PlayerControllerInterface
{
    GENERATED_BODY()

public:
    AMF_PlayerController();

    // ==================== Replication ====================
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    // ==================== Spectator State (NEW) ====================

    /** Current spectator state - Spectating, Playing, or Transitioning */
    UPROPERTY(ReplicatedUsing = OnRep_SpectatorState, BlueprintReadOnly, Category = "Spectator")
    EMF_SpectatorState CurrentSpectatorState;

    UFUNCTION()
    void OnRep_SpectatorState();

    // ==================== Team Management ====================
    /** Assigned team (Server authoritative) */
    UPROPERTY(ReplicatedUsing = OnRep_AssignedTeam, BlueprintReadOnly, Category = "Team")
    EMF_TeamID AssignedTeam;

    /** Server-side: Assign this controller to a team */
    UFUNCTION(BlueprintCallable, Category = "Team")
    void AssignToTeam(EMF_TeamID NewTeam);

    UFUNCTION()
    void OnRep_AssignedTeam();

    // ==================== Team Request Server RPCs (Called from Widgets) ====================

    /**
     * Request to join a team - Called from Widget buttons
     * This is the MAIN entry point for team joining from UI
     * @param RequestedTeam - The team to join
     */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Team|Request")
    void Server_RequestJoinTeam(EMF_TeamID RequestedTeam);

    /**
     * Request to leave current team and return to spectator
     * Called from Widget or disconnect
     */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Team|Request")
    void Server_RequestLeaveTeam();

    // ==================== Team Response Client RPCs ====================

    /**
     * Server response to team assignment request
     * Called by GameMode after processing join/leave request
     * @param bSuccess - Whether the request succeeded
     * @param Team - The assigned team (None if failed or left)
     * @param ErrorMessage - Error description if failed
     */
    UFUNCTION(Client, Reliable)
    void Client_OnTeamAssignmentResponse(bool bSuccess, EMF_TeamID Team, const FString &ErrorMessage);

    // ==================== Character Management ====================
    /** All characters this controller can switch to */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Character")
    TArray<AMF_PlayerCharacter *> TeamCharacters;

    /** Currently active character index */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Character")
    int32 ActiveCharacterIndex;

    /** Register a character as part of this controller's team */
    UFUNCTION(BlueprintCallable, Category = "Character")
    void RegisterTeamCharacter(AMF_PlayerCharacter *InCharacter);

    /** Unregister a character */
    UFUNCTION(BlueprintCallable, Category = "Character")
    void UnregisterTeamCharacter(AMF_PlayerCharacter *InCharacter);

    /** Get currently controlled character */
    UFUNCTION(BlueprintPure, Category = "Character")
    AMF_PlayerCharacter *GetCurrentCharacter() const;

    /** Switch to a specific character by index */
    UFUNCTION(BlueprintCallable, Category = "Character")
    void SwitchToCharacter(int32 CharacterIndex);

    /** Switch to the next available character */
    UFUNCTION(BlueprintCallable, Category = "Character")
    void SwitchToNextCharacter();

    /** Switch to nearest character to ball */
    UFUNCTION(BlueprintCallable, Category = "Character")
    void SwitchToNearestToBall();

    // ==================== Character Switch Server RPCs ====================
    /** Request to switch character (Client -> Server) */
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_RequestCharacterSwitch(int32 NewIndex);

    // ==================== Client RPCs ====================
    /** Notify client that character switch occurred */
    UFUNCTION(Client, Reliable)
    void Client_OnCharacterSwitched(AMF_PlayerCharacter *NewCharacter);

    // ==================== Events (Blueprint Bindable) ====================
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnControlledCharacterChanged OnControlledCharacterChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTeamAssigned OnTeamAssigned;

    /** Fired when server responds to team request - bind in Widget to handle response */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTeamAssignmentResponseDelegate OnTeamAssignmentResponseReceived;

    /** Fired when spectator state changes */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnSpectatorStateChanged OnSpectatorStateChanged;

    // ==================== Input Handling ====================
    /** Request player switch via input */
    UFUNCTION(BlueprintCallable, Category = "Input")
    void RequestPlayerSwitch();

    /** Request pause */
    UFUNCTION(BlueprintCallable, Category = "Input")
    void RequestPause();

    // ==================== Possession Control ====================
    /** Explicitly possess the first available team character */
    UFUNCTION(BlueprintCallable, Category = "Possession")
    void PossessFirstTeamCharacter();

    /** Possess a specific character directly (Server authoritative) */
    UFUNCTION(BlueprintCallable, Category = "Possession")
    void PossessCharacter(AMF_PlayerCharacter *CharacterToPossess);

    /** Enable spectator mode - unpossess current character (DEPRECATED: Use Server_RequestLeaveTeam) */
    UFUNCTION(BlueprintCallable, Category = "Possession")
    void SetSpectatorMode(bool bEnabled);

    /** Is this controller in spectator mode? (convenience getter) */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Possession")
    bool bIsSpectator;

    // ==================== Interface Implementation Helpers ====================

    /** Set the spectator state (Server only) */
    void SetSpectatorState(EMF_SpectatorState NewState);

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn *InPawn) override;
    virtual void OnUnPossess() override;

    /** Internal: Switch to specified character (Server side) */
    void Internal_SwitchToCharacter(int32 CharacterIndex);

    /** Find the character nearest to the ball */
    int32 FindNearestCharacterToBall() const;

    // ==================== Interface Implementation ====================
    virtual void OnTeamAssignmentResponse_Implementation(bool bSuccess, EMF_TeamID InAssignedTeam, const FString &ErrorMessage) override;
    virtual void OnTeamStateChanged_Implementation(EMF_TeamID NewTeamID, EMF_SpectatorState NewState) override;
    virtual void OnPossessedPawnChanged_Implementation(APawn *NewPawn) override;
    virtual EMF_TeamID GetCurrentTeamID_Implementation() override;
    virtual EMF_SpectatorState GetCurrentSpectatorState_Implementation() override;
    virtual bool IsSpectating_Implementation() override;

public:
    // ==================== Mobile Input Functions (UI Widget Support) ====================

    /** Get current team (convenience wrapper for widgets) */
    UFUNCTION(BlueprintPure, Category = "Team")
    EMF_TeamID GetCurrentTeam() const { return AssignedTeam; }

    /** Apply mobile joystick movement input */
    UFUNCTION(BlueprintCallable, Category = "Input|Mobile")
    void ApplyMobileMovementInput(FVector2D Direction);

    /** Mobile action button pressed */
    UFUNCTION(BlueprintCallable, Category = "Input|Mobile")
    void OnMobileActionPressed();

    /** Mobile action button released */
    UFUNCTION(BlueprintCallable, Category = "Input|Mobile")
    void OnMobileActionReleased();

    /** Set mobile sprint state */
    UFUNCTION(BlueprintCallable, Category = "Input|Mobile")
    void SetMobileSprintState(bool bSprinting);

private:
    /** Cached ball reference for switching logic */
    UPROPERTY()
    TWeakObjectPtr<AActor> CachedBallActor;

    /** Current mobile sprint state */
    bool bMobileSprinting = false;
};
