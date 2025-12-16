/*
 * @Author: Punal Manalan
 * @Description: MF_InputActionRow - Runtime-created input binding row (NOT MWCS)
 * @Date: 14/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputBinding/FS_InputActionBinding.h"
#include "InputBinding/FS_InputAxisBinding.h"
#include "InputCoreTypes.h"
#include "MF_InputActionRow.generated.h"

class UButton;
class UHorizontalBox;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMF_OnRebindRequested, bool, bIsAxisBinding, FName, BindingName);

UCLASS(BlueprintType)
class P_MINIFOOTBALL_API UMF_InputActionRow : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "MF|UI|Input")
    void SetActionBinding(const FS_InputActionBinding &InBinding, bool bIsToggleMode);

    UFUNCTION(BlueprintCallable, Category = "MF|UI|Input")
    void SetAxisBinding(const FS_InputAxisBinding &InBinding);

    /** Update the displayed key string (used while rebinding). */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|Input")
    void SetKeyDisplay(const FText &InKeyDisplay);

    /** Update the displayed mode string (e.g. "(Toggle)"). */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|Input")
    void SetModeDisplay(const FText &InModeDisplay);

    /** Highlight rebinding state. */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|Input")
    void SetRebinding(bool bInRebinding);

    /** Called when user clicks Rebind button (or otherwise initiates rebinding). */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|Input")
    virtual void StartRebinding();

    /** Called when a key is received during rebinding (UI-only helper; parent widget owns profile editing). */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|Input")
    virtual void OnInputReceived(FKey Key);

    /** Cancel rebinding. */
    UFUNCTION(BlueprintCallable, Category = "MF|UI|Input")
    virtual void CancelRebinding();

    UPROPERTY(BlueprintAssignable, Category = "MF|UI|Input")
    FMF_OnRebindRequested OnRebindRequested;

protected:
    virtual void NativeConstruct() override;

private:
    void BuildWidgetTreeIfNeeded();

    UFUNCTION()
    void HandleRebindClicked();

    // Runtime-created widget tree
    UPROPERTY(Transient)
    TObjectPtr<UHorizontalBox> RootRow;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> ActionLabel;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> ModeLabel;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> KeyLabel;

    UPROPERTY(Transient)
    TObjectPtr<UButton> RebindButton;

    // State
    bool bIsAxis = false;
    bool bIsRebinding = false;
    FName BindingName = NAME_None;
};
