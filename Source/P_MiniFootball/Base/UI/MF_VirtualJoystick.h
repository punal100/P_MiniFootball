/*
 * @Author: Punal Manalan
 * @Description: MF_VirtualJoystick - Touch joystick for mobile movement
 * @Date: 10/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MF_VirtualJoystick.generated.h"

// Forward declarations
class UImage;
class UBorder;

/** Delegate fired when joystick moves */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMF_OnJoystickMoved, FVector2D, Direction);

/** Delegate fired when joystick is released */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMF_OnJoystickReleased);

/**
 * UMF_VirtualJoystick - Touch joystick widget for mobile
 *
 * Features:
 * - Touch-based input
 * - Visual joystick graphics (base + thumb)
 * - Dead zone handling
 * - Outputs normalized FVector2D direction
 */
UCLASS(Blueprintable, BlueprintType)
class P_MINIFOOTBALL_API UMF_VirtualJoystick : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Get current joystick direction (normalized) */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|VirtualJoystick")
    FVector2D GetJoystickDirection() const { return CurrentDirection; }

    /** Get current joystick magnitude (0-1) */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|VirtualJoystick")
    float GetJoystickMagnitude() const { return CurrentMagnitude; }

    /** Check if joystick is being touched */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|VirtualJoystick")
    bool IsPressed() const { return bIsPressed; }

    /** Delegate fired when joystick moves */
    UPROPERTY(BlueprintAssignable, Category = "MF|UI|VirtualJoystick")
    FMF_OnJoystickMoved OnJoystickMoved;

    /** Delegate fired when joystick is released */
    UPROPERTY(BlueprintAssignable, Category = "MF|UI|VirtualJoystick")
    FMF_OnJoystickReleased OnJoystickReleased;

protected:
    virtual void NativeConstruct() override;
    virtual FReply NativeOnTouchStarted(const FGeometry &InGeometry, const FPointerEvent &InGestureEvent) override;
    virtual FReply NativeOnTouchMoved(const FGeometry &InGeometry, const FPointerEvent &InGestureEvent) override;
    virtual FReply NativeOnTouchEnded(const FGeometry &InGeometry, const FPointerEvent &InGestureEvent) override;

    // ==================== Widget Bindings ====================

    /** Joystick base background */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> JoystickBase;

    /** Joystick thumb/handle */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> JoystickThumb;

    // ==================== Configuration ====================

    /** Dead zone radius (0-1) - input below this is ignored */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|VirtualJoystick")
    float DeadZone = 0.1f;

    /** Maximum distance thumb can move from center (in pixels) */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|VirtualJoystick")
    float MaxThumbOffset = 50.0f;

public:
    // ==================== Widget Specification (JSON) ====================
    /**
     * Self-describing JSON specification for automated Widget Blueprint creation.
     * Consumed by P_MWCS (MWCS_CreateWidgets / MWCS_ValidateWidgets) to deterministically
     * generate/repair/validate WBP_MF_VirtualJoystick.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MiniFootball|WidgetSpec")
    static FString GetWidgetSpec();

private:
    /** Current joystick direction (normalized) */
    FVector2D CurrentDirection = FVector2D::ZeroVector;

    /** Current joystick magnitude (0-1) */
    float CurrentMagnitude = 0.0f;

    /** Whether joystick is currently pressed */
    bool bIsPressed = false;

    /** Center position for calculations */
    FVector2D CenterPosition = FVector2D::ZeroVector;

    /** Update joystick position and output */
    void UpdateJoystickPosition(const FVector2D &TouchPosition, const FGeometry &Geometry);

    /** Reset joystick to center */
    void ResetJoystick();

    /** Update thumb visual position */
    void UpdateThumbVisual(const FVector2D &Offset);
};
