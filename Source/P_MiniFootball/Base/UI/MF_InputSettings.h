/*
 * @Author: Punal Manalan
 * @Description: MF_InputSettings - Modular input rebinding overlay (dynamic rows)
 * @Date: 14/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputBinding/FS_InputProfile.h"
#include "Types/SlateEnums.h"
#include "MF_InputSettings.generated.h"

class UButton;
class UComboBoxString;
class UScrollBox;
class UTextBlock;
class UVerticalBox;
class UMF_InputActionRow;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMF_OnInputSettingsClosed);

UCLASS(Blueprintable, BlueprintType)
class P_MINIFOOTBALL_API UMF_InputSettings : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry &InGeometry, const FKeyEvent &InKeyEvent) override;

    UFUNCTION(BlueprintCallable, Category = "MF|UI|Input")
    void Show();

    UFUNCTION(BlueprintCallable, Category = "MF|UI|Input")
    void Hide();

    UPROPERTY(BlueprintAssignable, Category = "MF|UI|Input")
    FMF_OnInputSettingsClosed OnClosed;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MiniFootball|WidgetSpec")
    static FString GetWidgetSpec();

    UFUNCTION(BlueprintCallable, Category = "MF|UI|Input")
    void PopulateProfileList();

    UFUNCTION(BlueprintCallable, Category = "MF|UI|Input")
    void LoadPreset(FName PresetName);

    UFUNCTION(BlueprintCallable, Category = "MF|UI|Input")
    void ResetToDefaults();

protected:
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> InputSettingsTitle;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UComboBoxString> ProfileSelector;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> ResetDefaultsButton;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> EmptyStateText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UScrollBox> ActionListScroll;

    /** Optional inner container for ActionListScroll (MWCS pattern to ensure proper sizing). */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UVerticalBox> ActionListContentBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> SaveButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> CancelButton;

    /** If set, Save will write to this template name (otherwise creates a new timestamped template). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MF|UI|Input")
    FName TargetTemplateName = NAME_None;

    /** Optional override for the action/axis row widget class. If unset, uses UMF_WidgetConfigurationSubsystem (InputActionRow). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MF|UI|Input")
    TSubclassOf<UMF_InputActionRow> InputActionRowClassOverride;

private:
    enum class ERebindMode : uint8
    {
        None,
        Action,
        Axis
    };

    void LoadProfileForEditing();
    void RebuildRows();

    FText MakeActionKeyDisplay(const TArray<FS_KeyBinding> &Keys) const;
    FText MakeAxisKeyDisplay(const TArray<FS_AxisKeyBinding> &Keys) const;

    bool IsActionToggleMode(const FName &ActionName) const;

    void BeginRebindAction(int32 ActionIndex);
    void BeginRebindAxis(int32 AxisIndex);
    void CancelRebind();
    void ApplyCapturedKey(const FKey &PressedKey);

    void SyncProfileSelectorToPlayer();

    UFUNCTION()
    void HandleRowRebindRequested(bool bIsAxisBinding, FName BindingName);

    UFUNCTION()
    void HandleProfileSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void HandleResetDefaultsClicked();

    UFUNCTION()
    void HandleSaveClicked();

    UFUNCTION()
    void HandleCancelClicked();

    // Editing state
    bool bHasPendingProfile = false;
    FS_InputProfile PendingProfile;

    bool bSuppressProfileSelectionChanged = false;

    ERebindMode RebindMode = ERebindMode::None;
    int32 PendingIndex = INDEX_NONE;

    UPROPERTY(Transient)
    TObjectPtr<UMF_InputActionRow> PendingRow;
};
