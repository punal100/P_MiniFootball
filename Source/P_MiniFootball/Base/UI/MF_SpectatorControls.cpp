/*
 * @Author: Punal Manalan
 * @Description: MF_SpectatorControls - Spectator mode UI implementation
 * @Date: 10/12/2025
 */

#include "MF_SpectatorControls.h"
#include "MF_QuickTeamPanel.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MF_PlayerController.h"
#include "Match/MF_GameState.h"

FString UMF_SpectatorControls::GetWidgetSpec()
{
    static FString Spec = R"JSON({
    "WidgetClass": "UMF_SpectatorControls",
    "BlueprintName": "WBP_MF_SpectatorControls",
    "ParentClass": "/Script/P_MiniFootball.MF_SpectatorControls",
    "Category": "MF|UI|HUD",
    "Description": "Spectator mode controls with quick team join",
    "Version": "1.0.0",
    
    "DesignerPreview": {
        "SizeMode": "FillScreen",
        "ZoomLevel": 12,
        "ShowGrid": true
    },
    
    "Hierarchy": {
        "Root": {
            "Type": "CanvasPanel",
            "Name": "RootCanvas",
            "Children": [
                {
                    "Type": "VerticalBox",
                    "Name": "SpectatorContent",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0.5, "Y": 0}, "Max": {"X": 0.5, "Y": 0}},
                        "Alignment": {"X": 0.5, "Y": 0},
                        "Position": {"X": 0, "Y": 50}
                    },
                    "Children": [
                        {
                            "Type": "TextBlock",
                            "Name": "SpectatingLabel",
                            "BindingType": "Optional",
                            "Slot": {"HAlign": "Center"}
                        },
                        {
                            "Type": "TextBlock",
                            "Name": "CameraModeText",
                            "BindingType": "Optional",
                            "Slot": {"HAlign": "Center"}
                        }
                    ]
                },
                {
                    "Type": "HorizontalBox",
                    "Name": "QuickJoinContainer",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0.5, "Y": 1}, "Max": {"X": 0.5, "Y": 1}},
                        "Alignment": {"X": 0.5, "Y": 1},
                        "Position": {"X": 0, "Y": -100}
                    },
                    "Children": [
                        {
                            "Type": "UserWidget",
                            "Name": "QuickTeamA",
                            "BindingType": "Optional",
                            "WidgetClass": "/Script/P_MiniFootball.MF_QuickTeamPanel",
                            "Slot": {"Padding": {"Right": 20}}
                        },
                        {
                            "Type": "Button",
                            "Name": "OpenTeamSelectButton",
                            "BindingType": "Optional"
                        },
                        {
                            "Type": "UserWidget",
                            "Name": "QuickTeamB",
                            "BindingType": "Optional",
                            "WidgetClass": "/Script/P_MiniFootball.MF_QuickTeamPanel",
                            "Slot": {"Padding": {"Left": 20}}
                        }
                    ]
                },
                {
                    "Type": "TextBlock",
                    "Name": "ControlHintsText",
                    "BindingType": "Optional",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0.5, "Y": 1}, "Max": {"X": 0.5, "Y": 1}},
                        "Alignment": {"X": 0.5, "Y": 1},
                        "Position": {"X": 0, "Y": -20}
                    }
                }
            ]
        }
    },
    
    "Design": {
        "SpectatingLabel": {
            "Font": {"Size": 24, "Typeface": "Bold"},
            "Text": "SPECTATING",
            "ColorAndOpacity": {"R": 1.0, "G": 0.9, "B": 0.3, "A": 1.0}
        },
        "CameraModeText": {
            "Font": {"Size": 14, "Typeface": "Regular"},
            "Text": "Free Camera"
        },
        "OpenTeamSelectButton": {
            "Style": {
                "Normal": {"TintColor": {"R": 0.4, "G": 0.4, "B": 0.4, "A": 0.8}}
            }
        },
        "ControlHintsText": {
            "Font": {"Size": 12, "Typeface": "Regular"},
            "Text": "Press T for Team Selection"
        }
    },
    
    "Bindings": {
        "Required": [],
        "Optional": [
            {"Name": "SpectatingLabel", "Type": "UTextBlock", "Purpose": "Spectator mode label"},
            {"Name": "CameraModeText", "Type": "UTextBlock", "Purpose": "Camera mode display"},
            {"Name": "QuickTeamA", "Type": "UMF_QuickTeamPanel", "Purpose": "Quick join Team A"},
            {"Name": "QuickTeamB", "Type": "UMF_QuickTeamPanel", "Purpose": "Quick join Team B"},
            {"Name": "OpenTeamSelectButton", "Type": "UButton", "Purpose": "Open full team select"},
            {"Name": "ControlHintsText", "Type": "UTextBlock", "Purpose": "Control hints"}
        ]
    },
    
    "Delegates": [
        {
            "Name": "OnOpenTeamSelection",
            "Type": "FMF_OnOpenTeamSelection",
            "Signature": "void()",
            "Description": "Request to open team selection popup"
        }
    ],
    
    "Dependencies": [
        {"Class": "UMF_QuickTeamPanel", "Blueprint": "WBP_MF_QuickTeamPanel", "Required": false}
    ],
    
    "Comments": {
        "Header": "MF Spectator Controls - Spectator mode HUD overlay",
        "Usage": "Shown in MF_HUD when player is spectating"
    },
    
    "PythonSnippets": {
        "CreateRoot": "root = creator.add_widget('CanvasPanel', 'RootCanvas', None)",
        "CreateQuickPanels": "creator.add_widget('UserWidget', 'QuickTeamA', hbox, widget_class='WBP_MF_QuickTeamPanel')",
        "Note": "QuickTeamA and QuickTeamB use nested WBP_MF_QuickTeamPanel"
    }
})JSON";
    return Spec;
}

void UMF_SpectatorControls::NativeConstruct()
{
    Super::NativeConstruct();

    // Set initial spectating label
    if (SpectatingLabel)
    {
        SpectatingLabel->SetText(FText::FromString(TEXT("👁 SPECTATING")));
    }

    // Initialize quick team panels
    if (QuickTeamA)
    {
        QuickTeamA->SetTeamID(EMF_TeamID::TeamA);
        QuickTeamA->OnQuickJoinClicked.AddDynamic(this, &UMF_SpectatorControls::HandleQuickJoinTeamA);
    }

    if (QuickTeamB)
    {
        QuickTeamB->SetTeamID(EMF_TeamID::TeamB);
        QuickTeamB->OnQuickJoinClicked.AddDynamic(this, &UMF_SpectatorControls::HandleQuickJoinTeamB);
    }

    // Bind open team selection button
    if (OpenTeamSelectButton)
    {
        OpenTeamSelectButton->OnClicked.AddDynamic(this, &UMF_SpectatorControls::HandleOpenTeamSelectionClicked);
    }

    // Set control hints
    if (ControlHintsText)
    {
        ControlHintsText->SetText(FText::FromString(TEXT("[F] Toggle Camera    [TAB] Full Team Selection")));
    }

    // Initial data refresh
    RefreshTeamData();
}

void UMF_SpectatorControls::NativeDestruct()
{
    // Unbind delegates
    if (QuickTeamA)
    {
        QuickTeamA->OnQuickJoinClicked.RemoveDynamic(this, &UMF_SpectatorControls::HandleQuickJoinTeamA);
    }

    if (QuickTeamB)
    {
        QuickTeamB->OnQuickJoinClicked.RemoveDynamic(this, &UMF_SpectatorControls::HandleQuickJoinTeamB);
    }

    if (OpenTeamSelectButton)
    {
        OpenTeamSelectButton->OnClicked.RemoveDynamic(this, &UMF_SpectatorControls::HandleOpenTeamSelectionClicked);
    }

    Super::NativeDestruct();
}

void UMF_SpectatorControls::RefreshTeamData()
{
    // Refresh quick team panels
    if (QuickTeamA)
    {
        QuickTeamA->RefreshTeamData();
    }

    if (QuickTeamB)
    {
        QuickTeamB->RefreshTeamData();
    }

    // Update button states based on balance rules
    UpdateJoinButtonStates();
}

void UMF_SpectatorControls::UpdateCameraModeDisplay(bool bFollowingBall)
{
    if (CameraModeText)
    {
        FString ModeStr = bFollowingBall ? TEXT("Camera: Following Ball") : TEXT("Camera: Free Roam");
        CameraModeText->SetText(FText::FromString(ModeStr));
    }
}

void UMF_SpectatorControls::HandleQuickJoinTeamA(EMF_TeamID TeamID)
{
    RequestJoinTeam(EMF_TeamID::TeamA);
}

void UMF_SpectatorControls::HandleQuickJoinTeamB(EMF_TeamID TeamID)
{
    RequestJoinTeam(EMF_TeamID::TeamB);
}

void UMF_SpectatorControls::HandleOpenTeamSelectionClicked()
{
    OnOpenTeamSelection.Broadcast();
}

void UMF_SpectatorControls::RequestJoinTeam(EMF_TeamID TeamID)
{
    AMF_PlayerController *PC = GetMFPlayerController();
    if (PC)
    {
        PC->Server_RequestJoinTeam(TeamID);
    }
}

void UMF_SpectatorControls::UpdateJoinButtonStates()
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

    if (QuickTeamA)
    {
        QuickTeamA->SetQuickJoinEnabled(bCanJoinA);
    }

    if (QuickTeamB)
    {
        QuickTeamB->SetQuickJoinEnabled(bCanJoinB);
    }
}

AMF_PlayerController *UMF_SpectatorControls::GetMFPlayerController() const
{
    APlayerController *PC = GetOwningPlayer();
    return Cast<AMF_PlayerController>(PC);
}

AMF_GameState *UMF_SpectatorControls::GetGameState() const
{
    UWorld *World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    return Cast<AMF_GameState>(UGameplayStatics::GetGameState(World));
}
