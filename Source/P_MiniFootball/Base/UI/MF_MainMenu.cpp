/*
 * @Author: Punal Manalan
 * @Description: MF_MainMenu - Main menu root widget implementation
 * @Date: 14/12/2025
 */

#include "MF_MainMenu.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

#include "Manager/CPP_InputBindingManager.h"
#include "MF_MainSettings.h"

#include "UI/Configuration/MF_WidgetConfigurationSubsystem.h"
#include "UI/Configuration/MF_WidgetTypes.h"

namespace
{
    static UCPP_InputBindingManager *GetMEISManager()
    {
        return GEngine ? GEngine->GetEngineSubsystem<UCPP_InputBindingManager>() : nullptr;
    }
}

FString UMF_MainMenu::GetWidgetSpec()
{
    static FString Spec = R"JSON({
    "WidgetClass": "UMF_MainMenu",
    "BlueprintName": "WBP_MF_MainMenu",
    "ParentClass": "/Script/P_MiniFootball.MF_MainMenu",
    "Category": "MF|UI|Menus",
    "Description": "Main menu widget (New Game / Continue / Settings / Quit)",
    "Version": "1.0.0",

    "DesignerPreview": {
        "SizeMode": "FillScreen",
        "ZoomLevel": 14,
        "ShowGrid": true
    },

    "Hierarchy": {
        "Root": {
            "Type": "CanvasPanel",
            "Name": "RootCanvas",
            "Children": [
                {
                    "Type": "VerticalBox",
                    "Name": "MenuContainer",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0.5, "Y": 0.5}, "Max": {"X": 0.5, "Y": 0.5}},
                        "Alignment": {"X": 0.5, "Y": 0.5}
                    },
                    "Children": [
                        {
                            "Type": "TextBlock",
                            "Name": "TitleText",
                            "Text": "MINI FOOTBALL",
                            "Font": {"Size": 48, "Typeface": "Bold"},
                            "Justification": "Center",
                            "Slot": {"HAlign": "Center", "Padding": {"Bottom": 30}}
                        },
                        {
                            "Type": "Button",
                            "Name": "NewGameButton",
                            "BindingType": "Required",
                            "Slot": {"HAlign": "Fill", "Padding": {"Bottom": 10}},
                            "Children": [
                                {"Type": "TextBlock", "Name": "NewGameLabel", "Text": "NEW GAME", "FontSize": 18, "Justification": "Center"}
                            ]
                        },
                        {
                            "Type": "Button",
                            "Name": "ContinueButton",
                            "BindingType": "Required",
                            "Slot": {"HAlign": "Fill", "Padding": {"Bottom": 10}},
                            "Children": [
                                {"Type": "TextBlock", "Name": "ContinueLabel", "Text": "CONTINUE", "FontSize": 18, "Justification": "Center"}
                            ]
                        },
                        {
                            "Type": "Button",
                            "Name": "SettingsButton",
                            "BindingType": "Required",
                            "Slot": {"HAlign": "Fill", "Padding": {"Bottom": 10}},
                            "Children": [
                                {"Type": "TextBlock", "Name": "SettingsLabel", "Text": "SETTINGS", "FontSize": 18, "Justification": "Center"}
                            ]
                        },
                        {
                            "Type": "Button",
                            "Name": "QuitButton",
                            "BindingType": "Required",
                            "Slot": {"HAlign": "Fill", "Padding": {"Bottom": 20}},
                            "Children": [
                                {"Type": "TextBlock", "Name": "QuitLabel", "Text": "QUIT GAME", "FontSize": 18, "Justification": "Center"}
                            ]
                        },
                        {
                            "Type": "TextBlock",
                            "Name": "VersionText",
                            "BindingType": "Optional",
                            "Text": "v1.0.0",
                            "FontSize": 12,
                            "Justification": "Center",
                            "Slot": {"HAlign": "Center"}
                        }
                    ]
                }
            ]
        }
    },

    "Bindings": {
        "Required": [
            {"Name": "NewGameButton", "Type": "UButton"},
            {"Name": "ContinueButton", "Type": "UButton"},
            {"Name": "SettingsButton", "Type": "UButton"},
            {"Name": "QuitButton", "Type": "UButton"}
        ],
        "Optional": [
            {"Name": "VersionText", "Type": "UTextBlock"}
        ]
    },

    "Dependencies": [
        {"Class": "UMF_MainSettings", "Blueprint": "WBP_MF_MainSettings", "Required": false, "Order": 1}
    ]
})JSON";

    return Spec;
}

void UMF_MainMenu::NativeConstruct()
{
    Super::NativeConstruct();

    if (APlayerController *PC = GetOwningPlayer())
    {
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }

    if (NewGameButton)
    {
        NewGameButton->OnClicked.AddDynamic(this, &UMF_MainMenu::HandleNewGameClicked);
    }
    if (ContinueButton)
    {
        ContinueButton->OnClicked.AddDynamic(this, &UMF_MainMenu::HandleContinueClicked);
    }
    if (SettingsButton)
    {
        SettingsButton->OnClicked.AddDynamic(this, &UMF_MainMenu::HandleSettingsClicked);
    }
    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UMF_MainMenu::HandleQuitClicked);
    }

    RefreshState();
}

void UMF_MainMenu::NativeDestruct()
{
    if (NewGameButton)
    {
        NewGameButton->OnClicked.RemoveDynamic(this, &UMF_MainMenu::HandleNewGameClicked);
    }
    if (ContinueButton)
    {
        ContinueButton->OnClicked.RemoveDynamic(this, &UMF_MainMenu::HandleContinueClicked);
    }
    if (SettingsButton)
    {
        SettingsButton->OnClicked.RemoveDynamic(this, &UMF_MainMenu::HandleSettingsClicked);
    }
    if (QuitButton)
    {
        QuitButton->OnClicked.RemoveDynamic(this, &UMF_MainMenu::HandleQuitClicked);
    }

    if (MainSettings)
    {
        MainSettings->OnClosed.RemoveDynamic(this, &UMF_MainMenu::HandleSettingsClosed);
    }

    Super::NativeDestruct();
}

void UMF_MainMenu::RefreshState()
{
    FName MostRecent = NAME_None;
    const bool bHasAny = GetMostRecentTemplateName(MostRecent);
    SetContinueEnabled(bHasAny);

    if (VersionText)
    {
        VersionText->SetText(FText::FromString(TEXT("v1.0.0")));
    }
}

void UMF_MainMenu::HandleNewGameClicked()
{
    // New Game follows the spec flow: open settings overlay first.
    // We only launch gameplay if a *new* profile/template is saved during this session.
    FName BaselineName = NAME_None;
    NewGameBaselineTemplateTime = FDateTime::MinValue();
    GetMostRecentTemplateInfo(BaselineName, NewGameBaselineTemplateTime);
    bPendingLaunchAfterSettings = true;

    HandleSettingsClicked();
}

void UMF_MainMenu::HandleContinueClicked()
{
    UCPP_InputBindingManager *Manager = GetMEISManager();
    APlayerController *PC = GetOwningPlayer();
    if (!Manager || !PC)
    {
        return;
    }

    // Ensure we have a player registered.
    if (!Manager->HasPlayerRegistered(PC))
    {
        Manager->RegisterPlayer(PC);
    }

    FName MostRecentTemplate = NAME_None;
    if (!GetMostRecentTemplateName(MostRecentTemplate))
    {
        SetContinueEnabled(false);
        return;
    }

    Manager->ApplyTemplateToPlayer(PC, MostRecentTemplate);

    if (!GameplayLevelName.IsNone())
    {
        UGameplayStatics::OpenLevel(GetWorld(), GameplayLevelName);
    }
}

void UMF_MainMenu::HandleSettingsClicked()
{
    if (!MainSettings)
    {
        TSubclassOf<UMF_MainSettings> ClassToCreate = MainSettingsClass;
        if (!ClassToCreate && GEngine)
        {
            if (UMF_WidgetConfigurationSubsystem *WidgetConfig = GEngine->GetEngineSubsystem<UMF_WidgetConfigurationSubsystem>())
            {
                const TSubclassOf<UUserWidget> Resolved = WidgetConfig->GetWidgetClass(EMF_WidgetType::MainSettings);
                if (Resolved)
                {
                    ClassToCreate = Resolved.Get();
                }
            }
        }
        if (!ClassToCreate)
        {
            ClassToCreate = UMF_MainSettings::StaticClass();
        }

        MainSettings = CreateWidget<UMF_MainSettings>(GetOwningPlayer(), ClassToCreate);
        if (MainSettings)
        {
            MainSettings->OnClosed.AddDynamic(this, &UMF_MainMenu::HandleSettingsClosed);
            MainSettings->AddToViewport(2000);
        }
    }

    if (MainSettings)
    {
        MainSettings->Show();

        if (APlayerController *PC = GetOwningPlayer())
        {
            FInputModeUIOnly InputMode;
            InputMode.SetWidgetToFocus(MainSettings->TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = true;
        }
    }
}

void UMF_MainMenu::HandleQuitClicked()
{
    if (APlayerController *PC = GetOwningPlayer())
    {
        PC->ConsoleCommand(TEXT("quit"));
    }
}

void UMF_MainMenu::HandleSettingsClosed()
{
    // Re-check continue button state in case new profiles were created.
    RefreshState();

    if (!bPendingLaunchAfterSettings)
    {
        return;
    }

    bPendingLaunchAfterSettings = false;

    FName MostRecentTemplate = NAME_None;
    FDateTime MostRecentTime = FDateTime::MinValue();
    if (!GetMostRecentTemplateInfo(MostRecentTemplate, MostRecentTime))
    {
        return;
    }

    // Only proceed if we actually saved something new during this New Game session.
    if (MostRecentTime <= NewGameBaselineTemplateTime)
    {
        return;
    }

    UCPP_InputBindingManager *Manager = GetMEISManager();
    APlayerController *PC = GetOwningPlayer();
    if (!Manager || !PC)
    {
        return;
    }

    if (!Manager->HasPlayerRegistered(PC))
    {
        Manager->RegisterPlayer(PC);
    }

    Manager->ApplyTemplateToPlayer(PC, MostRecentTemplate);

    if (!GameplayLevelName.IsNone())
    {
        UGameplayStatics::OpenLevel(GetWorld(), GameplayLevelName);
    }
}

bool UMF_MainMenu::GetMostRecentTemplateName(FName &OutTemplateName) const
{
    FDateTime Timestamp;
    return GetMostRecentTemplateInfo(OutTemplateName, Timestamp);
}

bool UMF_MainMenu::GetMostRecentTemplateInfo(FName &OutTemplateName, FDateTime &OutTimestamp) const
{
    OutTemplateName = NAME_None;
    OutTimestamp = FDateTime::MinValue();

    UCPP_InputBindingManager *Manager = GetMEISManager();
    if (!Manager)
    {
        return false;
    }

    TArray<FName> Templates;
    Manager->GetAvailableTemplates(Templates);
    if (Templates.Num() <= 0)
    {
        return false;
    }

    FDateTime BestTime = FDateTime::MinValue();
    FName BestName = NAME_None;

    for (const FName &TemplateName : Templates)
    {
        FS_InputProfile Profile;
        if (Manager->GetTemplate(TemplateName, Profile))
        {
            if (Profile.Timestamp > BestTime)
            {
                BestTime = Profile.Timestamp;
                BestName = TemplateName;
            }
        }
    }

    if (BestName.IsNone())
    {
        // Fallback: use the first template if timestamps weren't readable.
        BestName = Templates[0];
        BestTime = FDateTime::MinValue();
    }

    OutTemplateName = BestName;
    OutTimestamp = BestTime;
    return !OutTemplateName.IsNone();
}

void UMF_MainMenu::SetContinueEnabled(bool bEnabled)
{
    if (!ContinueButton)
    {
        return;
    }

    ContinueButton->SetIsEnabled(bEnabled);
}
