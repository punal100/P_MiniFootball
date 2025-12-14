/*
 * @Author: Punal Manalan
 * @Description: MF_GameplayControls - Container for active gameplay UI controls
 * @Date: 10/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MF_GameplayControls.generated.h"

// Forward declarations
class UMF_VirtualJoystick;
class UMF_ActionButton;
class UMF_ToggleActionButton;
class UWidgetSwitcher;
class UOverlay;
class AMF_PlayerController;

/**
 * UMF_GameplayControls
 * Container widget for mobile touch controls during active gameplay.
 * Manages virtual joystick, action buttons, and sprint functionality.
 *
 * Design Notes:
 * - Automatically hidden on non-touch platforms
 * - Relays control events to player controller
 * - Supports dynamic button visibility based on context
 */
UCLASS(Blueprintable, BlueprintType)
class P_MINIFOOTBALL_API UMF_GameplayControls : public UUserWidget
{
    GENERATED_BODY()

public:
    //~ Begin UUserWidget Interface
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry &MyGeometry, float InDeltaTime) override;
    //~ End UUserWidget Interface

    /**
     * Refreshes control layout based on current device.
     * Called automatically on construct, can be called manually after device changes.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|Controls")
    void RefreshControlLayout();

    /**
     * Enables or disables all controls.
     * Use during cutscenes, pauses, or when controls should be locked.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|Controls")
    void SetControlsEnabled(bool bEnabled);

    /**
     * Shows or hides sprint button based on gameplay context.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|Controls")
    void SetSprintButtonVisible(bool bVisible);

    /**
     * Updates action button context based on game state.
     */
    UFUNCTION(BlueprintCallable, Category = "MF|Controls")
    void UpdateActionContext();

protected:
    /** Virtual joystick for movement input */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "MF|Components")
    TObjectPtr<UMF_VirtualJoystick> MovementJoystick;

    /** Primary action button (kick, pass, etc.) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "MF|Components")
    TObjectPtr<UMF_ActionButton> ActionButton;

    /** Sprint button for running */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UMF_ToggleActionButton> SprintButton;

    /** Container for left-side controls (joystick) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UOverlay> LeftControlContainer;

    /** Container for right-side controls (buttons) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UOverlay> RightControlContainer;

    /** Whether controls are currently enabled */
    UPROPERTY(BlueprintReadOnly, Category = "MF|State")
    bool bControlsEnabled = true;

    /** Cached reference to owning player controller */
    UPROPERTY()
    TObjectPtr<AMF_PlayerController> CachedPlayerController;

public:
    // ==================== Widget Specification (JSON) ====================
    /**
     * Self-describing JSON specification for automated Widget Blueprint creation.
     * Consumed by P_MWCS (MWCS_CreateWidgets / MWCS_ValidateWidgets) to deterministically
     * generate/repair/validate WBP_MF_GameplayControls.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MiniFootball|WidgetSpec")
    static FString GetWidgetSpec();

private:
    /** Event handlers */
    UFUNCTION()
    void HandleJoystickMoved(FVector2D Direction);

    UFUNCTION()
    void HandleJoystickReleased();

    UFUNCTION()
    void HandleActionPressed();

    UFUNCTION()
    void HandleActionReleased(float HoldDuration = 0.0f);

    UFUNCTION()
    void HandleSprintStateChanged(bool bSprinting);

    /** Helper to get cached or fetch player controller */
    AMF_PlayerController *GetMFPlayerController();

    /** Checks if current device is touch-enabled */
    bool IsTouchDevice() const;

    /** Updates control visibility based on device type */
    void UpdateTouchVisibility();
};
