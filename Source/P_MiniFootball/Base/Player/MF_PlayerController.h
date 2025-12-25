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
class UMF_HUD;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnControlledCharacterChanged, AMF_PlayerController *, Controller, AMF_PlayerCharacter *, NewCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeamAssigned, AMF_PlayerController *, Controller, EMF_TeamID, Team);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTeamAssignmentResponseDelegate, bool, bSuccess, EMF_TeamID, Team, const FString &, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpectatorStateChanged, AMF_PlayerController *, Controller, EMF_SpectatorState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerRoleChanged, AMF_PlayerController *, Controller, bool, bIsPlaying);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPossessedPawnChangedDelegate, AMF_PlayerController *, Controller, APawn *, NewPawn);

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

protected:
    virtual void CreateInputComponent(TSubclassOf<UInputComponent> InputComponentToCreate) override;

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

public:
    UFUNCTION(BlueprintPure, Category = "Team")
    EMF_TeamID GetAssignedTeam() const { return AssignedTeam; }

    UFUNCTION(BlueprintPure, Category = "Spectator")
    EMF_SpectatorState GetSpectatorState() const { return CurrentSpectatorState; }

    /** Server-side: Assign this controller to a team */
    UFUNCTION(BlueprintCallable, Category = "Team")
    void AssignToTeam(EMF_TeamID NewTeam);

    UFUNCTION()
    void OnRep_AssignedTeam();

    // ==================== Team Request Server RPCs (Called from Widgets) ====================

public:
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

    // DELETED: SwitchToNextCharacter - per PLAN.md this was wrong abstraction

    /** Switch to nearest character to ball (per PLAN.md: Q ALWAYS does this) */
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

    /** Fired when role changes between playing and spectating (derived from team/state/possession). */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPlayerRoleChanged OnPlayerRoleChanged;

    /** Fired when possessed pawn changes (OnPossess/OnUnPossess). */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPossessedPawnChangedDelegate OnMFPossessedPawnChanged;

    // ==================== UI Management (PHASE 11-5) ====================

    /** Create (or show) gameplay UI. Default implementation uses MainHUD widget. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MF|UI")
    void CreateGameplayUI();

    /** Create (or show) spectator UI. Default implementation uses MainHUD widget. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MF|UI")
    void CreateSpectatorUI();

    /** Remove all UI widgets created/owned by this controller. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MF|UI")
    void ClearUI();

    /** Returns the currently active HUD widget, if any. */
    UFUNCTION(BlueprintPure, Category = "MF|UI")
    UMF_HUD *GetCurrentHUD() const { return CurrentHUD; }

    // ==================== Input Handling ====================

public:
    /**
     * One-call helper to ensure input bindings exist and the Input Settings list can populate.
     * - Registers this controller with P_MEIS if needed
     * - Optionally creates the template if missing (only supports auto-creating the built-in "Default" template)
     * - Applies the template to this player (also applies to Enhanced Input)
     */
    UFUNCTION(BlueprintCallable, Category = "MF|Input")
    bool EnsureInputProfileReady(const FName TemplateName = FName(TEXT("Default")), bool bCreateTemplateIfMissing = true, bool bApplyEvenIfNotEmpty = false);

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

    UFUNCTION(BlueprintPure, Category = "Possession")
    bool IsSpectator() const { return bIsSpectator; }

    // ==================== Interface Implementation Helpers ====================

    /** Set the spectator state (Server only) */
    void SetSpectatorState(EMF_SpectatorState NewState);

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void OnPossess(APawn *InPawn) override;
    virtual void OnUnPossess() override;

    // ==================== Input System Init (P_MEIS) ====================
    /** Ensure P_MEIS is registered for this local controller. */
    void InitializeInputSystem();

    /** Load an input template/profile for this local controller (e.g. "Default"). */
    void LoadInputProfile(const FName &TemplateName);

    /** Apply the active profile to Enhanced Input. */
    void FinalizeInputSetup();

    /** Recompute role and broadcast if changed. */
    void UpdatePlayerRole();

    UFUNCTION()
    void HandlePlayerRoleChanged(AMF_PlayerController *Controller, bool bIsPlaying);

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

    /** C++ access to registered team characters (server/game logic). */
    const TArray<AMF_PlayerCharacter *> &GetRegisteredTeamCharacters() const { return TeamCharacters; }

    /** Find index of a registered character (or INDEX_NONE). */
    int32 GetRegisteredTeamCharacterIndex(AMF_PlayerCharacter *InCharacter) const { return TeamCharacters.IndexOfByKey(InCharacter); }

    /** Clear all registered team characters (server/game logic). */
    void ResetRegisteredTeamCharacters()
    {
        TeamCharacters.Empty();
        ActiveCharacterIndex = INDEX_NONE;
    }

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

    // Input init state
    bool bInputSystemInitialized = false;
    bool bInputProfileLoaded = false;

    // Role state
    bool bLastKnownIsPlaying = false;

    // UI state
    UPROPERTY(Transient)
    TObjectPtr<UMF_HUD> CurrentHUD;

    UPROPERTY(EditDefaultsOnly, Category = "MF|UI")
    int32 HUDZOrder = 100;
};
