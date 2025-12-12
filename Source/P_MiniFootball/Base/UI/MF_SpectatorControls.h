/*
 * @Author: Punal Manalan
 * @Description: MF_SpectatorControls - Spectator mode UI widget
 * @Date: 10/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/MF_Types.h"
#include "MF_SpectatorControls.generated.h"

// Forward declarations
class UTextBlock;
class UButton;
class UMF_QuickTeamPanel;
class AMF_GameState;
class AMF_PlayerController;

/** Delegate fired when team selection is requested */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMF_OnOpenTeamSelection);

/**
 * UMF_SpectatorControls - Spectator mode UI widget
 *
 * Shows when player is spectating:
 * - "SPECTATING" header
 * - Camera mode indicator
 * - Quick team panels (A and B)
 * - Control hints
 */
UCLASS(Blueprintable, BlueprintType)
class P_MINIFOOTBALL_API UMF_SpectatorControls : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Refresh team data from GameState */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|SpectatorControls")
    void RefreshTeamData();

    /** Update camera mode display */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|SpectatorControls")
    void UpdateCameraModeDisplay(bool bFollowingBall);

    /** Delegate for opening full team selection */
    UPROPERTY(BlueprintAssignable, Category = "MF|UI|SpectatorControls")
    FMF_OnOpenTeamSelection OnOpenTeamSelection;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** Handle quick join Team A */
    UFUNCTION()
    void HandleQuickJoinTeamA(EMF_TeamID TeamID);

    /** Handle quick join Team B */
    UFUNCTION()
    void HandleQuickJoinTeamB(EMF_TeamID TeamID);

    /** Handle open team selection button */
    UFUNCTION()
    void HandleOpenTeamSelectionClicked();

    // ==================== Widget Bindings ====================

    /** Spectating label */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> SpectatingLabel;

    /** Camera mode text */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> CameraModeText;

    /** Quick team A panel */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UMF_QuickTeamPanel> QuickTeamA;

    /** Quick team B panel */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UMF_QuickTeamPanel> QuickTeamB;

    /** Open team selection button */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> OpenTeamSelectButton;

    /** Control hints text */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> ControlHintsText;

public:
    // ==================== Widget Specification (JSON) ====================
    /**
     * Self-describing JSON specification for automated Widget Blueprint creation.
     * Used by MF_WidgetBlueprintCreator.py to construct WBP_MF_SpectatorControls.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MiniFootball|WidgetSpec")
    static FString GetWidgetSpec();

private:
    /** Get owning PlayerController */
    AMF_PlayerController *GetMFPlayerController() const;

    /** Get current GameState */
    AMF_GameState *GetGameState() const;

    /** Request join team via PlayerController */
    void RequestJoinTeam(EMF_TeamID TeamID);

    /** Update join button states based on balance rules */
    void UpdateJoinButtonStates();
};
