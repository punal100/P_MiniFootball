/*
 * @Author: Punal Manalan
 * @Description: MF_ScorePopup - Displays the latest goal scorer popup.
 * @Date: 12/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MF_ScorePopup.generated.h"

class UTextBlock;
class UImage;

/**
 * UMF_ScorePopup
 * Shows a transient message when a goal is scored.
 */
UCLASS(Blueprintable)
class P_MINIFOOTBALL_API UMF_ScorePopup : public UUserWidget
{
    GENERATED_BODY()

public:
    //~ Begin UUserWidget interface
    virtual void NativeConstruct() override;
    //~ End UUserWidget interface

    /** Update the displayed score text. */
    UFUNCTION(BlueprintCallable, Category = "MF|ScorePopup")
    void SetScoreText(const FText &InScoreText);

    /** Update the scorer name text shown below the score. */
    UFUNCTION(BlueprintCallable, Category = "MF|ScorePopup")
    void SetScorerName(const FText &InScorerName);

    /** Tint the background image (optional). */
    UFUNCTION(BlueprintCallable, Category = "MF|ScorePopup")
    void SetBackgroundTint(const FLinearColor &TintColor);

    /** Self-describing widget spec used by the automation script. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MiniFootball|WidgetSpec")
    static FString GetWidgetSpec();

protected:
    /** Score text block (optional binding). */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UTextBlock> ScoreText;

    /** Scorer name text block (optional binding). */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UTextBlock> ScorerNameText;

    /** Background image (optional binding). */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MF|Components")
    TObjectPtr<UImage> BackgroundImage;

private:
    /** Show or hide the widget based on whether text is present. */
    void UpdateVisibility();
};
