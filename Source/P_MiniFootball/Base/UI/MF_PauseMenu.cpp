/*
 * @Author: Punal Manalan
 * @Description: MF_PauseMenu - In-game pause menu implementation
 * @Date: 10/12/2025
 */

#include "MF_PauseMenu.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MF_PlayerController.h"
#include "Match/MF_GameState.h"
#include "MF_SettingsMenu.h"

FString UMF_PauseMenu::GetWidgetSpec()
{
    static FString Spec = R"JSON({
    "WidgetClass": "UMF_PauseMenu",
    "BlueprintName": "WBP_MF_PauseMenu",
    "ParentClass": "/Script/P_MiniFootball.MF_PauseMenu",
    "Category": "MF|UI|Menus",
    "Description": "In-game pause menu with options",
    "Version": "1.0.0",

    "DesignerPreview": {
        "SizeMode": "FillScreen",
        "ZoomLevel": 14,
        "ShowGrid": true
    },
    
    "Hierarchy": {
        "Root": {
            "Type": "CanvasPanel",
            "Name": "RootCanvas",
            "Children": [
                {
                    "Type": "Overlay",
                    "Name": "BackgroundOverlay",
                    "BindingType": "Optional",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0, "Y": 0}, "Max": {"X": 1, "Y": 1}},
                        "Offsets": {"Left": 0, "Top": 0, "Right": 0, "Bottom": 0}
                    }
                },
                {
                    "Type": "VerticalBox",
                    "Name": "MenuContainer",
                    "BindingType": "Optional",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0.5, "Y": 0.5}, "Max": {"X": 0.5, "Y": 0.5}},
                        "Alignment": {"X": 0.5, "Y": 0.5}
                    },
                    "Children": [
                        {
                            "Type": "TextBlock",
                            "Name": "TitleText",
                            "BindingType": "Optional",
                            "Text": "PAUSED",
                            "FontSize": 32,
                            "Justification": "Center",
                            "Slot": {"HAlign": "Center", "Padding": {"Bottom": 20}}
                        },
                        {
                            "Type": "TextBlock",
                            "Name": "CurrentTeamText",
                            "BindingType": "Optional",
                            "Text": "Team: None",
                            "FontSize": 18,
                            "Justification": "Center",
                            "Slot": {"HAlign": "Center", "Padding": {"Bottom": 30}}
                        },
                        {
                            "Type": "Button",
                            "Name": "ResumeButton",
                            "BindingType": "Required",
                            "Slot": {"HAlign": "Center", "Padding": {"Bottom": 10}},
                            "Children": [
                                {
                                    "Type": "TextBlock",
                                    "Name": "ResumeButtonLabel",
                                    "Text": "RESUME",
                                    "FontSize": 18,
                                    "Justification": "Center",
                                    "Slot": {"HAlign": "Center", "VAlign": "Center"}
                                }
                            ]
                        },
                        {
                            "Type": "Button",
                            "Name": "ChangeTeamButton",
                            "BindingType": "Optional",
                            "Slot": {"HAlign": "Center", "Padding": {"Bottom": 10}},
                            "Children": [
                                {
                                    "Type": "TextBlock",
                                    "Name": "ChangeTeamButtonLabel",
                                    "Text": "CHANGE TEAM",
                                    "FontSize": 18,
                                    "Justification": "Center",
                                    "Slot": {"HAlign": "Center", "VAlign": "Center"}
                                }
                            ]
                        },
                        {
                            "Type": "Button",
                            "Name": "LeaveTeamButton",
                            "BindingType": "Required",
                            "Slot": {"HAlign": "Center", "Padding": {"Bottom": 10}},
                            "Children": [
                                {
                                    "Type": "TextBlock",
                                    "Name": "LeaveTeamButtonLabel",
                                    "Text": "LEAVE TEAM",
                                    "FontSize": 18,
                                    "Justification": "Center",
                                    "Slot": {"HAlign": "Center", "VAlign": "Center"}
                                }
                            ]
                        },
                        {
                            "Type": "Button",
                            "Name": "SettingsButton",
                            "BindingType": "Optional",
                            "Slot": {"HAlign": "Center", "Padding": {"Bottom": 10}},
                            "Children": [
                                {
                                    "Type": "TextBlock",
                                    "Name": "SettingsButtonLabel",
                                    "Text": "SETTINGS",
                                    "FontSize": 18,
                                    "Justification": "Center",
                                    "Slot": {"HAlign": "Center", "VAlign": "Center"}
                                }
                            ]
                        },
                        {
                            "Type": "Button",
                            "Name": "QuitButton",
                            "BindingType": "Required",
                            "Slot": {"HAlign": "Center", "Padding": {"Top": 20}},
                            "Children": [
                                {
                                    "Type": "TextBlock",
                                    "Name": "QuitButtonLabel",
                                    "Text": "QUIT",
                                    "FontSize": 18,
                                    "Justification": "Center",
                                    "Slot": {"HAlign": "Center", "VAlign": "Center"}
                                }
                            ]
                        }
                    ]
                }
            ]
        }
    },
    
    "Design": {
        "BackgroundOverlay": {
            "Note": "Semi-transparent overlay behind menu"
        },
        "TitleText": {
            "Font": {"Size": 32, "Typeface": "Bold"},
            "Text": "PAUSED"
        },
        "CurrentTeamText": {
            "Font": {"Size": 18, "Typeface": "Regular"},
            "Text": "Team: None"
        },
        "ResumeButton": {
            "Style": {"Normal": {"TintColor": {"R": 0.2, "G": 0.5, "B": 0.2, "A": 1.0}}},
            "Size": {"X": 200, "Y": 50}
        },
        "LeaveTeamButton": {
            "Style": {"Normal": {"TintColor": {"R": 0.6, "G": 0.4, "B": 0.1, "A": 1.0}}},
            "Size": {"X": 200, "Y": 50}
        },
        "QuitButton": {
            "Style": {"Normal": {"TintColor": {"R": 0.6, "G": 0.2, "B": 0.2, "A": 1.0}}},
            "Size": {"X": 200, "Y": 50}
        }
    },
    
    "Bindings": {
        "Required": [
            {"Name": "ResumeButton", "Type": "UButton", "Purpose": "Resume game"},
            {"Name": "LeaveTeamButton", "Type": "UButton", "Purpose": "Leave current team"},
            {"Name": "QuitButton", "Type": "UButton", "Purpose": "Quit to menu"}
        ],
        "Optional": [
            {"Name": "TitleText", "Type": "UTextBlock", "Purpose": "Menu title"},
            {"Name": "CurrentTeamText", "Type": "UTextBlock", "Purpose": "Current team display"},
            {"Name": "ChangeTeamButton", "Type": "UButton", "Purpose": "Change team option"},
            {"Name": "SettingsButton", "Type": "UButton", "Purpose": "Settings access"},
            {"Name": "MenuContainer", "Type": "UVerticalBox", "Purpose": "Menu items container"},
            {"Name": "BackgroundOverlay", "Type": "UOverlay", "Purpose": "Background dimmer"}
        ]
    },
    
    "Delegates": [
        {
            "Name": "OnResumeClicked",
            "Type": "FMF_OnResumeClicked",
            "Signature": "void()",
            "Description": "Resume game requested"
        },
        {
            "Name": "OnLeaveTeamClicked",
            "Type": "FMF_OnLeaveTeamClicked",
            "Signature": "void()",
            "Description": "Leave team requested"
        },
        {
            "Name": "OnQuitToMenuClicked",
            "Type": "FMF_OnQuitToMenuClicked",
            "Signature": "void()",
            "Description": "Quit game requested"
        }
    ],
    
    "Dependencies": [],
    
    "Comments": {
        "Header": "MF Pause Menu - In-game pause/options menu",
        "Usage": "Shown when ESC pressed during gameplay"
    },
    
    "PythonSnippets": {
        "CreateRoot": "root = creator.add_widget('CanvasPanel', 'RootCanvas', None)",
        "CreateOverlay": "overlay = creator.add_widget('Overlay', 'BackgroundOverlay', root)",
        "CreateMenu": "menu = creator.add_widget('VerticalBox', 'MenuContainer', root)",
        "CreateButtons": "creator.add_widget('Button', 'ResumeButton', menu); creator.add_widget('Button', 'QuitButton', menu)"
    }
})JSON";
    return Spec;
}

void UMF_PauseMenu::NativeConstruct()
{
    Super::NativeConstruct();

    // Set title
    if (TitleText)
    {
        TitleText->SetText(FText::FromString(TEXT("⏸ PAUSED")));
    }

    // Bind button events
    if (ResumeButton)
    {
        ResumeButton->OnClicked.AddDynamic(this, &UMF_PauseMenu::HandleResumeClicked);
    }

    if (LeaveTeamButton)
    {
        LeaveTeamButton->OnClicked.AddDynamic(this, &UMF_PauseMenu::HandleLeaveTeamClicked);
    }

    if (ChangeTeamButton)
    {
        ChangeTeamButton->OnClicked.AddDynamic(this, &UMF_PauseMenu::HandleChangeTeamClicked);
    }

    if (SettingsButton)
    {
        SettingsButton->OnClicked.AddDynamic(this, &UMF_PauseMenu::HandleSettingsClicked);
    }

    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UMF_PauseMenu::HandleQuitClicked);
    }

    // Start hidden
    SetVisibility(ESlateVisibility::Collapsed);
    bIsVisible = false;
}

void UMF_PauseMenu::NativeDestruct()
{
    // Unbind all button events
    if (ResumeButton)
    {
        ResumeButton->OnClicked.RemoveDynamic(this, &UMF_PauseMenu::HandleResumeClicked);
    }

    if (LeaveTeamButton)
    {
        LeaveTeamButton->OnClicked.RemoveDynamic(this, &UMF_PauseMenu::HandleLeaveTeamClicked);
    }

    if (ChangeTeamButton)
    {
        ChangeTeamButton->OnClicked.RemoveDynamic(this, &UMF_PauseMenu::HandleChangeTeamClicked);
    }

    if (SettingsButton)
    {
        SettingsButton->OnClicked.RemoveDynamic(this, &UMF_PauseMenu::HandleSettingsClicked);
    }

    if (QuitButton)
    {
        QuitButton->OnClicked.RemoveDynamic(this, &UMF_PauseMenu::HandleQuitClicked);
    }

    Super::NativeDestruct();
}

void UMF_PauseMenu::ShowMenu()
{
    if (bIsVisible)
    {
        return;
    }

    // Refresh state before showing
    RefreshMenuState();

    // Show the widget
    SetVisibility(ESlateVisibility::Visible);
    bIsVisible = true;

    // Set input mode to UI + Game
    APlayerController *PC = GetOwningPlayer();
    if (PC)
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        InputMode.SetHideCursorDuringCapture(false);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;

        // Optionally pause the game (for single player)
        // UGameplayStatics::SetGamePaused(GetWorld(), true);
    }
}

void UMF_PauseMenu::HideMenu()
{
    if (!bIsVisible)
    {
        return;
    }

    // Hide the widget
    SetVisibility(ESlateVisibility::Collapsed);
    bIsVisible = false;

    // Restore input mode
    APlayerController *PC = GetOwningPlayer();
    if (PC)
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;

        // Unpause if we paused
        // UGameplayStatics::SetGamePaused(GetWorld(), false);
    }
}

void UMF_PauseMenu::ToggleMenu()
{
    if (bIsVisible)
    {
        HideMenu();
    }
    else
    {
        ShowMenu();
    }
}

void UMF_PauseMenu::RefreshMenuState()
{
    UpdateCurrentTeamDisplay();
    UpdateLeaveTeamButtonVisibility();
}

void UMF_PauseMenu::HandleResumeClicked()
{
    HideMenu();
    OnResumeClicked.Broadcast();
}

void UMF_PauseMenu::HandleLeaveTeamClicked()
{
    AMF_PlayerController *PC = GetMFPlayerController();
    if (PC)
    {
        PC->Server_RequestLeaveTeam();
    }

    OnLeaveTeamClicked.Broadcast();
    HideMenu();
}

void UMF_PauseMenu::HandleChangeTeamClicked()
{
    // This would typically open the team selection popup
    // For now, we just close this menu
    HideMenu();

    // The HUD should handle opening the team selection popup
    // after receiving this signal
}

void UMF_PauseMenu::HandleSettingsClicked()
{
    APlayerController *PC = GetOwningPlayer();
    if (!PC || !PC->IsLocalController())
    {
        return;
    }

    if (!SettingsMenu)
    {
        TSubclassOf<UMF_SettingsMenu> ClassToCreate = SettingsMenuClass;
        if (!ClassToCreate)
        {
            // Prefer the MWCS-generated blueprint if available.
            static const TCHAR *DefaultSettingsMenuClassPath =
                TEXT("/P_MiniFootball/BP/WBP/Sub/WBP_MF_SettingsMenu.WBP_MF_SettingsMenu_C");
            ClassToCreate = LoadClass<UMF_SettingsMenu>(nullptr, DefaultSettingsMenuClassPath);
        }

        if (!ClassToCreate)
        {
            ClassToCreate = UMF_SettingsMenu::StaticClass();
        }

        SettingsMenu = CreateWidget<UMF_SettingsMenu>(PC, ClassToCreate);
        if (SettingsMenu)
        {
            SettingsMenu->AddToViewport(1000); // above pause menu
            SettingsMenu->OnSettingsClosed.AddDynamic(this, &UMF_PauseMenu::HandleSettingsClosed);
        }
    }

    if (SettingsMenu)
    {
        SettingsMenu->SetVisibility(ESlateVisibility::Visible);

        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(SettingsMenu->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        InputMode.SetHideCursorDuringCapture(false);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }
}

void UMF_PauseMenu::HandleSettingsClosed()
{
    // Settings closed; keep pause menu open, but restore focus.
    APlayerController *PC = GetOwningPlayer();
    if (PC)
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        InputMode.SetHideCursorDuringCapture(false);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }
}

void UMF_PauseMenu::HandleQuitClicked()
{
    OnQuitToMenuClicked.Broadcast();

    // Leave team first if on one
    AMF_PlayerController *PC = GetMFPlayerController();
    if (PC)
    {
        EMF_TeamID CurrentTeam = GetCurrentTeam();
        if (CurrentTeam != EMF_TeamID::None)
        {
            PC->Server_RequestLeaveTeam();
        }
    }

    // Quit to main menu
    UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("MainMenu")));
}

void UMF_PauseMenu::UpdateLeaveTeamButtonVisibility()
{
    if (!LeaveTeamButton)
    {
        return;
    }

    EMF_TeamID CurrentTeam = GetCurrentTeam();
    bool bOnTeam = (CurrentTeam != EMF_TeamID::None);

    LeaveTeamButton->SetVisibility(bOnTeam ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

    // Also update change team button
    if (ChangeTeamButton)
    {
        ChangeTeamButton->SetVisibility(bOnTeam ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UMF_PauseMenu::UpdateCurrentTeamDisplay()
{
    if (!CurrentTeamText)
    {
        return;
    }

    EMF_TeamID CurrentTeam = GetCurrentTeam();
    FString TeamStr;

    switch (CurrentTeam)
    {
    case EMF_TeamID::TeamA:
        TeamStr = TEXT("Current Team: Team A");
        break;
    case EMF_TeamID::TeamB:
        TeamStr = TEXT("Current Team: Team B");
        break;
    case EMF_TeamID::None:
    default:
        TeamStr = TEXT("Spectating");
        break;
    }

    CurrentTeamText->SetText(FText::FromString(TeamStr));
}

AMF_PlayerController *UMF_PauseMenu::GetMFPlayerController() const
{
    APlayerController *PC = GetOwningPlayer();
    return Cast<AMF_PlayerController>(PC);
}

AMF_GameState *UMF_PauseMenu::GetGameState() const
{
    UWorld *World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    return Cast<AMF_GameState>(UGameplayStatics::GetGameState(World));
}

EMF_TeamID UMF_PauseMenu::GetCurrentTeam() const
{
    AMF_PlayerController *PC = GetMFPlayerController();
    if (PC)
    {
        return PC->GetCurrentTeam();
    }

    return EMF_TeamID::None;
}
