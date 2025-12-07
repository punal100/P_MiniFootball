/*
 * @Author: Punal Manalan
 * @Description: MF_GameMode - Server-Only Game Mode Header
 *               Manages match setup, player spawning, and team assignment
 *               Works with both Listen Server and Dedicated Server
 * @Date: 07/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/MF_Types.h"
#include "MF_GameMode.generated.h"

class AMF_PlayerCharacter;
class AMF_PlayerController;
class AMF_Ball;
class AMF_GameState;

/**
 * MF_GameMode - Server-Only Game Mode for Mini Football
 *
 * Responsibilities:
 * - Spawn players and assign teams
 * - Spawn ball
 * - Manage match flow (start, restart)
 * - Handle player connections/disconnections
 */
UCLASS()
class P_MINIFOOTBALL_API AMF_GameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMF_GameMode();

    // ==================== Configuration ====================
    /** Number of players per team */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
    int32 PlayersPerTeam;

    /** Player character class to spawn */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
    TSubclassOf<AMF_PlayerCharacter> PlayerCharacterClass;

    /** Ball class to spawn */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
    TSubclassOf<AMF_Ball> BallClass;

    /** Player spawn locations for Team A */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
    TArray<FVector> TeamASpawnLocations;

    /** Player spawn locations for Team B */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
    TArray<FVector> TeamBSpawnLocations;

    // ==================== Match Control ====================
    /** Start a new match */
    UFUNCTION(BlueprintCallable, Category = "Match")
    void StartNewMatch();

    /** Restart the current match */
    UFUNCTION(BlueprintCallable, Category = "Match")
    void RestartMatch();

    /** Spawn all team characters */
    UFUNCTION(BlueprintCallable, Category = "Match")
    void SpawnTeams();

    /** Spawn the ball */
    UFUNCTION(BlueprintCallable, Category = "Match")
    void SpawnBall();

    // ==================== Player Management ====================
    /** Get current MF_GameState */
    UFUNCTION(BlueprintPure, Category = "Match")
    AMF_GameState *GetMFGameState() const;

    /** Assign a player controller to a team */
    UFUNCTION(BlueprintCallable, Category = "Teams")
    void AssignPlayerToTeam(AMF_PlayerController *PC, EMF_TeamID Team);

    /** Get next available team (for auto-assignment) */
    UFUNCTION(BlueprintPure, Category = "Teams")
    EMF_TeamID GetNextAvailableTeam() const;

    // ==================== Possession Control ====================
    /** Make a controller possess a specific character (Server only) */
    UFUNCTION(BlueprintCallable, Category = "Possession")
    bool PossessCharacterWithController(AMF_PlayerController *PC, AMF_PlayerCharacter *Character);

    /** Make a controller possess first available character from their team */
    UFUNCTION(BlueprintCallable, Category = "Possession")
    bool PossessFirstAvailableTeamCharacter(AMF_PlayerController *PC);

    /** Make a controller possess a character by PlayerID within their team */
    UFUNCTION(BlueprintCallable, Category = "Possession")
    bool PossessTeamCharacterByIndex(AMF_PlayerController *PC, int32 PlayerIndex);

    /** Get all spawned characters for a team */
    UFUNCTION(BlueprintPure, Category = "Teams")
    TArray<AMF_PlayerCharacter *> GetSpawnedTeamCharacters(EMF_TeamID Team) const;

    /** Get all connected player controllers */
    UFUNCTION(BlueprintPure, Category = "Players")
    TArray<AMF_PlayerController *> GetAllMFPlayerControllers() const;

    /** Find a character by PlayerID and Team */
    UFUNCTION(BlueprintPure, Category = "Teams")
    AMF_PlayerCharacter *FindCharacterByTeamAndIndex(EMF_TeamID Team, int32 PlayerIndex) const;

    /** Register all team characters to a controller (call after spawn) */
    UFUNCTION(BlueprintCallable, Category = "Possession")
    void RegisterTeamCharactersToController(AMF_PlayerController *PC);

protected:
    // ==================== Game Mode Overrides ====================
    virtual void InitGame(const FString &MapName, const FString &Options, FString &ErrorMessage) override;
    virtual void BeginPlay() override;
    virtual void PostLogin(APlayerController *NewPlayer) override;
    virtual void Logout(AController *Exiting) override;
    virtual AActor *ChoosePlayerStart_Implementation(AController *Player) override;
    virtual UClass *GetDefaultPawnClassForController_Implementation(AController *InController) override;

    /** Spawn a single team character at location */
    AMF_PlayerCharacter *SpawnTeamCharacter(EMF_TeamID Team, int32 SpawnIndex);

    /** Setup default spawn locations if not configured */
    void SetupDefaultSpawnLocations();

private:
    /** Track spawned characters */
    UPROPERTY()
    TArray<AMF_PlayerCharacter *> SpawnedCharacters;

    /** The match ball */
    UPROPERTY()
    AMF_Ball *SpawnedBall;

    /** Track team player counts for balancing */
    int32 TeamAPlayerCount;
    int32 TeamBPlayerCount;
};
