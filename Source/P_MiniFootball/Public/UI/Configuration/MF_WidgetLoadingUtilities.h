/*
 * @Author: Punal Manalan
 * @Description: MF_WidgetLoadingUtilities - Blueprint helpers for creating/resolving widgets via UMF_WidgetConfigurationSubsystem
 * @Date: 14/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UI/Configuration/MF_WidgetTypes.h"

#include "MF_WidgetLoadingUtilities.generated.h"

class UUserWidget;

UCLASS()
class P_MINIFOOTBALL_API UMF_WidgetLoadingUtilities : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Resolve a widget class by enum via the widget config subsystem. */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|WidgetConfig", meta = (WorldContext = "WorldContextObject"))
    static TSubclassOf<UUserWidget> ResolveWidgetClassByType(UObject *WorldContextObject, EMF_WidgetType WidgetType);

    /** Resolve a widget class by string key via the widget config subsystem. */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|WidgetConfig", meta = (WorldContext = "WorldContextObject"))
    static TSubclassOf<UUserWidget> ResolveWidgetClassByKey(UObject *WorldContextObject, const FString &WidgetKey);

    /** Create a widget by enum type (optionally overriding class). */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|WidgetConfig", meta = (WorldContext = "WorldContextObject"))
    static UUserWidget *CreateWidgetByType(UObject *WorldContextObject, EMF_WidgetType WidgetType, TSubclassOf<UUserWidget> OverrideClass);

    /** Create a widget by string key (optionally overriding class). */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|WidgetConfig", meta = (WorldContext = "WorldContextObject"))
    static UUserWidget *CreateWidgetByKey(UObject *WorldContextObject, const FString &WidgetKey, TSubclassOf<UUserWidget> OverrideClass);
};
