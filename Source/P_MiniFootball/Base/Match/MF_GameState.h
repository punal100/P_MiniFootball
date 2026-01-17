/*
 * @Author: Punal Manalan
 * @Description: MF_GameState - Replicated Game State Header
 *               Manages match state, scores, time, and team data
 *               Full network replication for Listen Server and Dedicated Server
 * @Date: 07/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Core/MF_Types.h"
#include "MF_GameState.generated.h"

class AMF_Ball;
class AMF_PlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScoreChanged, EMF_TeamID, Team, int32, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchPhaseChanged, EMF_MatchPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchTimeUpdated, float, RemainingTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchEnded, EMF_TeamID, WinningTeam);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamRosterChanged, EMF_TeamID, Team);

/**
 * MF_GameState - Networked Game State for Mini Football
 *
 * Responsibilities:
 * - Track and replicate match score
 * - Track and replicate match time
 * - Track match phase (Kickoff, Playing, HalfTime, etc.)
 * - Provide team data to clients
 */
UCLASS()
class P_MINIFOOTBALL_API AMF_GameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AMF_GameState();

    // ==================== Replication ====================
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    // ==================== Match State ====================
    /** Current match phase */
    UPROPERTY(ReplicatedUsing = OnRep_MatchPhase, BlueprintReadOnly, Category = "Match")
    EMF_MatchPhase CurrentPhase;

    /** Team A score */
    UPROPERTY(ReplicatedUsing = OnRep_ScoreTeamA, BlueprintReadOnly, Category = "Match")
    int32 ScoreTeamA;

    /** Team B score */
    UPROPERTY(ReplicatedUsing = OnRep_ScoreTeamB, BlueprintReadOnly, Category = "Match")
    int32 ScoreTeamB;

    /** Remaining match time in seconds */
    UPROPERTY(ReplicatedUsing = OnRep_MatchTimeRemaining, BlueprintReadOnly, Category = "Match")
    float MatchTimeRemaining;

    /** Current half (1 or 2) */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    int32 CurrentHalf;

    /** Which team has kickoff */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    EMF_TeamID KickoffTeam;

    // ==================== Match Configuration ====================
    /** Total match time per half in seconds */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
    float HalfDuration;

    /** Score required to win (0 = time based only) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
    int32 ScoreToWin;

    // ==================== Team Rosters ====================
    /** Team A players */
    UPROPERTY(ReplicatedUsing = OnRep_TeamAPlayers, BlueprintReadOnly, Category = "Teams")
    TArray<AMF_PlayerCharacter *> TeamAPlayers;

    /** Team B players */
    UPROPERTY(ReplicatedUsing = OnRep_TeamBPlayers, BlueprintReadOnly, Category = "Teams")
    TArray<AMF_PlayerCharacter *> TeamBPlayers;

    // ==================== Ball Reference ====================
    /** The match ball */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ball")
    AMF_Ball *MatchBall;

    // ==================== Match Control (Server Only) ====================
    /** Start the match */
    UFUNCTION(BlueprintCallable, Category = "Match")
    void StartMatch();

    /** Pause the match */
    UFUNCTION(BlueprintCallable, Category = "Match")
    void PauseMatch();

    /** Resume the match */
    UFUNCTION(BlueprintCallable, Category = "Match")
    void ResumeMatch();

    /** End the match */
    UFUNCTION(BlueprintCallable, Category = "Match")
    void EndMatch();

    /** Add score to a team */
    UFUNCTION(BlueprintCallable, Category = "Match")
    void AddScore(EMF_TeamID Team, int32 Points = 1);

    /** Set match phase */
    UFUNCTION(BlueprintCallable, Category = "Match")
    void SetMatchPhase(EMF_MatchPhase NewPhase);

    /** Reset for kickoff */
    UFUNCTION(BlueprintCallable, Category = "Match")
    void ResetForKickoff(EMF_TeamID Team);

    /** Register the ball */
    UFUNCTION(BlueprintCallable, Category = "Ball")
    void RegisterBall(AMF_Ball *Ball);

    // ==================== Team Management ====================
    /** Register a player to a team */
    UFUNCTION(BlueprintCallable, Category = "Teams")
    void RegisterPlayer(AMF_PlayerCharacter *Player, EMF_TeamID Team);

    /** Unregister a player */
    UFUNCTION(BlueprintCallable, Category = "Teams")
    void UnregisterPlayer(AMF_PlayerCharacter *Player);

    /** Get all players on a team */
    UFUNCTION(BlueprintPure, Category = "Teams")
    TArray<AMF_PlayerCharacter *> GetTeamPlayers(EMF_TeamID Team) const;

    /** Get player count for a team (for UI widgets) */
    UFUNCTION(BlueprintPure, Category = "Teams")
    int32 GetTeamPlayerCount(EMF_TeamID Team) const;

    /** Get player names for a team (for UI widgets) */
    UFUNCTION(BlueprintPure, Category = "Teams")
    TArray<FString> GetTeamPlayerNames(EMF_TeamID Team) const;

    /** Get team roster data (for UI widgets) */
    UFUNCTION(BlueprintPure, Category = "Teams")
    FMF_TeamRosterData GetTeamRoster(EMF_TeamID Team) const;

    // ==================== State Getters ====================
    UFUNCTION(BlueprintPure, Category = "Match")
    int32 GetScore(EMF_TeamID Team) const;

    UFUNCTION(BlueprintPure, Category = "Match")
    bool IsMatchInProgress() const;

    UFUNCTION(BlueprintPure, Category = "Match")
    EMF_TeamID GetWinningTeam() const;

    UFUNCTION(BlueprintPure, Category = "Match")
    FString GetFormattedTime() const;

    // ==================== Authoritative Getters (per PLAN.md) ====================

    /** Get the active match ball (NEVER spawn - resolve existing) */
    UFUNCTION(BlueprintPure, Category = "Ball")
    AMF_Ball *GetMatchBall() const;

    /** Get team for a controller (authoritative resolution) */
    UFUNCTION(BlueprintPure, Category = "Teams")
    EMF_TeamID GetTeamForController(APlayerController *PC) const;

    /** Check if a team currently has ball possession */
    UFUNCTION(BlueprintPure, Category = "Match")
    bool TeamHasBall(EMF_TeamID Team) const;

    // ==================== Events ====================
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnScoreChanged OnScoreChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnMatchPhaseChanged OnMatchPhaseChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnMatchTimeUpdated OnMatchTimeUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnMatchEnded OnMatchEnded;

    /** Fired when team roster changes (on server and clients via RepNotify) */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTeamRosterChanged OnTeamRosterChanged;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ==================== Rep Notifies ====================
    UFUNCTION()
    void OnRep_MatchPhase();

    UFUNCTION()
    void OnRep_ScoreTeamA();

    UFUNCTION()
    void OnRep_ScoreTeamB();

    UFUNCTION()
    void OnRep_MatchTimeRemaining();

    UFUNCTION()
    void OnRep_TeamAPlayers();

    UFUNCTION()
    void OnRep_TeamBPlayers();

    // ==================== Internal Functions ====================
    void UpdateMatchTimer(float DeltaTime);
    void CheckWinCondition();
    void HandleHalfTime();
    void HandleMatchEnd();

    /** Handler for ball goal event */
    UFUNCTION()
    void HandleGoalScored(AMF_Ball *Ball, EMF_TeamID ScoringTeam);

    /** Notify all running AI that match is now Playing (forces state re-evaluation) */
    void NotifyAIMatchPlaying();

private:
    /** Is match timer running */
    bool bMatchTimerActive;

    /** Timer for delayed operations */
    FTimerHandle PhaseTimerHandle;
};
