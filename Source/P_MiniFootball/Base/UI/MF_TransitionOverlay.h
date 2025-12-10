/*
 * @Author: Punal Manalan
 * @Description: MF_TransitionOverlay - Loading/transition screen widget
 * @Date: 10/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MF_TransitionOverlay.generated.h"

// Forward declarations
class UTextBlock;
class UImage;
class UThrobber;

/**
 * UMF_TransitionOverlay - Loading/transition screen widget
 *
 * Shows during state transitions:
 * - "Joining Team..." when joining a team
 * - "Leaving Team..." when leaving
 * - Loading animation
 * - Fade in/out support
 */
UCLASS()
class P_MINIFOOTBALL_API UMF_TransitionOverlay : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Set status text */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|TransitionOverlay")
    void SetStatusText(const FString &StatusText);

    /** Show the overlay with animation */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|TransitionOverlay")
    void ShowOverlay();

    /** Hide the overlay with animation */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|TransitionOverlay")
    void HideOverlay();

    /** Show with specific message */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|TransitionOverlay")
    void ShowWithMessage(const FString &Message);

protected:
    virtual void NativeConstruct() override;

    // ==================== Widget Bindings ====================

    /** Status message text */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> StatusText;

    /** Loading throbber/spinner */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UThrobber> LoadingThrobber;

    /** Background overlay */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> BackgroundOverlay;

    // ==================== Configuration ====================

    /** Fade duration for show/hide */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|TransitionOverlay")
    float FadeDuration = 0.3f;

    /** Default status message */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|TransitionOverlay")
    FString DefaultStatusMessage = TEXT("Loading...");

private:
    /** Animation helper */
    void PlayFadeAnimation(bool bFadeIn);
};
