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
