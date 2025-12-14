/*
 * @Author: Punal Manalan
 * @Description: MF_WidgetTypes - Enum + helpers for modular widget resolution
 * @Date: 14/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "MF_WidgetTypes.generated.h"

/** Enumeration for known UI widget roles in P_MiniFootball. */
UENUM(BlueprintType)
enum class EMF_WidgetType : uint8
{
    // Root
    MainHUD UMETA(DisplayName = "Main HUD"),
    MainMenu UMETA(DisplayName = "Main Menu"),
    PauseMenu UMETA(DisplayName = "Pause Menu"),

    // Settings
    MainSettings UMETA(DisplayName = "Main Settings"),
    InputSettings UMETA(DisplayName = "Input Settings"),
    AudioSettings UMETA(DisplayName = "Audio Settings"),
    GraphicsSettings UMETA(DisplayName = "Graphics Settings"),

    // Gameplay UI
    HUD UMETA(DisplayName = "HUD"),
    MatchInfo UMETA(DisplayName = "Match Info"),
    TeamIndicator UMETA(DisplayName = "Team Indicator"),
    GameplayControls UMETA(DisplayName = "Gameplay Controls"),

    // Overlays & Popups
    TeamSelectionPopup UMETA(DisplayName = "Team Selection Popup"),
    TransitionOverlay UMETA(DisplayName = "Transition Overlay"),
    ScorePopup UMETA(DisplayName = "Score Popup"),

    // Controls
    VirtualJoystick UMETA(DisplayName = "Virtual Joystick"),
    ActionButton UMETA(DisplayName = "Action Button"),
    ToggleActionButton UMETA(DisplayName = "Toggle Action Button"),

    // Team & Player Display
    TeamPanel UMETA(DisplayName = "Team Panel"),
    QuickTeamPanel UMETA(DisplayName = "Quick Team Panel"),
    SpectatorControls UMETA(DisplayName = "Spectator Controls"),

    // Input UI
    InputActionRow UMETA(DisplayName = "Input Action Row"),

    /** Use Blueprint string keys via UMF_WidgetConfigurationSubsystem (Register/Get by key). */
    CustomByString UMETA(DisplayName = "Custom By String"),

    // Misc
    Unknown UMETA(DisplayName = "Unknown"),
};

namespace MF_WidgetTypes
{
    /** Stable string keys for JSON / config files. */
    inline FString ToKey(const EMF_WidgetType Type)
    {
        switch (Type)
        {
        case EMF_WidgetType::MainHUD:
            return TEXT("MainHUD");
        case EMF_WidgetType::MainMenu:
            return TEXT("MainMenu");
        case EMF_WidgetType::PauseMenu:
            return TEXT("PauseMenu");
        case EMF_WidgetType::MainSettings:
            return TEXT("MainSettings");
        case EMF_WidgetType::InputSettings:
            return TEXT("InputSettings");
        case EMF_WidgetType::AudioSettings:
            return TEXT("AudioSettings");
        case EMF_WidgetType::GraphicsSettings:
            return TEXT("GraphicsSettings");
        case EMF_WidgetType::HUD:
            return TEXT("HUD");
        case EMF_WidgetType::MatchInfo:
            return TEXT("MatchInfo");
        case EMF_WidgetType::TeamIndicator:
            return TEXT("TeamIndicator");
        case EMF_WidgetType::GameplayControls:
            return TEXT("GameplayControls");
        case EMF_WidgetType::TeamSelectionPopup:
            return TEXT("TeamSelectionPopup");
        case EMF_WidgetType::TransitionOverlay:
            return TEXT("TransitionOverlay");
        case EMF_WidgetType::ScorePopup:
            return TEXT("ScorePopup");
        case EMF_WidgetType::VirtualJoystick:
            return TEXT("VirtualJoystick");
        case EMF_WidgetType::ActionButton:
            return TEXT("ActionButton");
        case EMF_WidgetType::ToggleActionButton:
            return TEXT("ToggleActionButton");
        case EMF_WidgetType::TeamPanel:
            return TEXT("TeamPanel");
        case EMF_WidgetType::QuickTeamPanel:
            return TEXT("QuickTeamPanel");
        case EMF_WidgetType::SpectatorControls:
            return TEXT("SpectatorControls");
        case EMF_WidgetType::InputActionRow:
            return TEXT("InputActionRow");
        case EMF_WidgetType::CustomByString:
            return TEXT("CustomByString");
        default:
            return TEXT("Unknown");
        }
    }

    inline EMF_WidgetType FromKey(const FString &Key)
    {
        const FString K = Key.TrimStartAndEnd();

        if (K.Equals(TEXT("MainHUD"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::MainHUD;
        if (K.Equals(TEXT("MainMenu"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::MainMenu;
        if (K.Equals(TEXT("PauseMenu"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::PauseMenu;
        if (K.Equals(TEXT("MainSettings"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::MainSettings;
        if (K.Equals(TEXT("InputSettings"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::InputSettings;
        if (K.Equals(TEXT("AudioSettings"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::AudioSettings;
        if (K.Equals(TEXT("GraphicsSettings"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::GraphicsSettings;
        if (K.Equals(TEXT("HUD"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::HUD;
        if (K.Equals(TEXT("MatchInfo"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::MatchInfo;
        if (K.Equals(TEXT("TeamIndicator"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::TeamIndicator;
        if (K.Equals(TEXT("GameplayControls"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::GameplayControls;
        if (K.Equals(TEXT("TeamSelectionPopup"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::TeamSelectionPopup;
        if (K.Equals(TEXT("TransitionOverlay"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::TransitionOverlay;
        if (K.Equals(TEXT("ScorePopup"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::ScorePopup;
        if (K.Equals(TEXT("VirtualJoystick"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::VirtualJoystick;
        if (K.Equals(TEXT("ActionButton"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::ActionButton;
        if (K.Equals(TEXT("ToggleActionButton"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::ToggleActionButton;
        if (K.Equals(TEXT("TeamPanel"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::TeamPanel;
        if (K.Equals(TEXT("QuickTeamPanel"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::QuickTeamPanel;
        if (K.Equals(TEXT("SpectatorControls"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::SpectatorControls;
        if (K.Equals(TEXT("InputActionRow"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::InputActionRow;
        if (K.Equals(TEXT("CustomByString"), ESearchCase::IgnoreCase))
            return EMF_WidgetType::CustomByString;

        return EMF_WidgetType::Unknown;
    }
}
