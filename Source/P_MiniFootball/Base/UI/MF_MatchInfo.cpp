/*
 * @Author: Punal Manalan
 * @Description: MF_MatchInfo - Score, time, and match phase display implementation
 * @Date: 10/12/2025
 */

#include "MF_MatchInfo.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Match/MF_GameState.h"

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
