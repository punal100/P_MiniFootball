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
