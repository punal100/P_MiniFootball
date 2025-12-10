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
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MF_PlayerController.h"
#include "Match/MF_GameState.h"

void UMF_TeamSelectionPopup::NativeConstruct()
{
    Super::NativeConstruct();

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
    }

    if (TeamBPanel)
    {
        TeamBPanel->SetTeamID(EMF_TeamID::TeamB);
        TeamBPanel->OnJoinClicked.AddDynamic(this, &UMF_TeamSelectionPopup::HandleTeamBJoinClicked);
    }

    // Bind button events
    if (AutoAssignButton)
    {
        AutoAssignButton->OnClicked.AddDynamic(this, &UMF_TeamSelectionPopup::HandleAutoAssignClicked);
    }

    if (CloseButton)
    {
        CloseButton->OnClicked.AddDynamic(this, &UMF_TeamSelectionPopup::HandleCloseClicked);
    }

    // Start hidden
    SetVisibility(ESlateVisibility::Collapsed);
    bIsVisible = false;
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
    AMF_GameState *GS = GetGameState();
    if (!GS)
    {
        return;
    }

    // Pick team with fewer players (or Team A if equal)
    int32 CountA = GS->GetTeamPlayerCount(EMF_TeamID::TeamA);
    int32 CountB = GS->GetTeamPlayerCount(EMF_TeamID::TeamB);

    EMF_TeamID SelectedTeam = (CountB < CountA) ? EMF_TeamID::TeamB : EMF_TeamID::TeamA;

    OnTeamSelected.Broadcast(SelectedTeam);
    RequestJoinTeam(SelectedTeam);
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
