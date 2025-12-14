/*
 * @Author: Punal Manalan
 * @Description: MF_GraphicsSettings - Graphics settings overlay (stub)
 * @Date: 14/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MF_GraphicsSettings.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMF_OnGraphicsSettingsClosed);

UCLASS(Blueprintable, BlueprintType)
class P_MINIFOOTBALL_API UMF_GraphicsSettings : public UUserWidget
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
    FMF_OnGraphicsSettingsClosed OnClosed;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MiniFootball|WidgetSpec")
    static FString GetWidgetSpec();

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> BackButton;

private:
    UFUNCTION()
    void HandleBackClicked();
};
