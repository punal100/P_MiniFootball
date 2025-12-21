/*
 * @Author: Punal Manalan
 * @Description: MF_MainSettings - Main settings overlay with WidgetSwitcher navigation
 * @Date: 14/12/2025
 * @Updated: 21/12/2025 - Added WidgetSwitcher for panel-based settings navigation
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MF_MainSettings.generated.h"

class UButton;
class UWidgetSwitcher;
class UVerticalBox;
class UMF_InputSettings;
class UMF_AudioSettings;
class UMF_GraphicsSettings;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMF_OnMainSettingsClosed);

/**
 * Settings panel indices for WidgetSwitcher
 */
UENUM(BlueprintType)
enum class EMF_SettingsPanel : uint8
{
    SettingsMenu = 0 UMETA(DisplayName = "Settings Menu"),
    InputSettings = 1 UMETA(DisplayName = "Input Settings"),
    AudioSettings = 2 UMETA(DisplayName = "Audio Settings"),
    GraphicsSettings = 3 UMETA(DisplayName = "Graphics Settings"),
    MAX UMETA(Hidden)
};

UCLASS(Blueprintable, BlueprintType)
class P_MINIFOOTBALL_API UMF_MainSettings : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // ==================== Visibility ====================

    UFUNCTION(BlueprintCallable, Category = "MF|UI|Settings")
    void Show();

    UFUNCTION(BlueprintCallable, Category = "MF|UI|Settings")
    void Hide();

    UPROPERTY(BlueprintAssignable, Category = "MF|UI|Settings")
    FMF_OnMainSettingsClosed OnClosed;

    // ==================== Panel Navigation ====================

    /** Switch to a specific settings panel */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|Settings")
    void SwitchToPanel(EMF_SettingsPanel Panel);

    /** Go back to settings menu */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|Settings")
    void GoBackToMenu() { SwitchToPanel(EMF_SettingsPanel::SettingsMenu); }

    /** Get current panel */
    UFUNCTION(BlueprintPure, Category = "MF|UI|Settings")
    EMF_SettingsPanel GetCurrentPanel() const { return CurrentPanel; }

    // ==================== Widget Spec ====================

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MiniFootball|WidgetSpec")
    static FString GetWidgetSpec();

protected:
    // ==================== Main Switcher ====================

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UWidgetSwitcher> SettingsSwitcher;

    // ==================== Panel 0: Settings Menu ====================

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UVerticalBox> SettingsMenuPanel;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> InputButton;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> AudioButton;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> GraphicsButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> BackButton;

    // ==================== Panel 1: Input Settings Container ====================

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UVerticalBox> InputSettingsPanel;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> InputBackButton;

    // ==================== Panel 2: Audio Settings Container ====================

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UVerticalBox> AudioSettingsPanel;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> AudioBackButton;

    // ==================== Panel 3: Graphics Settings Container ====================

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UVerticalBox> GraphicsSettingsPanel;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> GraphicsBackButton;

    // ==================== Configuration ====================

    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|Settings")
    TSubclassOf<UMF_InputSettings> InputSettingsClass;

    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|Settings")
    TSubclassOf<UMF_AudioSettings> AudioSettingsClass;

    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|Settings")
    TSubclassOf<UMF_GraphicsSettings> GraphicsSettingsClass;

private:
    // ==================== Button Handlers ====================

    UFUNCTION()
    void HandleInputClicked();

    UFUNCTION()
    void HandleAudioClicked();

    UFUNCTION()
    void HandleGraphicsClicked();

    UFUNCTION()
    void HandleBackClicked();

    UFUNCTION()
    void HandleInputBackClicked();

    UFUNCTION()
    void HandleAudioBackClicked();

    UFUNCTION()
    void HandleGraphicsBackClicked();

    // ==================== Helper Methods ====================

    void BindButtonEvents();
    void UnbindButtonEvents();
    void EnsureInputSettingsCreated();
    void EnsureAudioSettingsCreated();
    void EnsureGraphicsSettingsCreated();

    // ==================== State ====================

    EMF_SettingsPanel CurrentPanel = EMF_SettingsPanel::SettingsMenu;

    UPROPERTY(Transient)
    TObjectPtr<UMF_InputSettings> EmbeddedInputSettings;

    UPROPERTY(Transient)
    TObjectPtr<UMF_AudioSettings> EmbeddedAudioSettings;

    UPROPERTY(Transient)
    TObjectPtr<UMF_GraphicsSettings> EmbeddedGraphicsSettings;
};
