/*
 * @Author: Punal Manalan
 * @Description: MF_HUD - Main HUD container implementation
 * @Date: 10/12/2025
 */

#include "MF_HUD.h"
#include "MF_MatchInfo.h"
#include "MF_TeamIndicator.h"
#include "MF_TransitionOverlay.h"
#include "MF_SpectatorControls.h"
#include "MF_GameplayControls.h"
#include "MF_TeamSelectionPopup.h"
#include "MF_PauseMenu.h"
#include "Components/WidgetSwitcher.h"
#include "Components/CanvasPanel.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MF_PlayerController.h"
#include "Match/MF_GameState.h"

void UMF_HUD::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind spectator controls events
    if (SpectatorControls)
    {
        SpectatorControls->OnOpenTeamSelection.AddDynamic(this, &UMF_HUD::HandleOpenTeamSelection);
    }

    // Bind team selection popup events
    if (TeamSelectionPopup)
    {
        TeamSelectionPopup->OnPopupClosed.AddDynamic(this, &UMF_HUD::HandleTeamSelectionClosed);
    }

    // Bind pause menu events
    if (PauseMenu)
    {
        PauseMenu->OnResumeClicked.AddDynamic(this, &UMF_HUD::HandlePauseMenuClosed);
    }

    // Initial state refresh
    RefreshFromPlayerState();

    // Set initial mode
    SetHUDMode(DetermineAppropriateMode());
}

void UMF_HUD::NativeDestruct()
{
    // Unbind all delegates
    if (SpectatorControls)
    {
        SpectatorControls->OnOpenTeamSelection.RemoveDynamic(this, &UMF_HUD::HandleOpenTeamSelection);
    }

    if (TeamSelectionPopup)
    {
        TeamSelectionPopup->OnPopupClosed.RemoveDynamic(this, &UMF_HUD::HandleTeamSelectionClosed);
    }

    if (PauseMenu)
    {
        PauseMenu->OnResumeClicked.RemoveDynamic(this, &UMF_HUD::HandlePauseMenuClosed);
    }

    CachedPlayerController = nullptr;

    Super::NativeDestruct();
}

void UMF_HUD::NativeTick(const FGeometry &MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Periodic refresh
    RefreshTimer += InDeltaTime;
    if (RefreshTimer >= RefreshInterval)
    {
        RefreshTimer = 0.0f;

        // Check if mode should change
        EMF_HUDMode AppropriateMode = DetermineAppropriateMode();
        if (AppropriateMode != CurrentMode && CurrentMode != EMF_HUDMode::Menu)
        {
            SetHUDMode(AppropriateMode);
        }

        // Refresh match info
        if (MatchInfo)
        {
            MatchInfo->RefreshMatchInfo();
        }
    }
}

void UMF_HUD::SetHUDMode(EMF_HUDMode NewMode)
{
    if (NewMode == CurrentMode)
    {
        return;
    }

    PreviousMode = CurrentMode;
    CurrentMode = NewMode;

    // Update widget visibility
    UpdateWidgetSwitcher();

    // Update team indicator
    if (TeamIndicator)
    {
        TeamIndicator->RefreshFromController();
    }
}

void UMF_HUD::ShowTeamSelectionPopup()
{
    if (TeamSelectionPopup)
    {
        PreviousMode = CurrentMode;
        CurrentMode = EMF_HUDMode::Menu;
        TeamSelectionPopup->ShowPopup();
    }
}

void UMF_HUD::HideTeamSelectionPopup()
{
    if (TeamSelectionPopup)
    {
        TeamSelectionPopup->HidePopup();
        // Mode will be restored in HandleTeamSelectionClosed
    }
}

void UMF_HUD::ShowPauseMenu()
{
    if (PauseMenu)
    {
        PreviousMode = CurrentMode;
        CurrentMode = EMF_HUDMode::Menu;
        PauseMenu->ShowMenu();
    }
}

void UMF_HUD::HidePauseMenu()
{
    if (PauseMenu)
    {
        PauseMenu->HideMenu();
        // Mode will be restored in HandlePauseMenuClosed
    }
}

void UMF_HUD::TogglePauseMenu()
{
    if (PauseMenu)
    {
        if (PauseMenu->IsMenuVisible())
        {
            HidePauseMenu();
        }
        else
        {
            ShowPauseMenu();
        }
    }
}

void UMF_HUD::ShowTransitionOverlay(const FString &Message)
{
    if (TransitionOverlay)
    {
        TransitionOverlay->SetStatusText(Message);
        TransitionOverlay->ShowOverlay();
    }

    PreviousMode = CurrentMode;
    CurrentMode = EMF_HUDMode::Transition;
}

void UMF_HUD::HideTransitionOverlay()
{
    if (TransitionOverlay)
    {
        TransitionOverlay->HideOverlay();
    }

    // Restore previous mode
    CurrentMode = PreviousMode;
    UpdateWidgetSwitcher();
}

void UMF_HUD::RefreshFromPlayerState()
{
    // Update team indicator
    if (TeamIndicator)
    {
        TeamIndicator->RefreshFromController();
    }

    // Update match info
    if (MatchInfo)
    {
        MatchInfo->RefreshMatchInfo();
    }

    // Update spectator controls if visible
    if (SpectatorControls && CurrentMode == EMF_HUDMode::Spectator)
    {
        SpectatorControls->RefreshTeamData();
    }

    // Update gameplay controls if visible
    if (GameplayControls && CurrentMode == EMF_HUDMode::Gameplay)
    {
        GameplayControls->RefreshControlLayout();
    }
}

void UMF_HUD::HandleOpenTeamSelection()
{
    ShowTeamSelectionPopup();
}

void UMF_HUD::HandleTeamSelectionClosed()
{
    // Determine new mode based on player state (may have joined a team)
    EMF_HUDMode NewMode = DetermineAppropriateMode();
    SetHUDMode(NewMode);
}

void UMF_HUD::HandlePauseMenuClosed()
{
    // Restore previous mode
    SetHUDMode(PreviousMode);
}

void UMF_HUD::UpdateWidgetSwitcher()
{
    if (!ModeSwitcher)
    {
        return;
    }

    switch (CurrentMode)
    {
    case EMF_HUDMode::Spectator:
        // Show spectator controls (index 0)
        ModeSwitcher->SetActiveWidgetIndex(0);
        break;

    case EMF_HUDMode::Gameplay:
        // Show gameplay controls (index 1)
        ModeSwitcher->SetActiveWidgetIndex(1);
        break;

    case EMF_HUDMode::Menu:
    case EMF_HUDMode::Transition:
        // Keep current widget visible, popups are overlays
        break;
    }

    // Update controls state
    if (SpectatorControls)
    {
        SpectatorControls->SetVisibility(CurrentMode == EMF_HUDMode::Spectator ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    if (GameplayControls)
    {
        bool bShowGameplay = (CurrentMode == EMF_HUDMode::Gameplay);
        GameplayControls->SetVisibility(bShowGameplay ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

        if (bShowGameplay)
        {
            GameplayControls->SetControlsEnabled(true);
        }
    }
}

AMF_PlayerController *UMF_HUD::GetMFPlayerController()
{
    if (!CachedPlayerController)
    {
        APlayerController *PC = GetOwningPlayer();
        CachedPlayerController = Cast<AMF_PlayerController>(PC);
    }

    return CachedPlayerController;
}

AMF_GameState *UMF_HUD::GetGameState() const
{
    UWorld *World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    return Cast<AMF_GameState>(UGameplayStatics::GetGameState(World));
}

EMF_HUDMode UMF_HUD::DetermineAppropriateMode() const
{
    AMF_PlayerController *PC = const_cast<UMF_HUD *>(this)->GetMFPlayerController();
    if (!PC)
    {
        return EMF_HUDMode::Spectator;
    }

    // Check if player is on a team
    EMF_TeamID CurrentTeam = PC->GetCurrentTeam();

    if (CurrentTeam == EMF_TeamID::None)
    {
        return EMF_HUDMode::Spectator;
    }
    else
    {
        return EMF_HUDMode::Gameplay;
    }
}
