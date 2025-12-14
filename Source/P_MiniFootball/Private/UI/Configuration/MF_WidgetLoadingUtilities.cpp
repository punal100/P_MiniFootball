/*
 * @Author: Punal Manalan
 * @Description: MF_WidgetLoadingUtilities - Implementation
 * @Date: 14/12/2025
 */

#include "UI/Configuration/MF_WidgetLoadingUtilities.h"

#include "UI/Configuration/MF_WidgetConfigurationSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"

namespace
{
    static UMF_WidgetConfigurationSubsystem *GetWidgetConfig()
    {
        return GEngine ? GEngine->GetEngineSubsystem<UMF_WidgetConfigurationSubsystem>() : nullptr;
    }

    static UWorld *GetWorldFromContext(UObject *WorldContextObject)
    {
        return WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
    }

    static APlayerController *GetLocalPlayerController(UObject *WorldContextObject)
    {
        if (UWorld *World = GetWorldFromContext(WorldContextObject))
        {
            return World->GetFirstPlayerController();
        }
        return nullptr;
    }
}

TSubclassOf<UUserWidget> UMF_WidgetLoadingUtilities::ResolveWidgetClassByType(UObject *WorldContextObject, const EMF_WidgetType WidgetType)
{
    if (UMF_WidgetConfigurationSubsystem *Config = GetWidgetConfig())
    {
        return Config->GetWidgetClass(WidgetType);
    }

    return nullptr;
}

TSubclassOf<UUserWidget> UMF_WidgetLoadingUtilities::ResolveWidgetClassByKey(UObject *WorldContextObject, const FString &WidgetKey)
{
    if (UMF_WidgetConfigurationSubsystem *Config = GetWidgetConfig())
    {
        return Config->GetWidgetClassByKey(WidgetKey);
    }

    return nullptr;
}

UUserWidget *UMF_WidgetLoadingUtilities::CreateWidgetByType(UObject *WorldContextObject, const EMF_WidgetType WidgetType, const TSubclassOf<UUserWidget> OverrideClass)
{
    APlayerController *PC = GetLocalPlayerController(WorldContextObject);
    if (!PC)
    {
        return nullptr;
    }

    TSubclassOf<UUserWidget> ClassToCreate = OverrideClass;
    if (!ClassToCreate)
    {
        ClassToCreate = ResolveWidgetClassByType(WorldContextObject, WidgetType);
    }

    return ClassToCreate ? CreateWidget<UUserWidget>(PC, ClassToCreate) : nullptr;
}

UUserWidget *UMF_WidgetLoadingUtilities::CreateWidgetByKey(UObject *WorldContextObject, const FString &WidgetKey, const TSubclassOf<UUserWidget> OverrideClass)
{
    APlayerController *PC = GetLocalPlayerController(WorldContextObject);
    if (!PC)
    {
        return nullptr;
    }

    TSubclassOf<UUserWidget> ClassToCreate = OverrideClass;
    if (!ClassToCreate)
    {
        ClassToCreate = ResolveWidgetClassByKey(WorldContextObject, WidgetKey);
    }

    return ClassToCreate ? CreateWidget<UUserWidget>(PC, ClassToCreate) : nullptr;
}
