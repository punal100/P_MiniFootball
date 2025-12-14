/*
 * @Author: Punal Manalan
 * @Description: MF_MainMenu - Main menu root widget (New Game / Continue / Settings / Quit)
 * @Date: 14/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MF_MainMenu.generated.h"

class UButton;
class UTextBlock;
class UMF_MainSettings;

/**
 * UMF_MainMenu
 * Root main menu widget. Uses P_MEIS templates for Continue availability.
 */
UCLASS(Blueprintable, BlueprintType)
class P_MINIFOOTBALL_API UMF_MainMenu : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** Refresh button states based on available profiles/templates. */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|MainMenu")
    void RefreshState();

    /** MWCS widget specification. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MiniFootball|WidgetSpec")
    static FString GetWidgetSpec();

protected:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UButton> NewGameButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UButton> ContinueButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UButton> SettingsButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UButton> QuitButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> VersionText;

    /** Overlay settings widget class (prefer WBP_MF_MainSettings). */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|MainMenu")
    TSubclassOf<UMF_MainSettings> MainSettingsClass;

    /** Level name to open for gameplay. */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|MainMenu")
    FName GameplayLevelName = FName(TEXT("MF_MatchArena"));

private:
    UFUNCTION()
    void HandleNewGameClicked();

    UFUNCTION()
    void HandleContinueClicked();

    UFUNCTION()
    void HandleSettingsClicked();

    UFUNCTION()
    void HandleQuitClicked();

    UFUNCTION()
    void HandleSettingsClosed();

    bool GetMostRecentTemplateName(FName &OutTemplateName) const;

    bool GetMostRecentTemplateInfo(FName &OutTemplateName, FDateTime &OutTimestamp) const;

    void SetContinueEnabled(bool bEnabled);

    bool bPendingLaunchAfterSettings = false;
    FDateTime NewGameBaselineTemplateTime = FDateTime::MinValue();

    UPROPERTY(Transient)
    TObjectPtr<UMF_MainSettings> MainSettings;
};
