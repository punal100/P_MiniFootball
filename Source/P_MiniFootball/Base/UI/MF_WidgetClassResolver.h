/*
 * @Author: Punal Manalan
 * @Description: MF_WidgetClassResolver - Small helper to resolve widget classes from explicit property or config settings.
 * @Date: 14/12/2025
 */

#pragma once

#include "CoreMinimal.h"

namespace MF_WidgetClassResolver
{
    template <typename TWidget>
    TSubclassOf<TWidget> Resolve(
        const TSubclassOf<TWidget> ExplicitClass,
        const TSoftClassPtr<TWidget> ConfigClass,
        const TSubclassOf<TWidget> NativeFallbackClass)
    {
        if (ExplicitClass)
        {
            return ExplicitClass;
        }

        if (!ConfigClass.IsNull())
        {
            if (UClass *Loaded = ConfigClass.LoadSynchronous())
            {
                return Loaded;
            }

            UE_LOG(LogTemp, Warning, TEXT("MF_WidgetClassResolver: Failed to load configured widget class '%s'"),
                   *ConfigClass.ToSoftObjectPath().ToString());
        }

        return NativeFallbackClass;
    }

    template <typename TWidget>
    TSubclassOf<TWidget> Resolve(
        const TSubclassOf<TWidget> ExplicitClass,
        const TSoftClassPtr<TWidget> ConfigClass)
    {
        return Resolve<TWidget>(ExplicitClass, ConfigClass, TWidget::StaticClass());
    }
}
