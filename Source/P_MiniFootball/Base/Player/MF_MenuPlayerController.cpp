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

#include "InputBinding/FS_InputProfile.h"

#include "Input/MF_DefaultInputTemplates.h"

#include "EnhancedInputComponent.h"

AMF_MenuPlayerController::AMF_MenuPlayerController()
{
}

void AMF_MenuPlayerController::CreateInputComponent(TSubclassOf<UInputComponent> InputComponentToCreate)
{
    Super::CreateInputComponent(UEnhancedInputComponent::StaticClass());
}

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

        static const FName DefaultTemplateName(TEXT("Default"));

        // Menu maps can open settings before gameplay controllers ever run.
        // Ensure the built-in Default template exists on disk before attempting to apply it.
        if (!Manager->DoesTemplateExist(DefaultTemplateName))
        {
            const FS_InputProfile DefaultTemplate = MF_DefaultInputTemplates::BuildDefaultInputTemplate(DefaultTemplateName);
            Manager->SaveProfileTemplate(DefaultTemplateName, DefaultTemplate);
        }

        // Ensure we have at least the Default template loaded, so menu UI and settings can read bindings.
        if (FS_InputProfile *Profile = Manager->GetProfileRefForPlayer(this))
        {
            if (Profile->ActionBindings.Num() == 0 && Profile->AxisBindings.Num() == 0)
            {
                Manager->ApplyTemplateToPlayer(this, DefaultTemplateName);
            }
        }

        Manager->ApplyPlayerProfileToEnhancedInput(this);
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
