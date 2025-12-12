/*
 * @Author: Punal Manalan
 * @Description: MF_PauseMenu - In-game pause menu with team and match options
 * @Date: 10/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/MF_Types.h"
#include "MF_PauseMenu.generated.h"

// Forward declarations
class UTextBlock;
class UButton;
class UVerticalBox;
class UOverlay;
class AMF_PlayerController;
class AMF_GameState;

/** Delegate for when resume is clicked */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMF_OnResumeClicked);

/** Delegate for when leave team is clicked */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMF_OnLeaveTeamClicked);

/** Delegate for when quit to menu is clicked */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMF_OnQuitToMenuClicked);

/**
 * UMF_PauseMenu
 * In-game pause menu providing player options.
 * Includes resume, leave team (return to spectator), and quit options.
 *
 * Design Notes:
 * - Pauses local game input when shown
 * - Leave team returns player to spectator mode
 * - Shows current team status
 */
UCLASS(Blueprintable, BlueprintType)
class P_MINIFOOTBALL_API UMF_PauseMenu : public UUserWidget
{
    GENERATED_BODY()

public:
    //~ Begin UUserWidget Interface
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    //~ End UUserWidget Interface

    /**
     * Shows the pause menu.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|PauseMenu")
    void ShowMenu();

    /**
     * Hides the pause menu.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|PauseMenu")
    void HideMenu();

    /**
     * Toggles pause menu visibility.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|PauseMenu")
    void ToggleMenu();

    /**
     * Checks if menu is currently visible.
     */
    UFUNCTION(BlueprintPure, Category = "MF|PauseMenu")
    bool IsMenuVisible() const { return bIsVisible; }

    /**
     * Updates the menu state based on current player status.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|PauseMenu")
    void RefreshMenuState();

    /** Broadcast when resume is clicked */
    UPROPERTY(BlueprintAssignable, Category = "MF|Events")
    FMF_OnResumeClicked OnResumeClicked;

    /** Broadcast when leave team is clicked */
    UPROPERTY(BlueprintAssignable, Category = "MF|Events")
    FMF_OnLeaveTeamClicked OnLeaveTeamClicked;

    /** Broadcast when quit is clicked */
    UPROPERTY(BlueprintAssignable, Category = "MF|Events")
    FMF_OnQuitToMenuClicked OnQuitToMenuClicked;

protected:
    /** Title text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UTextBlock> TitleText;

    /** Current team display text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UTextBlock> CurrentTeamText;

    /** Resume game button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "MF|Components")
    TObjectPtr<UButton> ResumeButton;

    /** Leave team button (only visible when on a team) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "MF|Components")
    TObjectPtr<UButton> LeaveTeamButton;

    /** Team selection button (opens team selection popup) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UButton> ChangeTeamButton;

    /** Settings button (optional) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UButton> SettingsButton;

    /** Quit to main menu button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "MF|Components")
    TObjectPtr<UButton> QuitButton;

    /** Container for menu items */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UVerticalBox> MenuContainer;

    /** Background overlay */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UOverlay> BackgroundOverlay;

    /** Whether menu is currently visible */
    UPROPERTY(BlueprintReadOnly, Category = "MF|State")
    bool bIsVisible = false;

public:
    // ==================== Widget Specification (JSON) ====================
    /**
     * Self-describing JSON specification for automated Widget Blueprint creation.
     * Used by MF_WidgetBlueprintCreator.py to construct WBP_MF_PauseMenu.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MiniFootball|WidgetSpec")
    static FString GetWidgetSpec();

private:
    /** Button event handlers */
    UFUNCTION()
    void HandleResumeClicked();

    UFUNCTION()
    void HandleLeaveTeamClicked();

    UFUNCTION()
    void HandleChangeTeamClicked();

    UFUNCTION()
    void HandleSettingsClicked();

    UFUNCTION()
    void HandleQuitClicked();

    /** Updates leave team button visibility based on current team */
    void UpdateLeaveTeamButtonVisibility();

    /** Updates current team display */
    void UpdateCurrentTeamDisplay();

    /** Helper getters */
    AMF_PlayerController *GetMFPlayerController() const;
    AMF_GameState *GetGameState() const;
    EMF_TeamID GetCurrentTeam() const;
};
