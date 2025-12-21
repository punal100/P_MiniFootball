/*
 * @Author: Punal Manalan
 * @Description: MF_TeamSelectionPopup - Modal team selection dialog implementation
 * @Date: 10/12/2025
 */

#include "MF_TeamSelectionPopup.h"
#include "MF_TeamPanel.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MF_PlayerController.h"
#include "Match/MF_GameState.h"

FString UMF_TeamSelectionPopup::GetWidgetSpec()
{
    static FString Spec = R"JSON({
    "WidgetClass": "UMF_TeamSelectionPopup",
    "BlueprintName": "WBP_MF_TeamSelectionPopup",
    "ParentClass": "/Script/P_MiniFootball.MF_TeamSelectionPopup",
    "Category": "MF|UI|Popups",
    "Description": "Full team selection popup with detailed team panels",
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
                    "Type": "Image",
                    "Name": "BackgroundOverlay",
                    "BindingType": "Optional",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0, "Y": 0}, "Max": {"X": 1, "Y": 1}},
                        "Offsets": {"Left": 0, "Top": 0, "Right": 0, "Bottom": 0}
                    }
                },
                {
                    "Type": "Border",
                    "Name": "PopupContainer",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0.5, "Y": 0.5}, "Max": {"X": 0.5, "Y": 0.5}},
                        "Alignment": {"X": 0.5, "Y": 0.5},
                        "Size": {"X": 100, "Y": 30},
                        "AutoSize": true
                    },
                    "Children": [
                        {
                            "Type": "VerticalBox",
                            "Name": "PopupContent",
                            "Slot": {
                                "Padding": {"Left": 20, "Top": 15, "Right": 20, "Bottom": 15},
                                "HAlign": "Fill",
                                "VAlign": "Fill"
                            },
                            "Children": [
                                {
                                    "Type": "HorizontalBox",
                                    "Name": "HeaderRow",
                                    "Slot": {"HAlign": "Fill", "VAlign": "Fill", "Size": {"Rule": "Auto"}},
                                    "Children": [
                                        {"Type": "TextBlock", "Name": "TitleText", "BindingType": "Optional", "Text": "SELECT TEAM", "Slot": {"HAlign": "Fill", "Size": {"Rule": "Fill", "Value": 1}, "VAlign": "Center"}},
                                        {"Type": "Button", "Name": "CloseButton", "BindingType": "Required", "Slot": {"HAlign": "Right", "VAlign": "Center", "Size": {"Rule": "Auto"}}, "Children": [
                                            {"Type": "TextBlock", "Name": "CloseButtonLabel", "Text": "X", "Slot": {"Padding": {"Left": 4, "Top": 2, "Right": 4, "Bottom": 2}, "HAlign": "Center", "VAlign": "Center"}}
                                        ]}
                                    ]
                                },
                                {
                                    "Type": "HorizontalBox",
                                    "Name": "TeamPanelsRow",
                                    "Properties": {"Spacing": 10},
                                    "Slot": {"HAlign": "Fill", "VAlign": "Fill", "Size": {"Rule": "Fill", "Value": 1}},
                                    "Children": [
                                        {
                                            "Type": "UserWidget",
                                            "Name": "TeamAPanel",
                                            "BindingType": "Required",
                                            "WidgetClass": "/Script/P_MiniFootball.MF_TeamPanel",
                                            "Slot": {"HAlign": "Fill", "VAlign": "Fill", "Size": {"Rule": "Fill", "Value": 1}, "Padding": {"Right": 10}}
                                        },
                                        {
                                            "Type": "UserWidget",
                                            "Name": "TeamBPanel",
                                            "BindingType": "Required",
                                            "WidgetClass": "/Script/P_MiniFootball.MF_TeamPanel",
                                            "Slot": {"HAlign": "Fill", "VAlign": "Fill", "Size": {"Rule": "Fill", "Value": 1}, "Padding": {"Left": 10}}
                                        }
                                    ]
                                },
                                {
                                    "Type": "HorizontalBox",
                                    "Name": "FooterRow",
                                    "Slot": {"HAlign": "Fill", "VAlign": "Fill", "Size": {"Rule": "Auto"}},
                                    "Children": [
                                        {"Type": "Button", "Name": "AutoAssignButton", "BindingType": "Optional", "Slot": {"Size": {"Rule": "Auto"}}, "Children": [
                                            {"Type": "TextBlock", "Name": "AutoAssignButtonLabel", "Text": "AUTO ASSIGN", "Slot": {"Padding": {"Left": 4, "Top": 2, "Right": 4, "Bottom": 2}, "HAlign": "Center", "VAlign": "Center"}}
                                        ]},
                                        {"Type": "TextBlock", "Name": "StatusText", "BindingType": "Optional", "Slot": {"Size": {"Rule": "Fill", "Value": 1}, "VAlign": "Center"}}
                                    ]
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
            "Size": {"X": 32, "Y": 32},
            "ColorAndOpacity": {"R": 0, "G": 0, "B": 0, "A": 0.7}
        },
        "PopupContainer": {
            "BrushColor": {"R": 0.1, "G": 0.1, "B": 0.15, "A": 0.95},
            "Padding": {"Left": 20, "Top": 15, "Right": 20, "Bottom": 15}
        },
        "TitleText": {
            "Font": {"Size": 28, "Typeface": "Bold"},
            "ColorAndOpacity": {"R": 1.0, "G": 1.0, "B": 1.0, "A": 1.0},
            "Text": "SELECT TEAM"
        },
        "CloseButton": {
            "Style": {
                "Normal": {"TintColor": {"R": 0.6, "G": 0.2, "B": 0.2, "A": 1.0}},
                "Hovered": {"TintColor": {"R": 0.72, "G": 0.72, "B": 0.72, "A": 1.0}},
                "Pressed": {"TintColor": {"R": 0.38, "G": 0.38, "B": 0.38, "A": 1.0}}
            },
            "IsFocusable": true
        },
        "CloseButtonLabel": {
            "Font": {"Size": 18, "Typeface": "Bold"},
            "ColorAndOpacity": {"R": 1.0, "G": 1.0, "B": 1.0, "A": 1.0}
        },
        "AutoAssignButton": {
            "Style": {
                "Normal": {"TintColor": {"R": 0.3, "G": 0.5, "B": 0.3, "A": 1.0}},
                "Hovered": {"TintColor": {"R": 0.72, "G": 0.72, "B": 0.72, "A": 1.0}},
                "Pressed": {"TintColor": {"R": 0.38, "G": 0.38, "B": 0.38, "A": 1.0}}
            },
            "IsFocusable": true
        },
        "AutoAssignButtonLabel": {
            "Font": {"Size": 14, "Typeface": "Bold"},
            "Text": "AUTO ASSIGN",
            "ColorAndOpacity": {"R": 1.0, "G": 1.0, "B": 1.0, "A": 1.0}
        },
        "StatusText": {
            "Font": {"Size": 14, "Typeface": "Regular"},
            "ColorAndOpacity": {"R": 1.0, "G": 1.0, "B": 1.0, "A": 1.0}
        }
    },
    
    "Bindings": {
        "Required": [
            {"Name": "TeamAPanel", "Type": "UMF_TeamPanel", "Purpose": "Team A selection panel"},
            {"Name": "TeamBPanel", "Type": "UMF_TeamPanel", "Purpose": "Team B selection panel"},
            {"Name": "CloseButton", "Type": "UButton", "Purpose": "Close popup button"}
        ],
        "Optional": [
            {"Name": "TitleText", "Type": "UTextBlock", "Purpose": "Popup title"},
            {"Name": "AutoAssignButton", "Type": "UButton", "Purpose": "Auto-balance assign"},
            {"Name": "BackgroundOverlay", "Type": "UImage", "Purpose": "Modal backdrop"},
            {"Name": "StatusText", "Type": "UTextBlock", "Purpose": "Status messages"}
        ]
    },
    
    "Delegates": [
        {
            "Name": "OnPopupClosed",
            "Type": "FMF_OnPopupClosed",
            "Signature": "void()",
            "Description": "Fired when popup is closed"
        },
        {
            "Name": "OnTeamSelected",
            "Type": "FMF_OnTeamSelected",
            "Signature": "void(EMF_TeamID TeamID)",
            "Description": "Fired when team is selected"
        }
    ],
    
    "Dependencies": [
        {"Class": "UMF_TeamPanel", "Blueprint": "WBP_MF_TeamPanel", "Required": true},
        "/Engine/EngineFonts/Roboto.Roboto"
    ],
    
    "Comments": {
        "Header": "MF Team Selection Popup - Full-screen team picker",
        "Usage": "Modal popup shown from SpectatorControls or PauseMenu"
    },
    
    "PythonSnippets": {
        "CreateRoot": "root = creator.add_widget('CanvasPanel', 'RootCanvas', None)",
        "CreateBG": "bg = creator.add_widget('Image', 'BackgroundOverlay', root, slot_data={'anchors': 'fill'})",
        "CreatePopup": "container = creator.add_widget('Border', 'PopupContainer', root)",
        "CreateTeamPanels": "creator.add_widget('UserWidget', 'TeamAPanel', panels_row, widget_class='WBP_MF_TeamPanel')"
    }
})JSON";
    return Spec;
}

void UMF_TeamSelectionPopup::NativeConstruct()
{
    Super::NativeConstruct();

    UE_LOG(LogTemp, Warning, TEXT("=== MF_TeamSelectionPopup::NativeConstruct ==="));

    // Set title
    if (TitleText)
    {
        TitleText->SetText(FText::FromString(TEXT("SELECT YOUR TEAM")));
    }

    // Initialize team panels
    if (TeamAPanel)
    {
        TeamAPanel->SetTeamID(EMF_TeamID::TeamA);
        TeamAPanel->OnJoinClicked.AddDynamic(this, &UMF_TeamSelectionPopup::HandleTeamAJoinClicked);
        UE_LOG(LogTemp, Warning, TEXT("  TeamAPanel: BOUND"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("  TeamAPanel: NULL"));
    }

    if (TeamBPanel)
    {
        TeamBPanel->SetTeamID(EMF_TeamID::TeamB);
        TeamBPanel->OnJoinClicked.AddDynamic(this, &UMF_TeamSelectionPopup::HandleTeamBJoinClicked);
        UE_LOG(LogTemp, Warning, TEXT("  TeamBPanel: BOUND"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("  TeamBPanel: NULL"));
    }

    // Bind button events
    if (AutoAssignButton)
    {
        AutoAssignButton->OnClicked.AddDynamic(this, &UMF_TeamSelectionPopup::HandleAutoAssignClicked);
        UE_LOG(LogTemp, Warning, TEXT("  AutoAssignButton: BOUND"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("  AutoAssignButton: NULL - Auto assign will NOT work!"));
    }

    if (CloseButton)
    {
        CloseButton->OnClicked.AddDynamic(this, &UMF_TeamSelectionPopup::HandleCloseClicked);
        UE_LOG(LogTemp, Warning, TEXT("  CloseButton: BOUND"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("  CloseButton: NULL"));
    }

    // Start hidden
    SetVisibility(ESlateVisibility::Collapsed);
    bIsVisible = false;

    UE_LOG(LogTemp, Warning, TEXT("=== MF_TeamSelectionPopup::NativeConstruct END ==="));
}

void UMF_TeamSelectionPopup::NativeDestruct()
{
    // Unbind delegates
    if (TeamAPanel)
    {
        TeamAPanel->OnJoinClicked.RemoveDynamic(this, &UMF_TeamSelectionPopup::HandleTeamAJoinClicked);
    }

    if (TeamBPanel)
    {
        TeamBPanel->OnJoinClicked.RemoveDynamic(this, &UMF_TeamSelectionPopup::HandleTeamBJoinClicked);
    }

    if (AutoAssignButton)
    {
        AutoAssignButton->OnClicked.RemoveDynamic(this, &UMF_TeamSelectionPopup::HandleAutoAssignClicked);
    }

    if (CloseButton)
    {
        CloseButton->OnClicked.RemoveDynamic(this, &UMF_TeamSelectionPopup::HandleCloseClicked);
    }

    Super::NativeDestruct();
}

void UMF_TeamSelectionPopup::ShowPopup()
{
    if (bIsVisible)
    {
        return;
    }

    // Refresh data before showing
    RefreshTeamData();

    // Clear any previous status
    ClearStatus();

    // Show the widget
    SetVisibility(ESlateVisibility::Visible);
    bIsVisible = true;

    // Set input mode to UI
    APlayerController *PC = GetOwningPlayer();
    if (PC)
    {
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }

    // Play show animation if using widget animation
    // PlayAnimation(ShowAnimation);
}

void UMF_TeamSelectionPopup::HidePopup()
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
    }

    // Broadcast closed event
    OnPopupClosed.Broadcast();
}

void UMF_TeamSelectionPopup::RefreshTeamData()
{
    AMF_GameState *GS = GetGameState();
    if (!GS)
    {
        return;
    }

    // Refresh team panels
    if (TeamAPanel)
    {
        FMF_TeamRosterData RosterA = GS->GetTeamRoster(EMF_TeamID::TeamA);
        TeamAPanel->SetPlayerData(RosterA.PlayerNames);
    }

    if (TeamBPanel)
    {
        FMF_TeamRosterData RosterB = GS->GetTeamRoster(EMF_TeamID::TeamB);
        TeamBPanel->SetPlayerData(RosterB.PlayerNames);
    }

    // Update button states
    UpdateJoinButtonStates();
}

void UMF_TeamSelectionPopup::HandleTeamAJoinClicked(EMF_TeamID TeamID)
{
    OnTeamSelected.Broadcast(EMF_TeamID::TeamA);
    RequestJoinTeam(EMF_TeamID::TeamA);
}

void UMF_TeamSelectionPopup::HandleTeamBJoinClicked(EMF_TeamID TeamID)
{
    OnTeamSelected.Broadcast(EMF_TeamID::TeamB);
    RequestJoinTeam(EMF_TeamID::TeamB);
}

void UMF_TeamSelectionPopup::HandleAutoAssignClicked()
{
    // Check if player is already on a team
    AMF_PlayerController *PC = GetMFPlayerController();
    if (PC && PC->GetAssignedTeam() != EMF_TeamID::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("TeamSelectionPopup::HandleAutoAssignClicked - Already on team %d, ignoring"),
               static_cast<int32>(PC->GetAssignedTeam()));
        ShowStatus(TEXT("Already on a team!"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("TeamSelectionPopup::HandleAutoAssignClicked - Requesting server auto-assign"));

    // Let server pick the best team by sending None
    OnTeamSelected.Broadcast(EMF_TeamID::None);
    RequestJoinTeam(EMF_TeamID::None);
}

void UMF_TeamSelectionPopup::HandleCloseClicked()
{
    HidePopup();
}

void UMF_TeamSelectionPopup::HandleBackgroundClicked()
{
    // Optional: close popup when clicking outside
    // HidePopup();
}

void UMF_TeamSelectionPopup::RequestJoinTeam(EMF_TeamID TeamID)
{
    AMF_PlayerController *PC = GetMFPlayerController();
    if (!PC)
    {
        ShowStatus(TEXT("Error: Invalid controller"));
        return;
    }

    // Show pending status
    FString TeamName = (TeamID == EMF_TeamID::TeamA) ? TEXT("Team A") : TEXT("Team B");
    ShowStatus(FString::Printf(TEXT("Joining %s..."), *TeamName));

    // Request join through controller
    PC->Server_RequestJoinTeam(TeamID);

    // Hide popup after requesting (UI will update based on state change)
    HidePopup();
}

void UMF_TeamSelectionPopup::UpdateJoinButtonStates()
{
    AMF_GameState *GS = GetGameState();
    if (!GS)
    {
        return;
    }

    int32 CountA = GS->GetTeamPlayerCount(EMF_TeamID::TeamA);
    int32 CountB = GS->GetTeamPlayerCount(EMF_TeamID::TeamB);

    // Balance rules: can only join team with equal or fewer players
    bool bCanJoinA = (CountA <= CountB);
    bool bCanJoinB = (CountB <= CountA);

    if (TeamAPanel)
    {
        TeamAPanel->SetJoinButtonEnabled(bCanJoinA);
    }

    if (TeamBPanel)
    {
        TeamBPanel->SetJoinButtonEnabled(bCanJoinB);
    }
}

void UMF_TeamSelectionPopup::ShowStatus(const FString &Message)
{
    if (StatusText)
    {
        StatusText->SetText(FText::FromString(Message));
        StatusText->SetVisibility(ESlateVisibility::Visible);
    }
}

void UMF_TeamSelectionPopup::ClearStatus()
{
    if (StatusText)
    {
        StatusText->SetText(FText::GetEmpty());
        StatusText->SetVisibility(ESlateVisibility::Collapsed);
    }
}

AMF_PlayerController *UMF_TeamSelectionPopup::GetMFPlayerController() const
{
    APlayerController *PC = GetOwningPlayer();
    return Cast<AMF_PlayerController>(PC);
}

AMF_GameState *UMF_TeamSelectionPopup::GetGameState() const
{
    UWorld *World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    return Cast<AMF_GameState>(UGameplayStatics::GetGameState(World));
}
