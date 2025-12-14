/*
 * @Author: Punal Manalan
 * @Description: MF_MenuPlayerController - UI-only controller for Entry/MainMenu maps
 * @Date: 14/12/2025
 */

#include "MF_MenuPlayerController.h"

#include "MF_MainMenu.h"

#include "UI/Configuration/MF_WidgetConfigurationSubsystem.h"
#include "UI/Configuration/MF_WidgetTypes.h"

#include "Engine/Engine.h"

#include "Manager/CPP_InputBindingManager.h"

namespace
{
    static UCPP_InputBindingManager *GetMEISManager()
    {
        return GEngine ? GEngine->GetEngineSubsystem<UCPP_InputBindingManager>() : nullptr;
    }

    static TSubclassOf<UMF_MainMenu> ResolveMainMenuClass(TSubclassOf<UMF_MainMenu> InClass)
    {
        if (InClass)
        {
            return InClass;
        }

        if (GEngine)
        {
            if (UMF_WidgetConfigurationSubsystem *WidgetConfig = GEngine->GetEngineSubsystem<UMF_WidgetConfigurationSubsystem>())
            {
                const TSubclassOf<UUserWidget> Resolved = WidgetConfig->GetWidgetClass(EMF_WidgetType::MainMenu);
                if (Resolved)
                {
                    return Resolved.Get();
                }
            }
        }

        return UMF_MainMenu::StaticClass();
    }
}

void AMF_MenuPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController())
    {
        return;
    }

    if (UCPP_InputBindingManager *Manager = GetMEISManager())
    {
        if (!Manager->HasPlayerRegistered(this))
        {
            Manager->RegisterPlayer(this);
        }
    }

    if (!MainMenu)
    {
        const TSubclassOf<UMF_MainMenu> ClassToCreate = ResolveMainMenuClass(MainMenuClass);
        MainMenu = CreateWidget<UMF_MainMenu>(this, ClassToCreate);
        if (MainMenu)
        {
            MainMenu->AddToViewport(MenuZOrder);
            MainMenu->RefreshState();
        }
    }

    if (MainMenu)
    {
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(MainMenu->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        SetInputMode(InputMode);
        bShowMouseCursor = true;
    }
}

void AMF_MenuPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (MainMenu)
    {
        MainMenu->RemoveFromParent();
        MainMenu = nullptr;
    }

    Super::EndPlay(EndPlayReason);
}
