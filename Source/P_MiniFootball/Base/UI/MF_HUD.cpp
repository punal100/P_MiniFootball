/*
 * @Author: Punal Manalan
 * @Description: MF_HUD - Main HUD container implementation
 * @Date: 10/12/2025
 */

#include "MF_HUD.h"
#include "MF_MatchInfo.h"
#include "MF_TeamIndicator.h"
#include "MF_TransitionOverlay.h"
#include "MF_SpectatorControls.h"
#include "MF_GameplayControls.h"
#include "MF_TeamSelectionPopup.h"
#include "MF_PauseMenu.h"
#include "Components/WidgetSwitcher.h"
#include "Components/CanvasPanel.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MF_PlayerController.h"
#include "Match/MF_GameState.h"

const FString &UMF_HUD::GetWidgetSpec()
{
    static FString Spec;

    if (!Spec.IsEmpty())
    {
        return Spec;
    }

    // We assemble JSON in multiple pieces because MSVC cannot handle extremely large string literals.
    TArray<FString> Parts;
    Parts.Reserve(8);

    // --- PART 1: Metadata / Toolbar / Hierarchy ---
    Parts.Add(TEXT(R"JSON(
{
    "WidgetClass": "UMF_HUD",
    "BlueprintName": "WBP_MF_HUD",
    "ParentClass": "/Script/P_MiniFootball.MF_HUD",
    "Category": "MF|UI|Core",
    "Description": "Main game HUD containing all UI elements",
    "Version": "1.0.0",
    "IsMasterWidget": true,

    "DesignerToolbar": {
        "DesiredSize": {"Width": 1920, "Height": 1080},
        "ZoomLevel": "1:4",
        "ShowGrid": true
    },

    "Hierarchy": {
        "Root": {
            "Type": "CanvasPanel",
            "Name": "RootCanvas",
            "BindingType": "Optional",
            "Children": [
                {
                    "Type": "UserWidget",
                    "Name": "MatchInfo",
                    "BindingType": "Required",
                    "WidgetClass": "/Script/P_MiniFootball.MF_MatchInfo",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0.5, "Y": 0}, "Max": {"X": 0.5, "Y": 0}},
                        "Position": {"X": 0, "Y": 20},
                        "Alignment": {"X": 0.5, "Y": 0},
                        "AutoSize": true
                    }
                },
                {
                    "Type": "UserWidget",
                    "Name": "TeamIndicator",
                    "BindingType": "Required",
                    "WidgetClass": "/Script/P_MiniFootball.MF_TeamIndicator",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0, "Y": 0}, "Max": {"X": 0, "Y": 0}},
                        "Position": {"X": 20, "Y": 20},
                        "Alignment": {"X": 0, "Y": 0},
                        "AutoSize": true
                    }
                },
                {
                    "Type": "WidgetSwitcher",
                    "Name": "ModeSwitcher",
                    "BindingType": "Required",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0, "Y": 0}, "Max": {"X": 1, "Y": 1}},
                        "Offsets": {"Left": 0, "Top": 100, "Right": 0, "Bottom": 0}
                    },
                    "Children": [
                        {
                            "Type": "UserWidget",
                            "Name": "SpectatorControls",
                            "BindingType": "Required",
                            "WidgetClass": "/Script/P_MiniFootball.MF_SpectatorControls",
                            "SwitcherIndex": 0,
                            "Comment": "Index 0: Spectator mode"
                        },
                        {
                            "Type": "UserWidget",
                            "Name": "GameplayControls",
                            "BindingType": "Required",
                            "WidgetClass": "/Script/P_MiniFootball.MF_GameplayControls",
                            "SwitcherIndex": 1,
                            "Comment": "Index 1: Gameplay mode"
                        }
                    ]
                },
                {
                    "Type": "UserWidget",
                    "Name": "TransitionOverlay",
                    "BindingType": "Optional",
                    "WidgetClass": "/Script/P_MiniFootball.MF_TransitionOverlay",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0, "Y": 0}, "Max": {"X": 1, "Y": 1}},
                        "Offsets": {"Left": 0, "Top": 0, "Right": 0, "Bottom": 0},
                        "ZOrder": 100
                    }
                },
                {
                    "Type": "UserWidget",
                    "Name": "TeamSelectionPopup",
                    "BindingType": "Optional",
                    "WidgetClass": "/Script/P_MiniFootball.MF_TeamSelectionPopup",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0, "Y": 0}, "Max": {"X": 1, "Y": 1}},
                        "Offsets": {"Left": 0, "Top": 0, "Right": 0, "Bottom": 0},
                        "ZOrder": 200
                    }
                },
                {
                    "Type": "UserWidget",
                    "Name": "PauseMenu",
                    "BindingType": "Optional",
                    "WidgetClass": "/Script/P_MiniFootball.MF_PauseMenu",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0, "Y": 0}, "Max": {"X": 1, "Y": 1}},
                        "Offsets": {"Left": 0, "Top": 0, "Right": 0, "Bottom": 0},
                        "ZOrder": 300
                    }
                }
            ]
        }
    },
)JSON"));

    // --- PART 2: Design / Bindings / Delegates ---
    Parts.Add(TEXT(R"JSON(
    "Design": {
        "RootCanvas": { "Note": "Full-screen canvas for all HUD elements" },
        "ModeSwitcher": { "ActiveWidgetIndex": 0, "Comment": "0=Spectator, 1=Gameplay" }
    },

    "Bindings": {
        "Required": [
            {"Name": "MatchInfo", "Type": "UMF_MatchInfo", "Purpose": "Score/timer display"},
            {"Name": "TeamIndicator", "Type": "UMF_TeamIndicator", "Purpose": "Current team display"},
            {"Name": "ModeSwitcher", "Type": "UWidgetSwitcher", "Purpose": "Mode toggle container"},
            {"Name": "SpectatorControls", "Type": "UMF_SpectatorControls", "Purpose": "Spectator UI"},
            {"Name": "GameplayControls", "Type": "UMF_GameplayControls", "Purpose": "Gameplay UI"}
        ],
        "Optional": [
            {"Name": "TransitionOverlay", "Type": "UMF_TransitionOverlay", "Purpose": "Loading screen"},
            {"Name": "TeamSelectionPopup", "Type": "UMF_TeamSelectionPopup", "Purpose": "Team picker modal"},
            {"Name": "PauseMenu", "Type": "UMF_PauseMenu", "Purpose": "Pause menu"},
            {"Name": "RootCanvas", "Type": "UCanvasPanel", "Purpose": "Root container"}
        ]
    },

    "Delegates": [
        {"Name": "OnHUDShown", "Description": "Triggered when HUD becomes visible"},
        {"Name": "OnHUDHidden", "Description": "Triggered when HUD hides"}
    ],
)JSON"));

    // --- PART 3: Dependencies / BuildOrder / Comments / Python ---
    Parts.Add(TEXT(R"JSON(
    "Dependencies": [
        {"Class": "UMF_MatchInfo", "Blueprint": "WBP_MF_MatchInfo", "Required": true, "Order": 1},
        {"Class": "UMF_TeamIndicator", "Blueprint": "WBP_MF_TeamIndicator", "Required": true, "Order": 2},
        {"Class": "UMF_TransitionOverlay", "Blueprint": "WBP_MF_TransitionOverlay", "Required": false, "Order": 3},
        {"Class": "UMF_VirtualJoystick", "Blueprint": "WBP_MF_VirtualJoystick", "Required": true, "Order": 4},
        {"Class": "UMF_ActionButton", "Blueprint": "WBP_MF_ActionButton", "Required": true, "Order": 5},
        {"Class": "UMF_SprintButton", "Blueprint": "WBP_MF_SprintButton", "Required": false, "Order": 6},
        {"Class": "UMF_QuickTeamPanel", "Blueprint": "WBP_MF_QuickTeamPanel", "Required": false, "Order": 7},
        {"Class": "UMF_TeamPanel", "Blueprint": "WBP_MF_TeamPanel", "Required": false, "Order": 8},
        {"Class": "UMF_SpectatorControls", "Blueprint": "WBP_MF_SpectatorControls", "Required": true, "Order": 9},
        {"Class": "UMF_GameplayControls", "Blueprint": "WBP_MF_GameplayControls", "Required": true, "Order": 10},
        {"Class": "UMF_TeamSelectionPopup", "Blueprint": "WBP_MF_TeamSelectionPopup", "Required": false, "Order": 11},
        {"Class": "UMF_PauseMenu", "Blueprint": "WBP_MF_PauseMenu", "Required": false, "Order": 12}
    ],

    "BuildOrder": [
        "WBP_MF_ActionButton",
        "WBP_MF_VirtualJoystick",
        "WBP_MF_SprintButton",
        "WBP_MF_MatchInfo",
        "WBP_MF_TeamIndicator",
        "WBP_MF_TransitionOverlay",
        "WBP_MF_QuickTeamPanel",
        "WBP_MF_TeamPanel",
        "WBP_MF_SpectatorControls",
        "WBP_MF_GameplayControls",
        "WBP_MF_TeamSelectionPopup",
        "WBP_MF_PauseMenu",
        "WBP_MF_HUD"
    ],

    "Comments": {
        "Header": "MF HUD - Master game HUD containing all UI elements",
        "Usage": "Created and added to viewport by MF_PlayerController",
        "CreationOrder": "Create all dependencies first, then MF_HUD last"
    },

    "PythonSnippets": {
        "CreateRoot": "root = creator.add_widget('CanvasPanel', 'RootCanvas', None)",
        "CreateMatchInfo": "match_info = creator.add_widget('UserWidget', 'MatchInfo', root, widget_class='WBP_MF_MatchInfo', slot_data={'anchors': 'top_center'})",
        "CreateTeamIndicator": "team_ind = creator.add_widget('UserWidget', 'TeamIndicator', root, widget_class='WBP_MF_TeamIndicator', slot_data={'anchors': 'top_left'})",
        "CreateSwitcher": "switcher = creator.add_widget('WidgetSwitcher', 'ModeSwitcher', root)",
        "CreateSpectator": "spec = creator.add_widget('UserWidget', 'SpectatorControls', switcher, widget_class='WBP_MF_SpectatorControls')",
        "CreateGameplay": "gameplay = creator.add_widget('UserWidget', 'GameplayControls', switcher, widget_class='WBP_MF_GameplayControls')",
        "CreateOverlays": "creator.add_widget('UserWidget', 'TransitionOverlay', root, widget_class='WBP_MF_TransitionOverlay', slot_data={'anchors': 'fill', 'zorder': 100})"
    }

}
)JSON"));

    // Combine all parts safely
    Spec = FString::Join(Parts, TEXT(""));

    return Spec;
}

void UMF_HUD::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind spectator controls events
    if (SpectatorControls)
    {
        SpectatorControls->OnOpenTeamSelection.AddDynamic(this, &UMF_HUD::HandleOpenTeamSelection);
    }

    // Bind team selection popup events
    if (TeamSelectionPopup)
    {
        TeamSelectionPopup->OnPopupClosed.AddDynamic(this, &UMF_HUD::HandleTeamSelectionClosed);
    }

    // Bind pause menu events
    if (PauseMenu)
    {
        PauseMenu->OnResumeClicked.AddDynamic(this, &UMF_HUD::HandlePauseMenuClosed);
    }

    // Initial state refresh
    RefreshFromPlayerState();

    // Set initial mode
    SetHUDMode(DetermineAppropriateMode());
}

void UMF_HUD::NativeDestruct()
{
    // Unbind all delegates
    if (SpectatorControls)
    {
        SpectatorControls->OnOpenTeamSelection.RemoveDynamic(this, &UMF_HUD::HandleOpenTeamSelection);
    }

    if (TeamSelectionPopup)
    {
        TeamSelectionPopup->OnPopupClosed.RemoveDynamic(this, &UMF_HUD::HandleTeamSelectionClosed);
    }

    if (PauseMenu)
    {
        PauseMenu->OnResumeClicked.RemoveDynamic(this, &UMF_HUD::HandlePauseMenuClosed);
    }

    CachedPlayerController = nullptr;

    Super::NativeDestruct();
}

void UMF_HUD::NativeTick(const FGeometry &MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Periodic refresh
    RefreshTimer += InDeltaTime;
    if (RefreshTimer >= RefreshInterval)
    {
        RefreshTimer = 0.0f;

        // Check if mode should change
        EMF_HUDMode AppropriateMode = DetermineAppropriateMode();
        if (AppropriateMode != CurrentMode && CurrentMode != EMF_HUDMode::Menu)
        {
            SetHUDMode(AppropriateMode);
        }

        // Refresh match info
        if (MatchInfo)
        {
            MatchInfo->RefreshMatchInfo();
        }
    }
}

void UMF_HUD::SetHUDMode(EMF_HUDMode NewMode)
{
    if (NewMode == CurrentMode)
    {
        return;
    }

    PreviousMode = CurrentMode;
    CurrentMode = NewMode;

    // Update widget visibility
    UpdateWidgetSwitcher();

    // Update team indicator
    if (TeamIndicator)
    {
        TeamIndicator->RefreshFromController();
    }
}

void UMF_HUD::ShowTeamSelectionPopup()
{
    if (TeamSelectionPopup)
    {
        PreviousMode = CurrentMode;
        CurrentMode = EMF_HUDMode::Menu;
        TeamSelectionPopup->ShowPopup();
    }
}

void UMF_HUD::HideTeamSelectionPopup()
{
    if (TeamSelectionPopup)
    {
        TeamSelectionPopup->HidePopup();
        // Mode will be restored in HandleTeamSelectionClosed
    }
}

void UMF_HUD::ShowPauseMenu()
{
    if (PauseMenu)
    {
        PreviousMode = CurrentMode;
        CurrentMode = EMF_HUDMode::Menu;
        PauseMenu->ShowMenu();
    }
}

void UMF_HUD::HidePauseMenu()
{
    if (PauseMenu)
    {
        PauseMenu->HideMenu();
        // Mode will be restored in HandlePauseMenuClosed
    }
}

void UMF_HUD::TogglePauseMenu()
{
    if (PauseMenu)
    {
        if (PauseMenu->IsMenuVisible())
        {
            HidePauseMenu();
        }
        else
        {
            ShowPauseMenu();
        }
    }
}

void UMF_HUD::ShowTransitionOverlay(const FString &Message)
{
    if (TransitionOverlay)
    {
        TransitionOverlay->SetStatusText(Message);
        TransitionOverlay->ShowOverlay();
    }

    PreviousMode = CurrentMode;
    CurrentMode = EMF_HUDMode::Transition;
}

void UMF_HUD::HideTransitionOverlay()
{
    if (TransitionOverlay)
    {
        TransitionOverlay->HideOverlay();
    }

    // Restore previous mode
    CurrentMode = PreviousMode;
    UpdateWidgetSwitcher();
}

void UMF_HUD::RefreshFromPlayerState()
{
    // Update team indicator
    if (TeamIndicator)
    {
        TeamIndicator->RefreshFromController();
    }

    // Update match info
    if (MatchInfo)
    {
        MatchInfo->RefreshMatchInfo();
    }

    // Update spectator controls if visible
    if (SpectatorControls && CurrentMode == EMF_HUDMode::Spectator)
    {
        SpectatorControls->RefreshTeamData();
    }

    // Update gameplay controls if visible
    if (GameplayControls && CurrentMode == EMF_HUDMode::Gameplay)
    {
        GameplayControls->RefreshControlLayout();
    }
}

void UMF_HUD::HandleOpenTeamSelection()
{
    ShowTeamSelectionPopup();
}

void UMF_HUD::HandleTeamSelectionClosed()
{
    // Determine new mode based on player state (may have joined a team)
    EMF_HUDMode NewMode = DetermineAppropriateMode();
    SetHUDMode(NewMode);
}

void UMF_HUD::HandlePauseMenuClosed()
{
    // Restore previous mode
    SetHUDMode(PreviousMode);
}

void UMF_HUD::UpdateWidgetSwitcher()
{
    if (!ModeSwitcher)
    {
        return;
    }

    switch (CurrentMode)
    {
    case EMF_HUDMode::Spectator:
        // Show spectator controls (index 0)
        ModeSwitcher->SetActiveWidgetIndex(0);
        break;

    case EMF_HUDMode::Gameplay:
        // Show gameplay controls (index 1)
        ModeSwitcher->SetActiveWidgetIndex(1);
        break;

    case EMF_HUDMode::Menu:
    case EMF_HUDMode::Transition:
        // Keep current widget visible, popups are overlays
        break;
    }

    // Update controls state
    if (SpectatorControls)
    {
        SpectatorControls->SetVisibility(CurrentMode == EMF_HUDMode::Spectator ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    if (GameplayControls)
    {
        bool bShowGameplay = (CurrentMode == EMF_HUDMode::Gameplay);
        GameplayControls->SetVisibility(bShowGameplay ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

        if (bShowGameplay)
        {
            GameplayControls->SetControlsEnabled(true);
        }
    }
}

AMF_PlayerController *UMF_HUD::GetMFPlayerController()
{
    if (!CachedPlayerController)
    {
        APlayerController *PC = GetOwningPlayer();
        CachedPlayerController = Cast<AMF_PlayerController>(PC);
    }

    return CachedPlayerController;
}

AMF_GameState *UMF_HUD::GetGameState() const
{
    UWorld *World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    return Cast<AMF_GameState>(UGameplayStatics::GetGameState(World));
}

EMF_HUDMode UMF_HUD::DetermineAppropriateMode() const
{
    AMF_PlayerController *PC = const_cast<UMF_HUD *>(this)->GetMFPlayerController();
    if (!PC)
    {
        return EMF_HUDMode::Spectator;
    }

    // Check if player is on a team
    EMF_TeamID CurrentTeam = PC->GetCurrentTeam();

    if (CurrentTeam == EMF_TeamID::None)
    {
        return EMF_HUDMode::Spectator;
    }
    else
    {
        return EMF_HUDMode::Gameplay;
    }
}
