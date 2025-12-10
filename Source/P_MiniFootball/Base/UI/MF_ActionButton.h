/*
 * @Author: Punal Manalan
 * @Description: MF_ActionButton - Context-sensitive action button for mobile
 * @Date: 10/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MF_ActionButton.generated.h"

// Forward declarations
class UButton;
class UImage;
class UTextBlock;

/** Action context for button icon/behavior */
UENUM(BlueprintType)
enum class EMF_ActionContext : uint8
{
    Shoot UMETA(DisplayName = "Shoot"),
    Pass UMETA(DisplayName = "Pass"),
    Tackle UMETA(DisplayName = "Tackle"),
    None UMETA(DisplayName = "None")
};

/** Delegate fired when action button is pressed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMF_OnActionPressed);

/** Delegate fired when action button is released */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMF_OnActionReleased, float, HoldDuration);

/**
 * UMF_ActionButton - Context-sensitive action button widget
 *
 * Features:
 * - Press/Release events
 * - Hold duration tracking (for pass vs shoot)
 * - Context icon (shoot/pass/tackle)
 * - Visual feedback on press
 */
UCLASS()
class P_MINIFOOTBALL_API UMF_ActionButton : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Set action context (changes icon) */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|ActionButton")
    void SetActionContext(EMF_ActionContext Context);

    /** Get current action context */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|ActionButton")
    EMF_ActionContext GetActionContext() const { return CurrentContext; }

    /** Check if button is currently pressed */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|ActionButton")
    bool IsPressed() const { return bIsPressed; }

    /** Get current hold duration (if pressed) */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|ActionButton")
    float GetHoldDuration() const;

    /** Delegate fired when button is pressed */
    UPROPERTY(BlueprintAssignable, Category = "MF|UI|ActionButton")
    FMF_OnActionPressed OnActionPressed;

    /** Delegate fired when button is released */
    UPROPERTY(BlueprintAssignable, Category = "MF|UI|ActionButton")
    FMF_OnActionReleased OnActionReleased;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry &MyGeometry, float InDeltaTime) override;

    /** Handle button press */
    UFUNCTION()
    void HandleButtonPressed();

    /** Handle button release */
    UFUNCTION()
    void HandleButtonReleased();

    // ==================== Widget Bindings ====================

    /** Main button */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> ActionButton;

    /** Action icon */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> ActionIcon;

    /** Action text label */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> ActionText;

    // ==================== Configuration ====================

    /** Icon for shoot action */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|ActionButton")
    TObjectPtr<UTexture2D> ShootIcon;

    /** Icon for pass action */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|ActionButton")
    TObjectPtr<UTexture2D> PassIcon;

    /** Icon for tackle action */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|ActionButton")
    TObjectPtr<UTexture2D> TackleIcon;

    /** Pressed button color */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|ActionButton")
    FLinearColor PressedColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

    /** Normal button color */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|ActionButton")
    FLinearColor NormalColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

private:
    /** Current action context */
    EMF_ActionContext CurrentContext = EMF_ActionContext::Shoot;

    /** Whether button is currently pressed */
    bool bIsPressed = false;

    /** Time when button was pressed */
    float PressStartTime = 0.0f;

    /** Update icon based on context */
    void UpdateIcon();

    /** Update visual state (pressed/normal) */
    void UpdateVisualState();
};
