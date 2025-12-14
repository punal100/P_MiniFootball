/*
 * @Author: Punal Manalan
 * @Description: MF_TeamPanel - Reusable team info panel widget implementation
 * @Date: 10/12/2025
 */

#include "MF_TeamPanel.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/Border.h"

FString UMF_TeamPanel::GetWidgetSpec()
{
    static FString Spec = R"JSON({
    "WidgetClass": "UMF_TeamPanel",
    "BlueprintName": "WBP_MF_TeamPanel",
    "ParentClass": "/Script/P_MiniFootball.MF_TeamPanel",
    "Category": "MF|UI|Team",
    "Description": "Full team selection panel with player list",
    "Version": "1.0.0",
    
    "DesignerPreview": {
        "SizeMode": "DesiredOnScreen",
        "ZoomLevel": 14,
        "ShowGrid": true
    },
    
    "Hierarchy": {
        "Root": {
            "Type": "CanvasPanel",
            "Name": "RootCanvas",
            "Children": [
                {
                    "Type": "Border",
                    "Name": "PanelBorder",
                    "BindingType": "Optional",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0, "Y": 0}, "Max": {"X": 1, "Y": 1}},
                        "Offsets": {"Left": 0, "Top": 0, "Right": 0, "Bottom": 0}
                    },
                    "Children": [
                        {
                            "Type": "VerticalBox",
                            "Name": "PanelContent",
                            "Children": [
                                {
                                    "Type": "TextBlock",
                                    "Name": "TeamNameText",
                                    "BindingType": "Required",
                                    "Text": "TEAM",
                                    "FontSize": 24,
                                    "Justification": "Center",
                                    "Slot": {"HAlign": "Center", "Padding": {"Top": 10, "Bottom": 5}}
                                },
                                {
                                    "Type": "TextBlock",
                                    "Name": "PlayerCountText",
                                    "BindingType": "Required",
                                    "Text": "0/3 PLAYERS",
                                    "FontSize": 14,
                                    "Justification": "Center",
                                    "Slot": {"HAlign": "Center", "Padding": {"Bottom": 10}}
                                },
                                {
                                    "Type": "VerticalBox",
                                    "Name": "PlayerListBox",
                                    "BindingType": "Required",
                                    "Slot": {"Fill": 1.0}
                                },
                                {
                                    "Type": "Button",
                                    "Name": "JoinButton",
                                    "BindingType": "Required",
                                    "Slot": {"HAlign": "Center", "Padding": {"Top": 10, "Bottom": 10}},
                                    "Children": [
                                        {
                                            "Type": "TextBlock",
                                            "Name": "JoinButtonText",
                                            "BindingType": "Optional",
                                            "Text": "JOIN TEAM",
                                            "FontSize": 16,
                                            "Justification": "Center",
                                            "Slot": {"HAlign": "Center", "VAlign": "Center"}
                                        }
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
        "PanelBorder": {
            "BrushColor": {"R": 0.1, "G": 0.1, "B": 0.1, "A": 0.9},
            "Padding": {"Left": 15, "Top": 10, "Right": 15, "Bottom": 10}
        },
        "TeamNameText": {
            "Font": {"Size": 24, "Typeface": "Bold"},
            "Text": "Team A"
        },
        "PlayerCountText": {
            "Font": {"Size": 14, "Typeface": "Regular"},
            "Text": "0/5 Players"
        },
        "JoinButton": {
            "Style": {
                "Normal": {"TintColor": {"R": 0.2, "G": 0.6, "B": 0.2, "A": 1.0}}
            }
        },
        "JoinButtonText": {
            "Font": {"Size": 16, "Typeface": "Bold"},
            "Text": "JOIN TEAM"
        }
    },
    
    "Bindings": {
        "Required": [
            {"Name": "TeamNameText", "Type": "UTextBlock", "Purpose": "Team name header"},
            {"Name": "PlayerCountText", "Type": "UTextBlock", "Purpose": "Player count display"},
            {"Name": "PlayerListBox", "Type": "UVerticalBox", "Purpose": "Player list container"},
            {"Name": "JoinButton", "Type": "UButton", "Purpose": "Join team button"}
        ],
        "Optional": [
            {"Name": "PanelBorder", "Type": "UBorder", "Purpose": "Team-colored background"},
            {"Name": "JoinButtonText", "Type": "UTextBlock", "Purpose": "Button label"}
        ]
    },
    
    "Delegates": [
        {
            "Name": "OnJoinClicked",
            "Type": "FMF_OnTeamJoinClicked",
            "Signature": "void(EMF_TeamID TeamID)",
            "Description": "Fired when join button is clicked"
        }
    ],
    
    "Dependencies": [],
    
    "Comments": {
        "Header": "MF Team Panel - Full team selection with player list",
        "Usage": "Used in TeamSelectionPopup for detailed team view"
    },
    
    "PythonSnippets": {
        "CreateRoot": "root = creator.add_widget('CanvasPanel', 'RootCanvas', None)",
        "CreateBorder": "border = creator.add_widget('Border', 'PanelBorder', root, slot_data={'anchors': 'fill'})",
        "CreateVBox": "vbox = creator.add_widget('VerticalBox', 'PanelContent', border)"
    }
})JSON";
    return Spec;
}

void UMF_TeamPanel::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind join button click
    if (JoinButton)
    {
        JoinButton->OnClicked.AddDynamic(this, &UMF_TeamPanel::HandleJoinButtonClicked);
    }
}

void UMF_TeamPanel::NativeDestruct()
{
    // Unbind delegates
    if (JoinButton)
    {
        JoinButton->OnClicked.RemoveDynamic(this, &UMF_TeamPanel::HandleJoinButtonClicked);
    }

    Super::NativeDestruct();
}

void UMF_TeamPanel::SetTeamID(EMF_TeamID InTeamID)
{
    TeamID = InTeamID;
    UpdateTeamVisuals();
}

void UMF_TeamPanel::UpdateTeamVisuals()
{
    // Set team name
    if (TeamNameText)
    {
        FString TeamName;
        switch (TeamID)
        {
        case EMF_TeamID::TeamA:
            TeamName = TEXT("TEAM A");
            break;
        case EMF_TeamID::TeamB:
            TeamName = TEXT("TEAM B");
            break;
        default:
            TeamName = TEXT("NO TEAM");
            break;
        }
        TeamNameText->SetText(FText::FromString(TeamName));
    }

    // Set team color on border
    if (PanelBorder)
    {
        FLinearColor TeamColor;
        switch (TeamID)
        {
        case EMF_TeamID::TeamA:
            TeamColor = TeamAColor;
            break;
        case EMF_TeamID::TeamB:
            TeamColor = TeamBColor;
            break;
        default:
            TeamColor = FLinearColor::Gray;
            break;
        }
        PanelBorder->SetBrushColor(TeamColor);
    }

    // Set default button text
    if (JoinButtonText)
    {
        FString ButtonText;
        switch (TeamID)
        {
        case EMF_TeamID::TeamA:
            ButtonText = TEXT("JOIN TEAM A");
            break;
        case EMF_TeamID::TeamB:
            ButtonText = TEXT("JOIN TEAM B");
            break;
        default:
            ButtonText = TEXT("JOIN");
            break;
        }
        JoinButtonText->SetText(FText::FromString(ButtonText));
    }
}

void UMF_TeamPanel::SetPlayerData(const TArray<FString> &PlayerNames)
{
    // Use default max players of 3
    SetPlayerDataWithMax(PlayerNames, 3);
}

void UMF_TeamPanel::SetPlayerDataWithMax(const TArray<FString> &PlayerNames, int32 MaxPlayers)
{
    // Update player count text
    if (PlayerCountText)
    {
        FString CountStr = FString::Printf(TEXT("Players: %d/%d"), PlayerNames.Num(), MaxPlayers);
        PlayerCountText->SetText(FText::FromString(CountStr));
    }

    // Clear existing player list
    if (PlayerListBox)
    {
        PlayerListBox->ClearChildren();

        // Add player names
        for (const FString &Name : PlayerNames)
        {
            UTextBlock *NameText = CreatePlayerNameText(Name);
            if (NameText)
            {
                PlayerListBox->AddChild(NameText);
            }
        }

        // Add empty slots
        int32 EmptySlots = MaxPlayers - PlayerNames.Num();
        for (int32 i = 0; i < EmptySlots; ++i)
        {
            UTextBlock *EmptyText = CreatePlayerNameText(TEXT("[Empty Slot]"));
            if (EmptyText)
            {
                EmptyText->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f, 0.7f)));
                PlayerListBox->AddChild(EmptyText);
            }
        }
    }
}

void UMF_TeamPanel::SetJoinButtonEnabled(bool bEnabled)
{
    if (JoinButton)
    {
        JoinButton->SetIsEnabled(bEnabled);
    }
}

void UMF_TeamPanel::SetJoinButtonText(const FString &NewText)
{
    if (JoinButtonText)
    {
        JoinButtonText->SetText(FText::FromString(NewText));
    }
}

void UMF_TeamPanel::HandleJoinButtonClicked()
{
    OnJoinClicked.Broadcast(TeamID);
}

UTextBlock *UMF_TeamPanel::CreatePlayerNameText(const FString &PlayerName)
{
    UTextBlock *TextBlock = NewObject<UTextBlock>(this);
    if (TextBlock)
    {
        TextBlock->SetText(FText::FromString(FString::Printf(TEXT("• %s"), *PlayerName)));
        TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));

        // Set font size
        FSlateFontInfo FontInfo = TextBlock->GetFont();
        FontInfo.Size = 14;
        TextBlock->SetFont(FontInfo);
    }
    return TextBlock;
}
