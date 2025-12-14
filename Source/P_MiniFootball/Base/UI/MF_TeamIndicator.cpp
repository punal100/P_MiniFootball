/*
 * @Author: Punal Manalan
 * @Description: MF_TeamIndicator - Shows current player's team implementation
 * @Date: 10/12/2025
 */

#include "MF_TeamIndicator.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Player/MF_PlayerController.h"

FString UMF_TeamIndicator::GetWidgetSpec()
{
    static FString Spec = R"JSON({
    "WidgetClass": "UMF_TeamIndicator",
    "BlueprintName": "WBP_MF_TeamIndicator",
    "ParentClass": "/Script/P_MiniFootball.MF_TeamIndicator",
    "Category": "MF|UI|HUD",
    "Description": "Shows current team affiliation with colored border",
    "Version": "1.0.0",
    
    "DesignerPreview": {
        "SizeMode": "Desired",
        "ZoomLevel": 14,
        "ShowGrid": true
    },
    
    "Hierarchy": {
        "Root": {
            "Type": "Border",
            "Name": "TeamColorBorder",
            "BindingType": "Optional",
            "Children": [
                {
                    "Type": "HorizontalBox",
                    "Name": "TeamContentBox",
                    "Children": [
                        {
                            "Type": "Image",
                            "Name": "TeamIcon",
                            "BindingType": "Optional",
                            "Slot": {"VAlign": "Center", "Padding": {"Right": 8}}
                        },
                        {
                            "Type": "TextBlock",
                            "Name": "TeamText",
                            "BindingType": "Required",
                            "Text": "SPECTATING",
                            "FontSize": 18,
                            "Justification": "Left",
                            "Slot": {"VAlign": "Center"}
                        }
                    ]
                }
            ]
        }
    },
    
    "Design": {
        "TeamColorBorder": {
            "BrushColor": {"R": 0.3, "G": 0.3, "B": 0.3, "A": 0.8},
            "Padding": {"Left": 10, "Top": 5, "Right": 10, "Bottom": 5}
        },
        "TeamIcon": {
            "Size": {"X": 24, "Y": 24}
        },
        "TeamText": {
            "Font": {"Size": 18, "Typeface": "Bold"},
            "ColorAndOpacity": {"R": 1.0, "G": 1.0, "B": 1.0, "A": 1.0},
            "Text": "Spectator"
        }
    },
    
    "Bindings": {
        "Required": [
            {"Name": "TeamText", "Type": "UTextBlock", "Purpose": "Team name/status display"}
        ],
        "Optional": [
            {"Name": "TeamColorBorder", "Type": "UBorder", "Purpose": "Colored background"},
            {"Name": "TeamIcon", "Type": "UImage", "Purpose": "Team logo/icon"}
        ]
    },
    
    "Delegates": [],
    
    "Dependencies": [],
    
    "Comments": {
        "Header": "MF Team Indicator - Current team display",
        "Usage": "Place in HUD to show player's team affiliation"
    },
    
    "PythonSnippets": {
        "CreateRoot": "root = creator.add_widget('CanvasPanel', 'RootCanvas', None)",
        "CreateBorder": "border = creator.add_widget('Border', 'TeamColorBorder', root, slot_data={'anchors': 'fill'})",
        "CreateText": "text = creator.add_widget('TextBlock', 'TeamText', border)"
    }
})JSON";
    return Spec;
}

void UMF_TeamIndicator::NativeConstruct()
{
    Super::NativeConstruct();

    // Initial state
    RefreshFromController();
}

void UMF_TeamIndicator::SetTeam(EMF_TeamID TeamID)
{
    FString TeamName;
    FLinearColor TeamColor;

    switch (TeamID)
    {
    case EMF_TeamID::TeamA:
        TeamName = TEXT("TEAM A");
        TeamColor = TeamAColor;
        break;
    case EMF_TeamID::TeamB:
        TeamName = TEXT("TEAM B");
        TeamColor = TeamBColor;
        break;
    default:
        SetSpectating();
        return;
    }

    if (TeamText)
    {
        TeamText->SetText(FText::FromString(TeamName));
        TeamText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    }

    if (TeamColorBorder)
    {
        TeamColorBorder->SetBrushColor(TeamColor);
    }
}

void UMF_TeamIndicator::SetSpectating()
{
    if (TeamText)
    {
        TeamText->SetText(FText::FromString(TEXT("SPECTATING")));
        TeamText->SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f)));
    }

    if (TeamColorBorder)
    {
        TeamColorBorder->SetBrushColor(SpectatorColor);
    }
}

void UMF_TeamIndicator::RefreshFromController()
{
    AMF_PlayerController *PC = GetMFPlayerController();
    if (!PC)
    {
        SetSpectating();
        return;
    }

    // Check spectator state first
    if (PC->CurrentSpectatorState == EMF_SpectatorState::Spectating)
    {
        SetSpectating();
        return;
    }

    // Show assigned team
    EMF_TeamID Team = PC->AssignedTeam;
    if (Team == EMF_TeamID::None)
    {
        SetSpectating();
    }
    else
    {
        SetTeam(Team);
    }
}

AMF_PlayerController *UMF_TeamIndicator::GetMFPlayerController() const
{
    APlayerController *PC = GetOwningPlayer();
    return Cast<AMF_PlayerController>(PC);
}
