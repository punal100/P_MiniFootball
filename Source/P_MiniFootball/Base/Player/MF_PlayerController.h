/*
 * @Author: Punal Manalan
 * @Description: MF_PlayerController - Networked Player Controller Header
 *               Manages player possession, team assignment, and character switching
 *               Works with both Listen Server and Dedicated Server
 * @Date: 07/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Core/MF_Types.h"
#include "MF_PlayerController.generated.h"

class AMF_PlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnControlledCharacterChanged, AMF_PlayerController *, Controller, AMF_PlayerCharacter *, NewCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeamAssigned, AMF_PlayerController *, Controller, EMF_TeamID, Team);

/**
 * MF_PlayerController - Networked Player Controller for Mini Football
 *
 * Responsibilities:
 * - Manages which character the player is currently controlling
 * - Handles player switching between team characters
 * - Receives team assignment from server
 * - Supports both Listen Server and Dedicated Server configurations
 */
UCLASS()
class P_MINIFOOTBALL_API AMF_PlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AMF_PlayerController();

    // ==================== Replication ====================
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    // ==================== Team Management ====================
    /** Assigned team (Server authoritative) */
    UPROPERTY(ReplicatedUsing = OnRep_AssignedTeam, BlueprintReadOnly, Category = "Team")
    EMF_TeamID AssignedTeam;

    /** Server-side: Assign this controller to a team */
    UFUNCTION(BlueprintCallable, Category = "Team")
    void AssignToTeam(EMF_TeamID NewTeam);

    UFUNCTION()
    void OnRep_AssignedTeam();

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

    // ==================== Server RPCs ====================
    /** Request to switch character (Client -> Server) */
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_RequestCharacterSwitch(int32 NewIndex);

    // ==================== Client RPCs ====================
    /** Notify client that character switch occurred */
    UFUNCTION(Client, Reliable)
    void Client_OnCharacterSwitched(AMF_PlayerCharacter *NewCharacter);

    // ==================== Events ====================
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnControlledCharacterChanged OnControlledCharacterChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTeamAssigned OnTeamAssigned;

    // ==================== Input Handling ====================
    /** Request player switch via input */
    UFUNCTION(BlueprintCallable, Category = "Input")
    void RequestPlayerSwitch();

    /** Request pause */
    UFUNCTION(BlueprintCallable, Category = "Input")
    void RequestPause();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn *InPawn) override;
    virtual void OnUnPossess() override;

    /** Internal: Switch to specified character (Server side) */
    void Internal_SwitchToCharacter(int32 CharacterIndex);

    /** Find the character nearest to the ball */
    int32 FindNearestCharacterToBall() const;

private:
    /** Cached ball reference for switching logic */
    UPROPERTY()
    TWeakObjectPtr<AActor> CachedBallActor;
};
