/*
 * @Author: Punal Manalan
 * @Description: MF_TeamSelectionPopup - Modal team selection dialog
 * @Date: 10/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/MF_Types.h"
#include "MF_TeamSelectionPopup.generated.h"

// Forward declarations
class UMF_TeamPanel;
class UTextBlock;
class UButton;
class UCanvasPanel;
class UOverlay;
class AMF_PlayerController;
class AMF_GameState;

/** Delegate for when popup is closed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMF_OnPopupClosed);

/** Delegate for when a team is selected */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMF_OnTeamSelected, EMF_TeamID, SelectedTeam);

/**
 * UMF_TeamSelectionPopup
 * Full-screen modal dialog for detailed team selection.
 * Shows both teams with full roster information and join buttons.
 *
 * Design Notes:
 * - Modal popup that blocks other UI interaction
 * - Shows detailed team info via MF_TeamPanel widgets
 * - Enforces team balance rules
 * - Can be dismissed without selecting
 */
UCLASS()
class P_MINIFOOTBALL_API UMF_TeamSelectionPopup : public UUserWidget
{
    GENERATED_BODY()

public:
    //~ Begin UUserWidget Interface
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    //~ End UUserWidget Interface

    /**
     * Shows the popup with animation.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|Popup")
    void ShowPopup();

    /**
     * Hides the popup with animation.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|Popup")
    void HidePopup();

    /**
     * Checks if popup is currently visible.
     */
    UFUNCTION(BlueprintPure, Category = "MF|Popup")
    bool IsPopupVisible() const { return bIsVisible; }

    /**
     * Refreshes team data display.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|Popup")
    void RefreshTeamData();

    /** Broadcast when popup is closed */
    UPROPERTY(BlueprintAssignable, Category = "MF|Events")
    FMF_OnPopupClosed OnPopupClosed;

    /** Broadcast when a team is selected (before joining) */
    UPROPERTY(BlueprintAssignable, Category = "MF|Events")
    FMF_OnTeamSelected OnTeamSelected;

protected:
    /** Title text for the popup */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UTextBlock> TitleText;

    /** Team A panel with full roster */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "MF|Components")
    TObjectPtr<UMF_TeamPanel> TeamAPanel;

    /** Team B panel with full roster */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "MF|Components")
    TObjectPtr<UMF_TeamPanel> TeamBPanel;

    /** Auto-assign button for balanced team selection */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UButton> AutoAssignButton;

    /** Close/cancel button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "MF|Components")
    TObjectPtr<UButton> CloseButton;

    /** Background overlay for modal effect */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UOverlay> BackgroundOverlay;

    /** Status text showing join result or errors */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UTextBlock> StatusText;

    /** Whether popup is currently visible */
    UPROPERTY(BlueprintReadOnly, Category = "MF|State")
    bool bIsVisible = false;

public:
    // ==================== Widget Specification (JSON) ====================
    /**
     * Self-describing JSON specification for automated Widget Blueprint creation.
     * Used by MF_WidgetBlueprintCreator.py to construct WBP_MF_TeamSelectionPopup.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MiniFootball|WidgetSpec")
    static const FString &GetWidgetSpec();

private:
    /** Event handlers */
    UFUNCTION()
    void HandleTeamAJoinClicked(EMF_TeamID TeamID);

    UFUNCTION()
    void HandleTeamBJoinClicked(EMF_TeamID TeamID);

    UFUNCTION()
    void HandleAutoAssignClicked();

    UFUNCTION()
    void HandleCloseClicked();

    /** Background click handler (for dismissing) */
    UFUNCTION()
    void HandleBackgroundClicked();

    /** Requests to join a specific team */
    void RequestJoinTeam(EMF_TeamID TeamID);

    /** Updates join button enabled states based on balance */
    void UpdateJoinButtonStates();

    /** Displays status message */
    void ShowStatus(const FString &Message);

    /** Clears status message */
    void ClearStatus();

    /** Helper getters */
    AMF_PlayerController *GetMFPlayerController() const;
    AMF_GameState *GetGameState() const;
};
