/*
 * @Author: Punal Manalan
 * @Description: MF_MatchInfo - Score, time, and match phase display implementation
 * @Date: 10/12/2025
 */

#include "MF_MatchInfo.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Match/MF_GameState.h"

FString UMF_MatchInfo::GetWidgetSpec()
{
    static FString Spec = R"JSON({
    "WidgetClass": "UMF_MatchInfo",
    "BlueprintName": "WBP_MF_MatchInfo",
    "ParentClass": "/Script/P_MiniFootball.MF_MatchInfo",
    "Category": "MF|UI|HUD",
    "Description": "Match score and timer display panel",
    "Version": "1.0.0",
    
    "DesignerToolbar": {
        "DesiredSize": {"Width": 600, "Height": 200},
        "ZoomLevel": "1:1",
        "ShowGrid": true
    },
    
    "Hierarchy": {
        "Root": {
            "Type": "CanvasPanel",
            "Name": "RootCanvas",
            "Children": [
                {
                    "Type": "HorizontalBox",
                    "Name": "ScoreContainer",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0.5, "Y": 0}, "Max": {"X": 0.5, "Y": 0}},
                        "Position": {"X": 0, "Y": 10},
                        "Size": {"X": 400, "Y": 80},
                        "Alignment": {"X": 0.5, "Y": 0}
                    },
                    "Children": [
                        {
                            "Type": "VerticalBox",
                            "Name": "TeamABox",
                            "Children": [
                                {"Type": "TextBlock", "Name": "TeamANameText", "BindingType": "Optional"},
                                {"Type": "TextBlock", "Name": "TeamAScoreText", "BindingType": "Required"}
                            ]
                        },
                        {"Type": "TextBlock", "Name": "MatchTimerText", "BindingType": "Required"},
                        {
                            "Type": "VerticalBox",
                            "Name": "TeamBBox",
                            "Children": [
                                {"Type": "TextBlock", "Name": "TeamBNameText", "BindingType": "Optional"},
                                {"Type": "TextBlock", "Name": "TeamBScoreText", "BindingType": "Required"}
                            ]
                        }
                    ]
                },
                {
                    "Type": "TextBlock",
                    "Name": "MatchPhaseText",
                    "BindingType": "Optional",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0.5, "Y": 0}, "Max": {"X": 0.5, "Y": 0}},
                        "Position": {"X": 0, "Y": 95},
                        "Alignment": {"X": 0.5, "Y": 0}
                    }
                }
            ]
        }
    },
    
    "Design": {
        "TeamAScoreText": {
            "Font": {"Size": 36, "Typeface": "Bold"},
            "ColorAndOpacity": {"R": 0.2, "G": 0.6, "B": 1.0, "A": 1.0},
            "Justification": "Center"
        },
        "TeamBScoreText": {
            "Font": {"Size": 36, "Typeface": "Bold"},
            "ColorAndOpacity": {"R": 1.0, "G": 0.3, "B": 0.3, "A": 1.0},
            "Justification": "Center"
        },
        "MatchTimerText": {
            "Font": {"Size": 28, "Typeface": "Bold"},
            "ColorAndOpacity": {"R": 1.0, "G": 1.0, "B": 1.0, "A": 1.0},
            "Justification": "Center",
            "Text": "00:00"
        },
        "MatchPhaseText": {
            "Font": {"Size": 16, "Typeface": "Regular"},
            "ColorAndOpacity": {"R": 0.8, "G": 0.8, "B": 0.8, "A": 1.0}
        }
    },
    
    "Bindings": {
        "Required": [
            {"Name": "TeamAScoreText", "Type": "UTextBlock", "Purpose": "Team A score display"},
            {"Name": "TeamBScoreText", "Type": "UTextBlock", "Purpose": "Team B score display"},
            {"Name": "MatchTimerText", "Type": "UTextBlock", "Purpose": "Match countdown timer"}
        ],
        "Optional": [
            {"Name": "MatchPhaseText", "Type": "UTextBlock", "Purpose": "Current match phase"},
            {"Name": "TeamANameText", "Type": "UTextBlock", "Purpose": "Team A name"},
            {"Name": "TeamBNameText", "Type": "UTextBlock", "Purpose": "Team B name"}
        ]
    },
    
    "Delegates": [],
    
    "Dependencies": [],
    
    "Comments": {
        "Header": "MF Match Info - Score and timer display for HUD",
        "Usage": "Place at top of MF_HUD for match status"
    },
    
    "PythonSnippets": {
        "CreateRoot": "root = creator.add_widget('CanvasPanel', 'RootCanvas', None)",
        "CreateScoreBox": "hbox = creator.add_widget('HorizontalBox', 'ScoreContainer', root)",
        "CreateScores": "creator.add_widget('TextBlock', 'TeamAScoreText', hbox); creator.add_widget('TextBlock', 'TeamBScoreText', hbox)"
    }
})JSON";
    return Spec;
}

void UMF_MatchInfo::NativeConstruct()
{
    Super::NativeConstruct();

    // Set initial team names
    if (TeamANameText)
    {
        TeamANameText->SetText(FText::FromString(TEXT("TEAM A")));
    }
    if (TeamBNameText)
    {
        TeamBNameText->SetText(FText::FromString(TEXT("TEAM B")));
    }

    // Initial refresh
    RefreshMatchInfo();
}

void UMF_MatchInfo::NativeTick(const FGeometry &MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Periodic update from GameState
    UpdateTimer += InDeltaTime;
    if (UpdateTimer >= UpdateInterval)
    {
        UpdateTimer = 0.0f;
        RefreshMatchInfo();
    }
}

void UMF_MatchInfo::RefreshMatchInfo()
{
    AMF_GameState *GS = GetGameState();
    if (!GS)
    {
        return;
    }

    // Update scores if changed
    int32 ScoreA = GS->ScoreTeamA;
    int32 ScoreB = GS->ScoreTeamB;
    if (ScoreA != CachedTeamAScore || ScoreB != CachedTeamBScore)
    {
        SetScores(ScoreA, ScoreB);
        CachedTeamAScore = ScoreA;
        CachedTeamBScore = ScoreB;
    }

    // Update time if changed significantly
    float TimeRemaining = GS->MatchTimeRemaining;
    if (FMath::Abs(TimeRemaining - CachedTimeRemaining) > 0.05f)
    {
        SetMatchTime(TimeRemaining);
        CachedTimeRemaining = TimeRemaining;
    }

    // Update phase if changed
    EMF_MatchPhase Phase = GS->CurrentPhase;
    if (Phase != CachedPhase)
    {
        SetMatchPhase(Phase);
        CachedPhase = Phase;
    }
}

void UMF_MatchInfo::SetScores(int32 TeamAScore, int32 TeamBScore)
{
    if (TeamAScoreText)
    {
        TeamAScoreText->SetText(FText::FromString(FString::Printf(TEXT("%d"), TeamAScore)));
    }
    if (TeamBScoreText)
    {
        TeamBScoreText->SetText(FText::FromString(FString::Printf(TEXT("%d"), TeamBScore)));
    }
}

void UMF_MatchInfo::SetMatchTime(float TimeRemaining)
{
    UpdateTimerDisplay(TimeRemaining);
}

void UMF_MatchInfo::SetMatchPhase(EMF_MatchPhase Phase)
{
    if (MatchPhaseText)
    {
        MatchPhaseText->SetText(FText::FromString(GetPhaseDisplayString(Phase)));
    }
}

void UMF_MatchInfo::UpdateTimerDisplay(float TimeRemaining)
{
    if (!MatchTimerText)
    {
        return;
    }

    // Clamp to non-negative
    TimeRemaining = FMath::Max(0.0f, TimeRemaining);

    // Format as MM:SS
    int32 TotalSeconds = FMath::FloorToInt(TimeRemaining);
    int32 Minutes = TotalSeconds / 60;
    int32 Seconds = TotalSeconds % 60;

    FString TimeStr = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
    MatchTimerText->SetText(FText::FromString(TimeStr));
}

FString UMF_MatchInfo::GetPhaseDisplayString(EMF_MatchPhase Phase) const
{
    switch (Phase)
    {
    case EMF_MatchPhase::WaitingForPlayers:
        return TEXT("WAITING FOR PLAYERS");
    case EMF_MatchPhase::Kickoff:
        return TEXT("KICKOFF");
    case EMF_MatchPhase::Playing:
        return TEXT(""); // Don't show anything during normal play
    case EMF_MatchPhase::GoalScored:
        return TEXT("GOAL!");
    case EMF_MatchPhase::HalfTime:
        return TEXT("HALF TIME");
    case EMF_MatchPhase::MatchEnd:
        return TEXT("FULL TIME");
    default:
        return TEXT("");
    }
}

AMF_GameState *UMF_MatchInfo::GetGameState() const
{
    UWorld *World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    return Cast<AMF_GameState>(UGameplayStatics::GetGameState(World));
}
