/*
 * @Author: Punal Manalan
 * @Description: MF_WidgetClassSettings - Configurable widget class references (no hard-coded /Game paths in C++)
 * @Date: 14/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MF_WidgetClassSettings.generated.h"

class UMF_MainMenu;
class UMF_MainSettings;
class UMF_InputSettings;
class UMF_AudioSettings;
class UMF_GraphicsSettings;
class UMF_HUD;
class UMF_MatchInfo;
class UMF_TeamIndicator;
class UMF_GameplayControls;
class UMF_SpectatorControls;
class UMF_TeamSelectionPopup;
class UMF_TransitionOverlay;
class UMF_ScorePopup;
class UMF_PauseMenu;
class UMF_VirtualJoystick;
class UMF_ActionButton;
class UMF_ToggleActionButton;
class UMF_TeamPanel;
class UMF_QuickTeamPanel;
class UMF_InputActionRow;

/**
 * Project-configurable widget class references used as fallbacks when per-widget `TSubclassOf<>` properties are unset.
 *
 * NOTE: This intentionally avoids any hard-coded asset paths in C++.
 * Configure these via Project Settings or DefaultGame.ini.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "MiniFootball Widget Classes"))
class P_MINIFOOTBALL_API UMF_WidgetClassSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    // Root

    /** Widget blueprint/class to use for the main HUD (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Root")
    TSoftClassPtr<UMF_HUD> MainHUDClass;

    /** Widget blueprint/class to use for the main menu (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Root")
    TSoftClassPtr<UMF_MainMenu> MainMenuClass;

    /** Widget blueprint/class to use for the pause menu (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Root")
    TSoftClassPtr<UMF_PauseMenu> PauseMenuClass;

    // Settings

    /** Widget blueprint/class to use for the main settings overlay (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Settings")
    TSoftClassPtr<UMF_MainSettings> MainSettingsClass;

    /** Widget blueprint/class to use for input settings overlay (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Settings")
    TSoftClassPtr<UMF_InputSettings> InputSettingsClass;

    /** Widget blueprint/class to use for audio settings overlay (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Settings")
    TSoftClassPtr<UMF_AudioSettings> AudioSettingsClass;

    /** Widget blueprint/class to use for graphics settings overlay (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Settings")
    TSoftClassPtr<UMF_GraphicsSettings> GraphicsSettingsClass;

    // Gameplay UI

    /** Widget blueprint/class to use for match info (score/time) (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Gameplay")
    TSoftClassPtr<UMF_MatchInfo> MatchInfoClass;

    /** Widget blueprint/class to use for team indicator (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Gameplay")
    TSoftClassPtr<UMF_TeamIndicator> TeamIndicatorClass;

    /** Widget blueprint/class to use for gameplay controls (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Gameplay")
    TSoftClassPtr<UMF_GameplayControls> GameplayControlsClass;

    /** Widget blueprint/class to use for spectator controls (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Gameplay")
    TSoftClassPtr<UMF_SpectatorControls> SpectatorControlsClass;

    // Overlays & Popups

    /** Widget blueprint/class to use for team selection popup (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Overlays")
    TSoftClassPtr<UMF_TeamSelectionPopup> TeamSelectionPopupClass;

    /** Widget blueprint/class to use for transition overlay (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Overlays")
    TSoftClassPtr<UMF_TransitionOverlay> TransitionOverlayClass;

    /** Widget blueprint/class to use for score popup (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Overlays")
    TSoftClassPtr<UMF_ScorePopup> ScorePopupClass;

    // Controls

    /** Widget blueprint/class to use for virtual joystick (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Controls")
    TSoftClassPtr<UMF_VirtualJoystick> VirtualJoystickClass;

    /** Widget blueprint/class to use for action button (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Controls")
    TSoftClassPtr<UMF_ActionButton> ActionButtonClass;

    /** Widget blueprint/class to use for toggle action button (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Controls")
    TSoftClassPtr<UMF_ToggleActionButton> ToggleActionButtonClass;

    // Team & Player Display

    /** Widget blueprint/class to use for team panel (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Team")
    TSoftClassPtr<UMF_TeamPanel> TeamPanelClass;

    /** Widget blueprint/class to use for quick team panel (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Team")
    TSoftClassPtr<UMF_QuickTeamPanel> QuickTeamPanelClass;

    // Input UI

    /** Widget blueprint/class to use for the runtime-created input action row (fallback). */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|Input")
    TSoftClassPtr<UMF_InputActionRow> InputActionRowClass;

    /** If enabled, load widget class overrides from a JSON file at engine startup. */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|JSON")
    bool bAutoLoadJsonConfig = false;

    /** JSON path (relative to project dir) containing widget type -> class path overrides. */
    UPROPERTY(Config, EditAnywhere, Category = "Widgets|JSON")
    FString JsonConfigPath = TEXT("Saved/WidgetConfig.json");

    virtual FName GetCategoryName() const override
    {
        return TEXT("Game");
    }
};
