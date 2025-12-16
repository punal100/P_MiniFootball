/*
 * @Author: Punal Manalan
 * @Description: MF_MenuPlayerController - UI-only controller for Entry/MainMenu maps
 * @Date: 14/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MF_MenuPlayerController.generated.h"

class UMF_MainMenu;

/**
 * Menu-only PlayerController.
 * Spawns WBP_MF_MainMenu and configures UI input mode.
 */
UCLASS()
class P_MINIFOOTBALL_API AMF_MenuPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AMF_MenuPlayerController();

protected:
    virtual void CreateInputComponent(TSubclassOf<UInputComponent> InputComponentToCreate) override;

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
    /** Menu widget class (prefer WBP_MF_MainMenu). */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|Menu")
    TSubclassOf<UMF_MainMenu> MainMenuClass;

    /** Z-order for the menu widget. */
    UPROPERTY(EditDefaultsOnly, Category = "MF|UI|Menu")
    int32 MenuZOrder = 1000;

private:
    UPROPERTY(Transient)
    TObjectPtr<UMF_MainMenu> MainMenu;
};
