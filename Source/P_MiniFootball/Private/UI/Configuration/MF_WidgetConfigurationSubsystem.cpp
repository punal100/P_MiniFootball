/*
 * @Author: Punal Manalan
 * @Description: MF_WidgetConfigurationSubsystem - Implementation
 * @Date: 14/12/2025
 */

#include "UI/Configuration/MF_WidgetConfigurationSubsystem.h"

#include "Settings/MF_WidgetClassSettings.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

// Native fallbacks
#include "UI/MF_ActionButton.h"
#include "UI/MF_MainMenu.h"
#include "UI/MF_MainSettings.h"
#include "UI/MF_PauseMenu.h"
#include "UI/MF_HUD.h"
#include "UI/MF_MatchInfo.h"
#include "UI/MF_TeamIndicator.h"
#include "UI/MF_GameplayControls.h"
#include "UI/MF_SpectatorControls.h"
#include "UI/MF_TeamSelectionPopup.h"
#include "UI/MF_TransitionOverlay.h"
#include "UI/MF_ScorePopup.h"
#include "UI/MF_VirtualJoystick.h"
#include "UI/MF_ToggleActionButton.h"
#include "UI/MF_TeamPanel.h"
#include "UI/MF_QuickTeamPanel.h"
#include "UI/MF_InputActionRow.h"
#include "UI/MF_InputSettings.h"
#include "UI/MF_AudioSettings.h"
#include "UI/MF_GraphicsSettings.h"

namespace
{
    static FString NormalizeConfigPath(const FString &InPath)
    {
        if (InPath.IsEmpty())
        {
            return FString();
        }

        if (FPaths::IsRelative(InPath))
        {
            return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / InPath);
        }

        return InPath;
    }

    static void ReloadWidgetConfigFromJson()
    {
        if (!GEngine)
        {
            return;
        }

        if (UMF_WidgetConfigurationSubsystem *WidgetConfig = GEngine->GetEngineSubsystem<UMF_WidgetConfigurationSubsystem>())
        {
            WidgetConfig->ReloadFromConfiguredPath();
        }
    }

    static FAutoConsoleCommand CCmdReloadWidgetConfig(
        TEXT("MF.WidgetConfig.Reload"),
        TEXT("Reload P_MiniFootball widget class overrides from the configured JSON path (UMF_WidgetClassSettings.JsonConfigPath)."),
        FConsoleCommandDelegate::CreateStatic(&ReloadWidgetConfigFromJson));
}

void UMF_WidgetConfigurationSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
    Super::Initialize(Collection);

    InitializeNativeDefaults();

    const UMF_WidgetClassSettings *Settings = GetDefault<UMF_WidgetClassSettings>();
    if (Settings && Settings->bAutoLoadJsonConfig)
    {
        ReloadFromConfiguredPath();
    }
}

void UMF_WidgetConfigurationSubsystem::Deinitialize()
{
    RuntimeOverrides.Empty();
    JsonOverrides.Empty();
    NativeDefaults.Empty();
    RuntimeOverridesByKey.Empty();
    JsonOverridesByKey.Empty();

    Super::Deinitialize();
}

void UMF_WidgetConfigurationSubsystem::InitializeNativeDefaults()
{
    NativeDefaults.Empty();

    NativeDefaults.Add(EMF_WidgetType::MainHUD, UMF_HUD::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::HUD, UMF_HUD::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::MainMenu, UMF_MainMenu::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::PauseMenu, UMF_PauseMenu::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::MainSettings, UMF_MainSettings::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::InputSettings, UMF_InputSettings::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::AudioSettings, UMF_AudioSettings::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::GraphicsSettings, UMF_GraphicsSettings::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::MatchInfo, UMF_MatchInfo::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::TeamIndicator, UMF_TeamIndicator::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::GameplayControls, UMF_GameplayControls::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::SpectatorControls, UMF_SpectatorControls::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::TeamSelectionPopup, UMF_TeamSelectionPopup::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::TransitionOverlay, UMF_TransitionOverlay::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::ScorePopup, UMF_ScorePopup::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::VirtualJoystick, UMF_VirtualJoystick::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::ActionButton, UMF_ActionButton::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::ToggleActionButton, UMF_ToggleActionButton::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::TeamPanel, UMF_TeamPanel::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::QuickTeamPanel, UMF_QuickTeamPanel::StaticClass());
    NativeDefaults.Add(EMF_WidgetType::InputActionRow, UMF_InputActionRow::StaticClass());
}

TSubclassOf<UUserWidget> UMF_WidgetConfigurationSubsystem::GetWidgetClass(const EMF_WidgetType WidgetType)
{
    if (WidgetType == EMF_WidgetType::Unknown || WidgetType == EMF_WidgetType::CustomByString)
    {
        return nullptr;
    }

    if (TSubclassOf<UUserWidget> C = ResolveFromRuntime(WidgetType))
    {
        return C;
    }

    if (TSubclassOf<UUserWidget> C = ResolveFromSettings(WidgetType))
    {
        return C;
    }

    if (TSubclassOf<UUserWidget> C = ResolveFromJson(WidgetType))
    {
        return C;
    }

    return ResolveFromNativeDefaults(WidgetType);
}

TSubclassOf<UUserWidget> UMF_WidgetConfigurationSubsystem::GetWidgetClassByKey(const FString &WidgetKey)
{
    const FString Key = WidgetKey.TrimStartAndEnd();
    if (Key.IsEmpty())
    {
        return nullptr;
    }

    if (TSubclassOf<UUserWidget> C = ResolveFromRuntimeByKey(Key))
    {
        return C;
    }

    if (TSubclassOf<UUserWidget> C = ResolveFromJsonByKey(Key))
    {
        return C;
    }

    return nullptr;
}

void UMF_WidgetConfigurationSubsystem::RegisterWidgetClass(const EMF_WidgetType WidgetType, const TSoftClassPtr<UUserWidget> WidgetClass)
{
    if (WidgetType == EMF_WidgetType::Unknown || WidgetType == EMF_WidgetType::CustomByString)
    {
        return;
    }

    RuntimeOverrides.Add(WidgetType, WidgetClass);
}

void UMF_WidgetConfigurationSubsystem::RegisterWidgetClassByKey(const FString &WidgetKey, const TSoftClassPtr<UUserWidget> WidgetClass)
{
    const FString Key = WidgetKey.TrimStartAndEnd();
    if (Key.IsEmpty())
    {
        return;
    }

    RuntimeOverridesByKey.Add(Key, WidgetClass);
}

void UMF_WidgetConfigurationSubsystem::UnregisterWidgetClass(const EMF_WidgetType WidgetType)
{
    RuntimeOverrides.Remove(WidgetType);
}

void UMF_WidgetConfigurationSubsystem::UnregisterWidgetClassByKey(const FString &WidgetKey)
{
    const FString Key = WidgetKey.TrimStartAndEnd();
    if (Key.IsEmpty())
    {
        return;
    }

    RuntimeOverridesByKey.Remove(Key);
}

TSubclassOf<UUserWidget> UMF_WidgetConfigurationSubsystem::ResolveFromRuntime(const EMF_WidgetType WidgetType) const
{
    if (const TSoftClassPtr<UUserWidget> *Found = RuntimeOverrides.Find(WidgetType))
    {
        if (!Found->IsNull())
        {
            if (UClass *Loaded = Found->LoadSynchronous())
            {
                return Loaded;
            }
        }
    }

    return nullptr;
}

TSubclassOf<UUserWidget> UMF_WidgetConfigurationSubsystem::ResolveFromSettings(const EMF_WidgetType WidgetType) const
{
    const UMF_WidgetClassSettings *Settings = GetDefault<UMF_WidgetClassSettings>();
    if (!Settings)
    {
        return nullptr;
    }

    switch (WidgetType)
    {
    case EMF_WidgetType::MainHUD:
    case EMF_WidgetType::HUD:
        return Settings->MainHUDClass.IsNull() ? nullptr : Settings->MainHUDClass.LoadSynchronous();
    case EMF_WidgetType::MainMenu:
        return Settings->MainMenuClass.IsNull() ? nullptr : Settings->MainMenuClass.LoadSynchronous();
    case EMF_WidgetType::PauseMenu:
        return Settings->PauseMenuClass.IsNull() ? nullptr : Settings->PauseMenuClass.LoadSynchronous();
    case EMF_WidgetType::MainSettings:
        return Settings->MainSettingsClass.IsNull() ? nullptr : Settings->MainSettingsClass.LoadSynchronous();
    case EMF_WidgetType::InputSettings:
        return Settings->InputSettingsClass.IsNull() ? nullptr : Settings->InputSettingsClass.LoadSynchronous();
    case EMF_WidgetType::AudioSettings:
        return Settings->AudioSettingsClass.IsNull() ? nullptr : Settings->AudioSettingsClass.LoadSynchronous();
    case EMF_WidgetType::GraphicsSettings:
        return Settings->GraphicsSettingsClass.IsNull() ? nullptr : Settings->GraphicsSettingsClass.LoadSynchronous();
    case EMF_WidgetType::MatchInfo:
        return Settings->MatchInfoClass.IsNull() ? nullptr : Settings->MatchInfoClass.LoadSynchronous();
    case EMF_WidgetType::TeamIndicator:
        return Settings->TeamIndicatorClass.IsNull() ? nullptr : Settings->TeamIndicatorClass.LoadSynchronous();
    case EMF_WidgetType::GameplayControls:
        return Settings->GameplayControlsClass.IsNull() ? nullptr : Settings->GameplayControlsClass.LoadSynchronous();
    case EMF_WidgetType::SpectatorControls:
        return Settings->SpectatorControlsClass.IsNull() ? nullptr : Settings->SpectatorControlsClass.LoadSynchronous();
    case EMF_WidgetType::TeamSelectionPopup:
        return Settings->TeamSelectionPopupClass.IsNull() ? nullptr : Settings->TeamSelectionPopupClass.LoadSynchronous();
    case EMF_WidgetType::TransitionOverlay:
        return Settings->TransitionOverlayClass.IsNull() ? nullptr : Settings->TransitionOverlayClass.LoadSynchronous();
    case EMF_WidgetType::ScorePopup:
        return Settings->ScorePopupClass.IsNull() ? nullptr : Settings->ScorePopupClass.LoadSynchronous();
    case EMF_WidgetType::VirtualJoystick:
        return Settings->VirtualJoystickClass.IsNull() ? nullptr : Settings->VirtualJoystickClass.LoadSynchronous();
    case EMF_WidgetType::ActionButton:
        return Settings->ActionButtonClass.IsNull() ? nullptr : Settings->ActionButtonClass.LoadSynchronous();
    case EMF_WidgetType::ToggleActionButton:
        return Settings->ToggleActionButtonClass.IsNull() ? nullptr : Settings->ToggleActionButtonClass.LoadSynchronous();
    case EMF_WidgetType::TeamPanel:
        return Settings->TeamPanelClass.IsNull() ? nullptr : Settings->TeamPanelClass.LoadSynchronous();
    case EMF_WidgetType::QuickTeamPanel:
        return Settings->QuickTeamPanelClass.IsNull() ? nullptr : Settings->QuickTeamPanelClass.LoadSynchronous();
    case EMF_WidgetType::InputActionRow:
        return Settings->InputActionRowClass.IsNull() ? nullptr : Settings->InputActionRowClass.LoadSynchronous();
    default:
        return nullptr;
    }
}

TSubclassOf<UUserWidget> UMF_WidgetConfigurationSubsystem::ResolveFromJson(const EMF_WidgetType WidgetType) const
{
    if (const FString *Path = JsonOverrides.Find(WidgetType))
    {
        if (!Path->IsEmpty())
        {
            const FSoftClassPath SoftPath(*Path);
            if (UClass *Loaded = SoftPath.TryLoadClass<UUserWidget>())
            {
                return Loaded;
            }

            UE_LOG(LogTemp, Warning, TEXT("MF_WidgetConfig: Failed to load JSON widget class '%s' for %s"),
                   **Path,
                   *MF_WidgetTypes::ToKey(WidgetType));
        }
    }

    return nullptr;
}

TSubclassOf<UUserWidget> UMF_WidgetConfigurationSubsystem::ResolveFromNativeDefaults(const EMF_WidgetType WidgetType) const
{
    if (const TSubclassOf<UUserWidget> *Found = NativeDefaults.Find(WidgetType))
    {
        return *Found;
    }

    return nullptr;
}

TSubclassOf<UUserWidget> UMF_WidgetConfigurationSubsystem::ResolveFromRuntimeByKey(const FString &WidgetKey) const
{
    if (const TSoftClassPtr<UUserWidget> *Found = RuntimeOverridesByKey.Find(WidgetKey))
    {
        if (!Found->IsNull())
        {
            if (UClass *Loaded = Found->LoadSynchronous())
            {
                return Loaded;
            }
        }
    }

    return nullptr;
}

TSubclassOf<UUserWidget> UMF_WidgetConfigurationSubsystem::ResolveFromJsonByKey(const FString &WidgetKey) const
{
    if (const FString *Path = JsonOverridesByKey.Find(WidgetKey))
    {
        if (!Path->IsEmpty())
        {
            const FSoftClassPath SoftPath(*Path);
            if (UClass *Loaded = SoftPath.TryLoadClass<UUserWidget>())
            {
                return Loaded;
            }

            UE_LOG(LogTemp, Warning, TEXT("MF_WidgetConfig: Failed to load JSON widget class '%s' for key '%s'"), **Path, *WidgetKey);
        }
    }

    return nullptr;
}

bool UMF_WidgetConfigurationSubsystem::LoadConfigurationFromJSON(const FString &ConfigPath)
{
    const FString AbsPath = NormalizeConfigPath(ConfigPath);
    if (AbsPath.IsEmpty() || !IFileManager::Get().FileExists(*AbsPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_WidgetConfig: JSON config not found: %s"), *AbsPath);
        return false;
    }

    FString JsonText;
    if (!FFileHelper::LoadFileToString(JsonText, *AbsPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_WidgetConfig: Failed reading JSON config: %s"), *AbsPath);
        return false;
    }

    TSharedPtr<FJsonObject> Root;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_WidgetConfig: Invalid JSON: %s"), *AbsPath);
        return false;
    }

    const TSharedPtr<FJsonObject> *WidgetClassesObj = nullptr;
    if (!Root->TryGetObjectField(TEXT("WidgetClasses"), WidgetClassesObj) || !WidgetClassesObj || !WidgetClassesObj->IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_WidgetConfig: JSON missing 'WidgetClasses' object: %s"), *AbsPath);
        return false;
    }

    JsonOverrides.Empty();
    JsonOverridesByKey.Empty();

    for (const auto &Pair : (*WidgetClassesObj)->Values)
    {
        const EMF_WidgetType Type = MF_WidgetTypes::FromKey(Pair.Key);
        FString ClassPath;
        if (Pair.Value.IsValid() && Pair.Value->TryGetString(ClassPath) && !ClassPath.IsEmpty())
        {
            if (Type == EMF_WidgetType::Unknown || Type == EMF_WidgetType::CustomByString)
            {
                // Treat unknown keys as dynamic Blueprint keys
                JsonOverridesByKey.Add(Pair.Key, ClassPath);
            }
            else
            {
                JsonOverrides.Add(Type, ClassPath);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("MF_WidgetConfig: Loaded %d enum widget overrides + %d key widget overrides from %s"),
           JsonOverrides.Num(),
           JsonOverridesByKey.Num(),
           *AbsPath);
    return true;
}

bool UMF_WidgetConfigurationSubsystem::SaveConfigurationToJSON(const FString &ConfigPath)
{
    const FString AbsPath = NormalizeConfigPath(ConfigPath);
    if (AbsPath.IsEmpty())
    {
        return false;
    }

    TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
    Root->SetNumberField(TEXT("Version"), 1);

    TSharedRef<FJsonObject> WidgetClassesObj = MakeShared<FJsonObject>();

    // Prefer runtime overrides (enum)
    for (const auto &Pair : RuntimeOverrides)
    {
        if (!Pair.Value.IsNull())
        {
            const FString Path = Pair.Value.ToSoftObjectPath().ToString();
            if (!Path.IsEmpty())
            {
                WidgetClassesObj->SetStringField(MF_WidgetTypes::ToKey(Pair.Key), Path);
            }
        }
    }

    // Prefer runtime overrides (key)
    for (const auto &Pair : RuntimeOverridesByKey)
    {
        if (!Pair.Value.IsNull())
        {
            const FString Path = Pair.Value.ToSoftObjectPath().ToString();
            if (!Path.IsEmpty())
            {
                WidgetClassesObj->SetStringField(Pair.Key, Path);
            }
        }
    }

    // Fill any missing enum entries from loaded JSON overrides
    for (const auto &Pair : JsonOverrides)
    {
        const FString Key = MF_WidgetTypes::ToKey(Pair.Key);
        if (!WidgetClassesObj->HasField(Key))
        {
            WidgetClassesObj->SetStringField(Key, Pair.Value);
        }
    }

    // Fill any missing key entries from loaded JSON overrides
    for (const auto &Pair : JsonOverridesByKey)
    {
        if (!WidgetClassesObj->HasField(Pair.Key))
        {
            WidgetClassesObj->SetStringField(Pair.Key, Pair.Value);
        }
    }

    Root->SetObjectField(TEXT("WidgetClasses"), WidgetClassesObj);

    FString OutText;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutText);
    if (!FJsonSerializer::Serialize(Root, Writer))
    {
        return false;
    }

    if (!FFileHelper::SaveStringToFile(OutText, *AbsPath))
    {
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("MF_WidgetConfig: Wrote JSON config to %s"), *AbsPath);
    return true;
}

bool UMF_WidgetConfigurationSubsystem::ReloadFromConfiguredPath()
{
    const UMF_WidgetClassSettings *Settings = GetDefault<UMF_WidgetClassSettings>();
    if (!Settings)
    {
        return false;
    }

    return LoadConfigurationFromJSON(Settings->JsonConfigPath);
}
