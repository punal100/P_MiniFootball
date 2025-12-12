/*
 * @Author: Punal Manalan
 * @Description: MF_HUD - Main HUD container with widget switcher for game modes
 * @Date: 10/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/MF_Types.h"
#include "MF_HUD.generated.h"

// Forward declarations
class UMF_MatchInfo;
class UMF_TeamIndicator;
class UMF_TransitionOverlay;
class UMF_SpectatorControls;
class UMF_GameplayControls;
class UMF_TeamSelectionPopup;
class UMF_PauseMenu;
class UWidgetSwitcher;
class UCanvasPanel;
class AMF_PlayerController;
class AMF_GameState;

/** HUD mode enum for widget switching */
UENUM(BlueprintType)
enum class EMF_HUDMode : uint8
{
    /** Spectating the game */
    Spectator,
    /** Active gameplay on a team */
    Gameplay,
    /** In a menu or popup */
    Menu,
    /** Transitioning between states */
    Transition
};

/**
 * UMF_HUD
 * Main HUD container that manages all gameplay UI.
 * Uses widget switcher to toggle between spectator and gameplay modes.
 *
 * Design Notes:
 * - Single root widget containing all HUD elements
 * - Widget switcher toggles between spectator/gameplay controls
 * - Overlay elements (match info, team indicator) always visible
 * - Handles modal popups (team selection, pause menu)
 * - Responds to player state changes from controller
 */
UCLASS(Blueprintable, BlueprintType)
class P_MINIFOOTBALL_API UMF_HUD : public UUserWidget
{
    GENERATED_BODY()

public:
    //~ Begin UUserWidget Interface
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry &MyGeometry, float InDeltaTime) override;
    //~ End UUserWidget Interface

    /**
     * Sets the HUD mode, switching visible widgets.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|HUD")
    void SetHUDMode(EMF_HUDMode NewMode);

    /**
     * Gets the current HUD mode.
     */
    UFUNCTION(BlueprintPure, Category = "MF|HUD")
    EMF_HUDMode GetHUDMode() const { return CurrentMode; }

    /**
     * Shows the team selection popup.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|HUD")
    void ShowTeamSelectionPopup();

    /**
     * Hides the team selection popup.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|HUD")
    void HideTeamSelectionPopup();

    /**
     * Shows the pause menu.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|HUD")
    void ShowPauseMenu();

    /**
     * Hides the pause menu.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|HUD")
    void HidePauseMenu();

    /**
     * Toggles pause menu visibility.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|HUD")
    void TogglePauseMenu();

    /**
     * Shows transition overlay with message.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|HUD")
    void ShowTransitionOverlay(const FString &Message);

    /**
     * Hides transition overlay.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|HUD")
    void HideTransitionOverlay();

    /**
     * Updates HUD based on player state.
     * Called automatically when player state changes.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|HUD")
    void RefreshFromPlayerState();

protected:
    //-- Always-Visible Components --//

    /** Match info display (score, time) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "MF|Components")
    TObjectPtr<UMF_MatchInfo> MatchInfo;

    /** Current team indicator */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "MF|Components")
    TObjectPtr<UMF_TeamIndicator> TeamIndicator;

    //-- Mode-Specific Widgets (via Switcher) --//

    /** Widget switcher for mode-based UI */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "MF|Components")
    TObjectPtr<UWidgetSwitcher> ModeSwitcher;

    /** Spectator mode controls */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "MF|Components")
    TObjectPtr<UMF_SpectatorControls> SpectatorControls;

    /** Gameplay mode controls */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "MF|Components")
    TObjectPtr<UMF_GameplayControls> GameplayControls;

    //-- Overlay Widgets --//

    /** Transition overlay for loading/state changes */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UMF_TransitionOverlay> TransitionOverlay;

    //-- Popup Widgets --//

    /** Team selection popup (modal) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UMF_TeamSelectionPopup> TeamSelectionPopup;

    /** Pause menu (modal) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UMF_PauseMenu> PauseMenu;

    //-- Container --//

    /** Root canvas panel */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UCanvasPanel> RootCanvas;

    //-- State --//

    /** Current HUD mode */
    UPROPERTY(BlueprintReadOnly, Category = "MF|State")
    EMF_HUDMode CurrentMode = EMF_HUDMode::Spectator;

    /** Previous mode for restoration */
    UPROPERTY(BlueprintReadOnly, Category = "MF|State")
    EMF_HUDMode PreviousMode = EMF_HUDMode::Spectator;

    /** Refresh interval for periodic updates */
    UPROPERTY(EditDefaultsOnly, Category = "MF|Config")
    float RefreshInterval = 0.5f;

public:
    // ==================== Widget Specification (JSON) ====================
    /**
     * Self-describing JSON specification for automated Widget Blueprint creation.
     * Used by MF_WidgetBlueprintCreator.py to construct WBP_MF_HUD.
     *
     * This is the MASTER HUD widget that contains all other MF widgets.
     * Dependencies must be created in order before creating this widget.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MiniFootball|WidgetSpec")
    static FString GetWidgetSpec();

private:
    /** Cached player controller */
    UPROPERTY()
    TObjectPtr<AMF_PlayerController> CachedPlayerController;

    /** Timer for periodic refresh */
    float RefreshTimer = 0.0f;

    /** Event handlers */
    UFUNCTION()
    void HandleOpenTeamSelection();

    UFUNCTION()
    void HandleTeamSelectionClosed();

    UFUNCTION()
    void HandlePauseMenuClosed();

    /** Updates widget switcher to match current mode */
    void UpdateWidgetSwitcher();

    /** Helper getters */
    AMF_PlayerController *GetMFPlayerController();
    AMF_GameState *GetGameState() const;

    /** Determines appropriate mode based on player state */
    EMF_HUDMode DetermineAppropriateMode() const;
};
