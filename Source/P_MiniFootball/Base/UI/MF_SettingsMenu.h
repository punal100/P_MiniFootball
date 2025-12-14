/*
 * @Author: Punal Manalan
 * @Description: MF_SettingsMenu - Settings menu with Input configuration
 * @Date: 14/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ComboBoxString.h"
#include "InputBinding/FS_InputProfile.h"
#include "MF_SettingsMenu.generated.h"

class UButton;
class UVerticalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMF_OnSettingsClosed);

/**
 * UMF_SettingsMenu
 * Settings menu that currently exposes Input configuration:
 * - Sprint Mode (Hold vs Toggle) stored in the player's P_MEIS profile
 */
UCLASS(Blueprintable, BlueprintType)
class P_MINIFOOTBALL_API UMF_SettingsMenu : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry &InGeometry, const FKeyEvent &InKeyEvent) override;

    UPROPERTY(BlueprintAssignable, Category = "MF|UI|Settings")
    FMF_OnSettingsClosed OnSettingsClosed;

    // ==================== Widget Specification (JSON) ====================
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MiniFootball|WidgetSpec")
    static FString GetWidgetSpec();

private:
    enum class ERebindMode : uint8
    {
        None,
        ActionSingle,
        MoveWASD
    };

    // ==================== Bindings ====================

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UComboBoxString> SprintModeCombo;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UVerticalBox> ActionBindingsList;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> SaveButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> CancelButton;

    // ==================== State ====================

    bool bLoadedSprintToggleMode = false;
    bool bPendingSprintToggleMode = false;

    bool bHasPendingProfile = false;
    FS_InputProfile PendingProfile;

    ERebindMode RebindMode = ERebindMode::None;
    FName PendingRebindName = NAME_None;
    int32 PendingMoveStep = 0;

    // ==================== Handlers ====================

    UFUNCTION()
    void HandleSprintModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void HandleSaveSettings();

    UFUNCTION()
    void HandleCancelSettings();

    UFUNCTION()
    void HandleRebindMoveClicked();

    UFUNCTION()
    void HandleRebindActionClicked();

    UFUNCTION()
    void HandleRebindSprintClicked();

    UFUNCTION()
    void HandleRebindSwitchPlayerClicked();

    UFUNCTION()
    void HandleRebindPauseClicked();

    // ==================== Helpers ====================

    void RefreshFromProfile();
    void RefreshSprintMode();
    void PopulateActionList();

    void LoadProfileForEditing();
    void BeginRebindAction(const FName &ActionName);
    void BeginRebindMove();
    void CancelRebind();
    void ApplyCapturedKeyToPendingProfile(const FKeyEvent &InKeyEvent);
};
