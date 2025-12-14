/*
 * @Author: Punal Manalan
 * @Description: MF_WidgetConfigurationSubsystem - Centralized, modular widget class resolution (settings + JSON + runtime overrides)
 * @Date: 14/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "UI/Configuration/MF_WidgetTypes.h"

#include "MF_WidgetConfigurationSubsystem.generated.h"

class UUserWidget;

/**
 * Global widget configuration subsystem.
 *
 * Resolution order:
 * 1) Runtime overrides (RegisterWidgetClass)
 * 2) Project settings (UMF_WidgetClassSettings)
 * 3) JSON config (optional)
 * 4) Native fallback classes (registered internally)
 */
UCLASS()
class P_MINIFOOTBALL_API UMF_WidgetConfigurationSubsystem : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase &Collection) override;
    virtual void Deinitialize() override;

    /** Get a widget class for a given widget type (may return null if nothing is configured). */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|WidgetConfig")
    TSubclassOf<UUserWidget> GetWidgetClass(EMF_WidgetType WidgetType);

    /** Get a widget class by string key (for Blueprint-driven dynamic widgets). */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|WidgetConfig")
    TSubclassOf<UUserWidget> GetWidgetClassByKey(const FString &WidgetKey);

    /** Register/override a widget class at runtime (highest priority). */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|WidgetConfig")
    void RegisterWidgetClass(EMF_WidgetType WidgetType, TSoftClassPtr<UUserWidget> WidgetClass);

    /** Register/override a widget class at runtime using a string key (highest priority). */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|WidgetConfig")
    void RegisterWidgetClassByKey(const FString &WidgetKey, TSoftClassPtr<UUserWidget> WidgetClass);

    /** Clear a runtime override. */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|WidgetConfig")
    void UnregisterWidgetClass(EMF_WidgetType WidgetType);

    /** Clear a runtime override using a string key. */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|WidgetConfig")
    void UnregisterWidgetClassByKey(const FString &WidgetKey);

    /** Load widget class overrides from JSON. Path is relative to project dir unless absolute. */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|WidgetConfig")
    bool LoadConfigurationFromJSON(const FString &ConfigPath);

    /** Save current overrides (runtime + settings-derived) into JSON. */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|WidgetConfig")
    bool SaveConfigurationToJSON(const FString &ConfigPath);

    /** Reload using the configured settings JsonConfigPath. */
    bool ReloadFromConfiguredPath();

private:
    TMap<EMF_WidgetType, TSoftClassPtr<UUserWidget>> RuntimeOverrides;
    TMap<EMF_WidgetType, FString> JsonOverrides;
    TMap<EMF_WidgetType, TSubclassOf<UUserWidget>> NativeDefaults;

    // Custom Blueprint keys (these are not constrained to EMF_WidgetType)
    TMap<FString, TSoftClassPtr<UUserWidget>> RuntimeOverridesByKey;
    TMap<FString, FString> JsonOverridesByKey;

    void InitializeNativeDefaults();

    TSubclassOf<UUserWidget> ResolveFromRuntime(const EMF_WidgetType WidgetType) const;
    TSubclassOf<UUserWidget> ResolveFromSettings(const EMF_WidgetType WidgetType) const;
    TSubclassOf<UUserWidget> ResolveFromJson(const EMF_WidgetType WidgetType) const;
    TSubclassOf<UUserWidget> ResolveFromNativeDefaults(const EMF_WidgetType WidgetType) const;

    TSubclassOf<UUserWidget> ResolveFromRuntimeByKey(const FString &WidgetKey) const;
    TSubclassOf<UUserWidget> ResolveFromJsonByKey(const FString &WidgetKey) const;
};
