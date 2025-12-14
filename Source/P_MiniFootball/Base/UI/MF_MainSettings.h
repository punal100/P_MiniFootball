/*
 * @Author: Punal Manalan
 * @Description: MF_MainSettings - Main settings overlay (Input/Audio/Graphics)
 * @Date: 14/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MF_MainSettings.generated.h"

class UButton;
class UMF_InputSettings;
class UMF_AudioSettings;
class UMF_GraphicsSettings;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMF_OnMainSettingsClosed);

UCLASS(Blueprintable, BlueprintType)
class P_MINIFOOTBALL_API UMF_MainSettings : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION(BlueprintCallable, Category = "MF|UI|Settings")
    void Show();

    UFUNCTION(BlueprintCallable, Category = "MF|UI|Settings")
    void Hide();

    UPROPERTY(BlueprintAssignable, Category = "MF|UI|Settings")
    FMF_OnMainSettingsClosed OnClosed;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MiniFootball|WidgetSpec")
    static FString GetWidgetSpec();

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> InputButton;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> AudioButton;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> GraphicsButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> BackButton;

    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|Settings")
    TSubclassOf<UMF_InputSettings> InputSettingsClass;

    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|Settings")
    TSubclassOf<UMF_AudioSettings> AudioSettingsClass;

    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|Settings")
    TSubclassOf<UMF_GraphicsSettings> GraphicsSettingsClass;

private:
    UFUNCTION()
    void HandleInputClicked();

    UFUNCTION()
    void HandleAudioClicked();

    UFUNCTION()
    void HandleGraphicsClicked();

    UFUNCTION()
    void HandleBackClicked();

    UFUNCTION()
    void HandleChildClosed();

    UPROPERTY(Transient)
    TObjectPtr<UMF_InputSettings> InputSettings;

    UPROPERTY(Transient)
    TObjectPtr<UMF_AudioSettings> AudioSettings;

    UPROPERTY(Transient)
    TObjectPtr<UMF_GraphicsSettings> GraphicsSettings;
};
