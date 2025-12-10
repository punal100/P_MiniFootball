/*
 * @Author: Punal Manalan
 * @Description: MF_PauseMenu - In-game pause menu implementation
 * @Date: 10/12/2025
 */

#include "MF_PauseMenu.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MF_PlayerController.h"
#include "Match/MF_GameState.h"

void UMF_PauseMenu::NativeConstruct()
{
    Super::NativeConstruct();

    // Set title
    if (TitleText)
    {
        TitleText->SetText(FText::FromString(TEXT("⏸ PAUSED")));
    }

    // Bind button events
    if (ResumeButton)
    {
        ResumeButton->OnClicked.AddDynamic(this, &UMF_PauseMenu::HandleResumeClicked);
    }

    if (LeaveTeamButton)
    {
        LeaveTeamButton->OnClicked.AddDynamic(this, &UMF_PauseMenu::HandleLeaveTeamClicked);
    }

    if (ChangeTeamButton)
    {
        ChangeTeamButton->OnClicked.AddDynamic(this, &UMF_PauseMenu::HandleChangeTeamClicked);
    }

    if (SettingsButton)
    {
        SettingsButton->OnClicked.AddDynamic(this, &UMF_PauseMenu::HandleSettingsClicked);
    }

    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UMF_PauseMenu::HandleQuitClicked);
    }

    // Start hidden
    SetVisibility(ESlateVisibility::Collapsed);
    bIsVisible = false;
}

void UMF_PauseMenu::NativeDestruct()
{
    // Unbind all button events
    if (ResumeButton)
    {
        ResumeButton->OnClicked.RemoveDynamic(this, &UMF_PauseMenu::HandleResumeClicked);
    }

    if (LeaveTeamButton)
    {
        LeaveTeamButton->OnClicked.RemoveDynamic(this, &UMF_PauseMenu::HandleLeaveTeamClicked);
    }

    if (ChangeTeamButton)
    {
        ChangeTeamButton->OnClicked.RemoveDynamic(this, &UMF_PauseMenu::HandleChangeTeamClicked);
    }

    if (SettingsButton)
    {
        SettingsButton->OnClicked.RemoveDynamic(this, &UMF_PauseMenu::HandleSettingsClicked);
    }

    if (QuitButton)
    {
        QuitButton->OnClicked.RemoveDynamic(this, &UMF_PauseMenu::HandleQuitClicked);
    }

    Super::NativeDestruct();
}

void UMF_PauseMenu::ShowMenu()
{
    if (bIsVisible)
    {
        return;
    }

    // Refresh state before showing
    RefreshMenuState();

    // Show the widget
    SetVisibility(ESlateVisibility::Visible);
    bIsVisible = true;

    // Set input mode to UI + Game
    APlayerController *PC = GetOwningPlayer();
    if (PC)
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        InputMode.SetHideCursorDuringCapture(false);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;

        // Optionally pause the game (for single player)
        // UGameplayStatics::SetGamePaused(GetWorld(), true);
    }
}

void UMF_PauseMenu::HideMenu()
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

        // Unpause if we paused
        // UGameplayStatics::SetGamePaused(GetWorld(), false);
    }
}

void UMF_PauseMenu::ToggleMenu()
{
    if (bIsVisible)
    {
        HideMenu();
    }
    else
    {
        ShowMenu();
    }
}

void UMF_PauseMenu::RefreshMenuState()
{
    UpdateCurrentTeamDisplay();
    UpdateLeaveTeamButtonVisibility();
}

void UMF_PauseMenu::HandleResumeClicked()
{
    HideMenu();
    OnResumeClicked.Broadcast();
}

void UMF_PauseMenu::HandleLeaveTeamClicked()
{
    AMF_PlayerController *PC = GetMFPlayerController();
    if (PC)
    {
        PC->Server_RequestLeaveTeam();
    }

    OnLeaveTeamClicked.Broadcast();
    HideMenu();
}

void UMF_PauseMenu::HandleChangeTeamClicked()
{
    // This would typically open the team selection popup
    // For now, we just close this menu
    HideMenu();

    // The HUD should handle opening the team selection popup
    // after receiving this signal
}

void UMF_PauseMenu::HandleSettingsClicked()
{
    // Settings functionality - to be implemented
    // Could open a settings submenu or separate widget
}

void UMF_PauseMenu::HandleQuitClicked()
{
    OnQuitToMenuClicked.Broadcast();

    // Leave team first if on one
    AMF_PlayerController *PC = GetMFPlayerController();
    if (PC)
    {
        EMF_TeamID CurrentTeam = GetCurrentTeam();
        if (CurrentTeam != EMF_TeamID::None)
        {
            PC->Server_RequestLeaveTeam();
        }
    }

    // Quit to main menu
    UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("MainMenu")));
}

void UMF_PauseMenu::UpdateLeaveTeamButtonVisibility()
{
    if (!LeaveTeamButton)
    {
        return;
    }

    EMF_TeamID CurrentTeam = GetCurrentTeam();
    bool bOnTeam = (CurrentTeam != EMF_TeamID::None);

    LeaveTeamButton->SetVisibility(bOnTeam ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

    // Also update change team button
    if (ChangeTeamButton)
    {
        ChangeTeamButton->SetVisibility(bOnTeam ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UMF_PauseMenu::UpdateCurrentTeamDisplay()
{
    if (!CurrentTeamText)
    {
        return;
    }

    EMF_TeamID CurrentTeam = GetCurrentTeam();
    FString TeamStr;

    switch (CurrentTeam)
    {
    case EMF_TeamID::TeamA:
        TeamStr = TEXT("Current Team: Team A");
        break;
    case EMF_TeamID::TeamB:
        TeamStr = TEXT("Current Team: Team B");
        break;
    case EMF_TeamID::None:
    default:
        TeamStr = TEXT("Spectating");
        break;
    }

    CurrentTeamText->SetText(FText::FromString(TeamStr));
}

AMF_PlayerController *UMF_PauseMenu::GetMFPlayerController() const
{
    APlayerController *PC = GetOwningPlayer();
    return Cast<AMF_PlayerController>(PC);
}

AMF_GameState *UMF_PauseMenu::GetGameState() const
{
    UWorld *World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    return Cast<AMF_GameState>(UGameplayStatics::GetGameState(World));
}

EMF_TeamID UMF_PauseMenu::GetCurrentTeam() const
{
    AMF_PlayerController *PC = GetMFPlayerController();
    if (PC)
    {
        return PC->GetCurrentTeam();
    }

    return EMF_TeamID::None;
}
