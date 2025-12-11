/*
 * @Author: Punal Manalan
 * @Description: MF_SprintButton - Sprint hold button for mobile
 * @Date: 10/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MF_SprintButton.generated.h"

// Forward declarations
class UButton;
class UImage;
class UTextBlock;

/** Delegate fired when sprint state changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMF_OnSprintStateChanged, bool, bIsSprinting);

/**
 * UMF_SprintButton - Sprint hold button widget
 *
 * Features:
 * - Hold to sprint
 * - Visual feedback while sprinting
 * - Sprint state delegate
 */
UCLASS()
class P_MINIFOOTBALL_API UMF_SprintButton : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Check if sprint is active */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|SprintButton")
    bool IsSprinting() const { return bIsSprinting; }

    /** Delegate fired when sprint state changes */
    UPROPERTY(BlueprintAssignable, Category = "MF|UI|SprintButton")
    FMF_OnSprintStateChanged OnSprintStateChanged;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** Handle button press */
    UFUNCTION()
    void HandleButtonPressed();

    /** Handle button release */
    UFUNCTION()
    void HandleButtonReleased();

    // ==================== Widget Bindings ====================

    /** Sprint button */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> SprintButton;

    /** Sprint icon */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> SprintIcon;

    /** Sprint label */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> SprintText;

    // ==================== Configuration ====================

    /** Color when sprinting */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|SprintButton")
    FLinearColor SprintingColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);

    /** Color when not sprinting */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|SprintButton")
    FLinearColor NormalColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

public:
    // ==================== Widget Specification (JSON) ====================
    /**
     * Self-describing JSON specification for automated Widget Blueprint creation.
     * Used by MF_WidgetBlueprintCreator.py to construct WBP_MF_SprintButton.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MiniFootball|WidgetSpec")
    static const FString &GetWidgetSpec();

private:
    /** Current sprint state */
    bool bIsSprinting = false;

    /** Update visual state */
    void UpdateVisualState();
};
