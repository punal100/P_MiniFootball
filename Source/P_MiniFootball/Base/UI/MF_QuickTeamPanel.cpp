/*
 * @Author: Punal Manalan
 * @Description: MF_QuickTeamPanel - Compact team preview widget implementation
 * @Date: 10/12/2025
 */

#include "MF_QuickTeamPanel.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/Border.h"
#include "Kismet/GameplayStatics.h"
#include "Match/MF_GameState.h"

FString UMF_QuickTeamPanel::GetWidgetSpec()
{
    static FString Spec = R"JSON({
    "WidgetClass": "UMF_QuickTeamPanel",
    "BlueprintName": "WBP_MF_QuickTeamPanel",
    "ParentClass": "/Script/P_MiniFootball.MF_QuickTeamPanel",
    "Category": "MF|UI|Team",
    "Description": "Compact quick-join team panel for spectator mode",
    "Version": "1.0.0",

    "DesignerPreview": {
        "SizeMode": "Desired",
        "ZoomLevel": 14,
        "ShowGrid": true
    },
    
    "Hierarchy": {
        "Root": {
            "Type": "Border",
            "Name": "PanelBorder",
            "BindingType": "Optional",
            "Children": [
                {
                    "Type": "VerticalBox",
                    "Name": "QuickPanelContent",
                    "Properties": {"SizeToContent": true, "Spacing": 4},
                    "Children": [
                        {
                            "Type": "TextBlock",
                            "Name": "TeamNameText",
                            "BindingType": "Required",
                            "Text": "TEAM",
                            "FontSize": 16,
                            "Justification": "Center",
                            "Slot": {"HAlign": "Center", "VAlign": "Fill", "Size": {"Rule": "Auto"}}
                        },
                        {
                            "Type": "TextBlock",
                            "Name": "PlayerCountText",
                            "BindingType": "Required",
                            "Text": "0/3",
                            "FontSize": 12,
                            "Justification": "Center",
                            "Slot": {"HAlign": "Center", "VAlign": "Fill", "Size": {"Rule": "Auto"}}
                        },
                        {
                            "Type": "VerticalBox",
                            "Name": "PlayerListBox",
                            "BindingType": "Optional",
                            "Properties": {"SizeToContent": true, "Spacing": 2},
                            "Slot": {"HAlign": "Fill", "VAlign": "Fill", "Size": {"Rule": "Auto"}}
                        },
                        {
                            "Type": "Button",
                            "Name": "QuickJoinButton",
                            "BindingType": "Required",
                            "Slot": {"HAlign": "Fill", "VAlign": "Fill", "Size": {"Rule": "Auto"}},
                            "Children": [
                                {
                                    "Type": "TextBlock",
                                    "Name": "ShortcutHintText",
                                    "BindingType": "Optional",
                                    "Text": "JOIN",
                                    "FontSize": 12,
                                    "Justification": "Center"
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
            "BrushColor": {"R": 0.15, "G": 0.15, "B": 0.15, "A": 0.85},
            "Padding": {"Left": 8, "Top": 6, "Right": 8, "Bottom": 6}
        },
        "TeamNameText": {
            "Font": {"Size": 16, "Typeface": "Bold"},
            "ColorAndOpacity": {"R": 1.0, "G": 1.0, "B": 1.0, "A": 1.0}
        },
        "PlayerCountText": {
            "Font": {"Size": 12, "Typeface": "Regular"},
            "ColorAndOpacity": {"R": 1.0, "G": 1.0, "B": 1.0, "A": 1.0},
            "Text": "0/5"
        },
        "QuickJoinButton": {
            "Style": {
                "Normal": {"TintColor": {"R": 0.3, "G": 0.5, "B": 0.3, "A": 1.0}},
                "Hovered": {"TintColor": {"R": 0.72, "G": 0.72, "B": 0.72, "A": 1.0}},
                "Pressed": {"TintColor": {"R": 0.38, "G": 0.38, "B": 0.38, "A": 1.0}}
            },
            "IsFocusable": true
        },
        "ShortcutHintText": {
            "Font": {"Size": 12, "Typeface": "Regular"},
            "ColorAndOpacity": {"R": 1.0, "G": 1.0, "B": 1.0, "A": 1.0},
            "Text": "Press 1"
        }
    },
    
    "Bindings": {
        "Required": [
            {"Name": "TeamNameText", "Type": "UTextBlock", "Purpose": "Team name"},
            {"Name": "PlayerCountText", "Type": "UTextBlock", "Purpose": "Player count"},
            {"Name": "QuickJoinButton", "Type": "UButton", "Purpose": "Quick join button"}
        ],
        "Optional": [
            {"Name": "PanelBorder", "Type": "UBorder", "Purpose": "Panel background"},
            {"Name": "PlayerListBox", "Type": "UVerticalBox", "Purpose": "Compact player list"},
            {"Name": "ShortcutHintText", "Type": "UTextBlock", "Purpose": "Keyboard hint"}
        ]
    },
    
    "Delegates": [
        {
            "Name": "OnQuickJoinClicked",
            "Type": "FMF_OnQuickJoinClicked",
            "Signature": "void(EMF_TeamID TeamID)",
            "Description": "Fired when quick join is triggered"
        }
    ],
    
    "Dependencies": [
        "/Engine/EngineFonts/Roboto.Roboto"
    ],
    
    "Comments": {
        "Header": "MF Quick Team Panel - Compact spectator team join",
        "Usage": "Used in SpectatorControls for fast team selection"
    },
    
    "PythonSnippets": {
        "CreateRoot": "root = creator.add_widget('CanvasPanel', 'RootCanvas', None)",
        "CreateBorder": "border = creator.add_widget('Border', 'PanelBorder', root)",
        "CreateContent": "vbox = creator.add_widget('VerticalBox', 'QuickPanelContent', border)"
    }
})JSON";
    return Spec;
}

void UMF_QuickTeamPanel::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind quick join button click
    if (QuickJoinButton)
    {
        QuickJoinButton->OnClicked.AddDynamic(this, &UMF_QuickTeamPanel::HandleQuickJoinClicked);
    }
}

void UMF_QuickTeamPanel::NativeDestruct()
{
    // Unbind delegates
    if (QuickJoinButton)
    {
        QuickJoinButton->OnClicked.RemoveDynamic(this, &UMF_QuickTeamPanel::HandleQuickJoinClicked);
    }

    Super::NativeDestruct();
}

void UMF_QuickTeamPanel::SetTeamID(EMF_TeamID InTeamID)
{
    TeamID = InTeamID;
    UpdateTeamVisuals();

    // Set default shortcut hint
    if (ShortcutHintText)
    {
        FString Hint;
        switch (TeamID)
        {
        case EMF_TeamID::TeamA:
            Hint = TEXT("(1)");
            break;
        case EMF_TeamID::TeamB:
            Hint = TEXT("(2)");
            break;
        default:
            Hint = TEXT("");
            break;
        }
        ShortcutHintText->SetText(FText::FromString(Hint));
    }
}

void UMF_QuickTeamPanel::UpdateTeamVisuals()
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
            TeamName = TEXT("TEAM");
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
}

void UMF_QuickTeamPanel::RefreshTeamData()
{
    AMF_GameState *GS = GetGameState();
    if (!GS)
    {
        return;
    }

    // Get player count for this team
    int32 Count = GS->GetTeamPlayerCount(TeamID);
    SetPlayerCount(Count);

    // Get player names for this team (if we have a player list box)
    if (PlayerListBox)
    {
        PlayerListBox->ClearChildren();

        TArray<FString> Names = GS->GetTeamPlayerNames(TeamID);
        int32 DisplayCount = FMath::Min(Names.Num(), MaxDisplayedPlayers);

        for (int32 i = 0; i < DisplayCount; ++i)
        {
            UTextBlock *NameText = NewObject<UTextBlock>(this);
            if (NameText)
            {
                NameText->SetText(FText::FromString(Names[i]));
                FSlateFontInfo FontInfo = NameText->GetFont();
                FontInfo.Size = 12;
                NameText->SetFont(FontInfo);
                PlayerListBox->AddChild(NameText);
            }
        }

        // Show "+X more" if there are more players
        if (Names.Num() > MaxDisplayedPlayers)
        {
            UTextBlock *MoreText = NewObject<UTextBlock>(this);
            if (MoreText)
            {
                FString MoreStr = FString::Printf(TEXT("+%d more"), Names.Num() - MaxDisplayedPlayers);
                MoreText->SetText(FText::FromString(MoreStr));
                MoreText->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)));
                FSlateFontInfo FontInfo = MoreText->GetFont();
                FontInfo.Size = 10;
                MoreText->SetFont(FontInfo);
                PlayerListBox->AddChild(MoreText);
            }
        }
    }
}

void UMF_QuickTeamPanel::SetPlayerCount(int32 Count)
{
    CachedPlayerCount = Count;

    if (PlayerCountText)
    {
        FString CountStr = FString::Printf(TEXT("(%d)"), Count);
        PlayerCountText->SetText(FText::FromString(CountStr));
    }
}

void UMF_QuickTeamPanel::SetQuickJoinEnabled(bool bEnabled)
{
    if (QuickJoinButton)
    {
        QuickJoinButton->SetIsEnabled(bEnabled);
    }
}

void UMF_QuickTeamPanel::SetShortcutHint(const FString &HintText)
{
    if (ShortcutHintText)
    {
        ShortcutHintText->SetText(FText::FromString(HintText));
    }
}

void UMF_QuickTeamPanel::HandleQuickJoinClicked()
{
    OnQuickJoinClicked.Broadcast(TeamID);
}

AMF_GameState *UMF_QuickTeamPanel::GetGameState() const
{
    UWorld *World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    return Cast<AMF_GameState>(UGameplayStatics::GetGameState(World));
}
