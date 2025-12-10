/*
 * @Author: Punal Manalan
 * @Description: MF_MatchInfo - Score, time, and match phase display widget
 * @Date: 10/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/MF_Types.h"
#include "MF_MatchInfo.generated.h"

// Forward declarations
class UTextBlock;
class AMF_GameState;

/**
 * UMF_MatchInfo - Score and match info display widget
 *
 * Displays:
 * - Team A score
 * - Team B score
 * - Match timer
 * - Current match phase
 *
 * Binds to GameState replication for auto-updates
 */
UCLASS()
class P_MINIFOOTBALL_API UMF_MatchInfo : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Force refresh from GameState */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|MatchInfo")
    void RefreshMatchInfo();

    /** Manually set scores (for testing) */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|MatchInfo")
    void SetScores(int32 TeamAScore, int32 TeamBScore);

    /** Manually set match time */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|MatchInfo")
    void SetMatchTime(float TimeRemaining);

    /** Manually set match phase */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|MatchInfo")
    void SetMatchPhase(EMF_MatchPhase Phase);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry &MyGeometry, float InDeltaTime) override;

    // ==================== Widget Bindings ====================

    /** Team A score text */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TeamAScoreText;

    /** Team B score text */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TeamBScoreText;

    /** Match timer text */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> MatchTimerText;

    /** Match phase indicator */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> MatchPhaseText;

    /** Team A name text */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TeamANameText;

    /** Team B name text */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TeamBNameText;

    // ==================== Configuration ====================

    /** Update interval for refreshing from GameState */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|MatchInfo")
    float UpdateInterval = 0.1f;

private:
    /** Timer for periodic updates */
    float UpdateTimer = 0.0f;

    /** Cached values to detect changes */
    int32 CachedTeamAScore = -1;
    int32 CachedTeamBScore = -1;
    float CachedTimeRemaining = -1.0f;
    EMF_MatchPhase CachedPhase = EMF_MatchPhase::WaitingForPlayers;

    /** Get current GameState */
    AMF_GameState *GetGameState() const;

    /** Update timer display */
    void UpdateTimerDisplay(float TimeRemaining);

    /** Get display string for match phase */
    FString GetPhaseDisplayString(EMF_MatchPhase Phase) const;
};
