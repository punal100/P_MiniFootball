/*
 * @Author: Punal Manalan
 * @Description: MF_TeamIndicator - Shows current player's team
 * @Date: 10/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/MF_Types.h"
#include "MF_TeamIndicator.generated.h"

// Forward declarations
class UTextBlock;
class UBorder;
class UImage;
class AMF_PlayerController;

/**
 * UMF_TeamIndicator - Current team display widget
 *
 * Shows the player's current team assignment:
 * - Team name/icon
 * - Team color background
 * - "SPECTATING" if not on a team
 */
UCLASS()
class P_MINIFOOTBALL_API UMF_TeamIndicator : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Update to show current team */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|TeamIndicator")
    void SetTeam(EMF_TeamID TeamID);

    /** Update to show spectating state */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|TeamIndicator")
    void SetSpectating();

    /** Refresh from PlayerController state */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|TeamIndicator")
    void RefreshFromController();

protected:
    virtual void NativeConstruct() override;

    // ==================== Widget Bindings ====================

    /** Background border for team color */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> TeamColorBorder;

    /** Team name/status text */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TeamText;

    /** Optional team icon */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> TeamIcon;

    // ==================== Configuration ====================

    /** Team A color */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|TeamIndicator")
    FLinearColor TeamAColor = FLinearColor(0.8f, 0.2f, 0.2f, 1.0f);

    /** Team B color */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|TeamIndicator")
    FLinearColor TeamBColor = FLinearColor(0.2f, 0.2f, 0.8f, 1.0f);

    /** Spectator color */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|TeamIndicator")
    FLinearColor SpectatorColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);

private:
    /** Get owning PlayerController */
    AMF_PlayerController *GetMFPlayerController() const;
};
