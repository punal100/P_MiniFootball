/*
 * @Author: Punal Manalan
 * @Description: MF_ToggleActionButton - Generic hold/toggle button bound to a P_MEIS action
 * @Date: 14/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MF_ToggleActionButton.generated.h"

class UButton;
class UImage;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMF_OnToggleActionStateChanged, bool, bIsActive);

/**
 * UMF_ToggleActionButton
 * Generic UI button that emits a boolean state for an input action.
 *
 * Behavior:
 * - If ActionName is configured as toggle (in player's profile ToggleModeActions), Press toggles state and Release does nothing.
 * - Otherwise (hold), Press sets true and Release sets false.
 */
UCLASS(Blueprintable, BlueprintType)
class P_MINIFOOTBALL_API UMF_ToggleActionButton : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Action this button controls (must match P_MEIS profile binding name). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MF|UI|ToggleAction")
    FName ActionName = NAME_None;

    /** Current active state (toggle ON or hold currently pressed). */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|ToggleAction")
    bool IsActive() const
    {
        return bIsActive;
    }

    /** Fired when the state changes. */
    UPROPERTY(BlueprintAssignable, Category = "MF|UI|ToggleAction")
    FMF_OnToggleActionStateChanged OnStateChanged;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION()
    void HandleButtonPressed();

    UFUNCTION()
    void HandleButtonReleased();

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> ActionButton;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> ActionIcon;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> ActionText;

    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|ToggleAction")
    FLinearColor ActiveColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);

    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|ToggleAction")
    FLinearColor InactiveColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MiniFootball|WidgetSpec")
    static FString GetWidgetSpec();

private:
    bool bIsActive = false;
    bool bUseToggleMode = false;

    void RefreshToggleModeFromProfile();
    void RefreshActiveStateFromProfile();
    void SetActiveState(bool bNewActive, bool bBroadcast);
    void UpdateVisualState();
};
