/*
 * @Author: Punal Manalan
 * @Description: MF_TeamPanel - Reusable team info panel widget
 * @Date: 10/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/MF_Types.h"
#include "MF_TeamPanel.generated.h"

// Forward declarations
class UTextBlock;
class UButton;
class UVerticalBox;
class UBorder;

/** Delegate fired when join button is clicked */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMF_OnJoinTeamClicked, EMF_TeamID, TeamID);

/**
 * UMF_TeamPanel - Reusable team info panel widget
 *
 * Displays team information including:
 * - Team name and color
 * - Player count (X/MaxPlayers)
 * - List of player names
 * - Join team button
 *
 * Used in UMF_TeamSelectionPopup for team selection
 */
UCLASS()
class P_MINIFOOTBALL_API UMF_TeamPanel : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Initialize the panel with a team ID */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|TeamPanel")
    void SetTeamID(EMF_TeamID InTeamID);

    /** Get the team ID this panel represents */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|TeamPanel")
    EMF_TeamID GetTeamID() const { return TeamID; }

    /** Update player list display (with default max players) */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|TeamPanel")
    void SetPlayerData(const TArray<FString> &PlayerNames);

    /** Update player list display with specific max */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|TeamPanel")
    void SetPlayerDataWithMax(const TArray<FString> &PlayerNames, int32 MaxPlayers);

    /** Enable/disable the join button */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|TeamPanel")
    void SetJoinButtonEnabled(bool bEnabled);

    /** Set button text (e.g., "JOIN TEAM A" or "LEAVE TEAM") */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|TeamPanel")
    void SetJoinButtonText(const FString &NewText);

    /** Delegate for join button click - bind in parent widget */
    UPROPERTY(BlueprintAssignable, Category = "MF|UI|TeamPanel")
    FMF_OnJoinTeamClicked OnJoinClicked;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** Handle join button click */
    UFUNCTION()
    void HandleJoinButtonClicked();

    // ==================== Widget Bindings ====================
    // Use BindWidget meta for UMG designer support

    /** Border for team color */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> PanelBorder;

    /** Team name text */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TeamNameText;

    /** Player count text (e.g., "Players: 2/3") */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> PlayerCountText;

    /** Container for player names */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> PlayerListBox;

    /** Join team button */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> JoinButton;

    /** Join button text */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> JoinButtonText;

    // ==================== Configuration ====================

    /** Team color for Team A */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|TeamPanel")
    FLinearColor TeamAColor = FLinearColor(0.8f, 0.2f, 0.2f, 1.0f); // Red

    /** Team color for Team B */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|TeamPanel")
    FLinearColor TeamBColor = FLinearColor(0.2f, 0.2f, 0.8f, 1.0f); // Blue

private:
    /** Current team ID this panel represents */
    EMF_TeamID TeamID = EMF_TeamID::None;

    /** Update visual appearance based on team */
    void UpdateTeamVisuals();

    /** Create a text block for player name display */
    UTextBlock *CreatePlayerNameText(const FString &PlayerName);
};
