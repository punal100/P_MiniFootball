/*
 * @Author: Punal Manalan
 * @Description: MF_QuickTeamPanel - Compact team preview widget for spectator mode
 * @Date: 10/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/MF_Types.h"
#include "MF_QuickTeamPanel.generated.h"

// Forward declarations
class UTextBlock;
class UButton;
class UVerticalBox;
class UBorder;
class AMF_GameState;

/** Delegate fired when quick join button is clicked */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMF_OnQuickJoinClicked, EMF_TeamID, TeamID);

/**
 * UMF_QuickTeamPanel - Compact team preview widget
 *
 * Shows condensed team info for quick team selection in spectator mode:
 * - Team name with player count
 * - Compact player list (2-3 names)
 * - Quick join button with keyboard shortcut hint
 *
 * Used in UMF_SpectatorControls for quick team joining
 */
UCLASS(Blueprintable, BlueprintType)
class P_MINIFOOTBALL_API UMF_QuickTeamPanel : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Initialize the panel with a team ID */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|QuickTeamPanel")
    void SetTeamID(EMF_TeamID InTeamID);

    /** Get the team ID this panel represents */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|QuickTeamPanel")
    EMF_TeamID GetTeamID() const { return TeamID; }

    /** Refresh team data from GameState */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|QuickTeamPanel")
    void RefreshTeamData();

    /** Set player count directly */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|QuickTeamPanel")
    void SetPlayerCount(int32 Count);

    /** Enable/disable quick join button */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|QuickTeamPanel")
    void SetQuickJoinEnabled(bool bEnabled);

    /** Set keyboard shortcut hint text */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|QuickTeamPanel")
    void SetShortcutHint(const FString &HintText);

    /** Delegate for quick join click - bind in parent widget */
    UPROPERTY(BlueprintAssignable, Category = "MF|UI|QuickTeamPanel")
    FMF_OnQuickJoinClicked OnQuickJoinClicked;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** Handle quick join button click */
    UFUNCTION()
    void HandleQuickJoinClicked();

    // ==================== Widget Bindings ====================

    /** Panel border for team color */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> PanelBorder;

    /** Team name text */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TeamNameText;

    /** Player count text in header */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> PlayerCountText;

    /** Compact player list */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UVerticalBox> PlayerListBox;

    /** Quick join button */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> QuickJoinButton;

    /** Keyboard shortcut hint */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> ShortcutHintText;

    // ==================== Configuration ====================

    /** Team color for Team A */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|QuickTeamPanel")
    FLinearColor TeamAColor = FLinearColor(0.8f, 0.2f, 0.2f, 0.8f);

    /** Team color for Team B */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|QuickTeamPanel")
    FLinearColor TeamBColor = FLinearColor(0.2f, 0.2f, 0.8f, 0.8f);

    /** Max players to display in compact list */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|QuickTeamPanel")
    int32 MaxDisplayedPlayers = 3;

public:
    // ==================== Widget Specification (JSON) ====================
    /**
     * Self-describing JSON specification for automated Widget Blueprint creation.
     * Used by MF_WidgetBlueprintCreator.py to construct WBP_MF_QuickTeamPanel.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MiniFootball|WidgetSpec")
    static FString GetWidgetSpec();

private:
    /** Current team ID this panel represents */
    EMF_TeamID TeamID = EMF_TeamID::None;

    /** Cached player count */
    int32 CachedPlayerCount = 0;

    /** Update visual appearance based on team */
    void UpdateTeamVisuals();

    /** Get current GameState */
    AMF_GameState *GetGameState() const;
};
