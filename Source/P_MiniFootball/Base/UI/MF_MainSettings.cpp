/*
 * @Author: Punal Manalan
 * @Description: MF_MainSettings - Main settings overlay implementation
 * @Date: 14/12/2025
 */

#include "MF_MainSettings.h"

#include "Components/Button.h"
#include "Engine/Engine.h"
#include "UObject/UObjectGlobals.h"
#include "MF_AudioSettings.h"
#include "MF_GraphicsSettings.h"
#include "MF_InputSettings.h"

#include "UI/Configuration/MF_WidgetConfigurationSubsystem.h"
#include "UI/Configuration/MF_WidgetTypes.h"

FString UMF_MainSettings::GetWidgetSpec()
{
    static FString Spec = R"JSON({
    "WidgetClass": "UMF_MainSettings",
    "BlueprintName": "WBP_MF_MainSettings",
    "ParentClass": "/Script/P_MiniFootball.MF_MainSettings",
    "Category": "MF|UI|Menus",
    "Description": "Main settings overlay (Input/Audio/Graphics)",
    "Version": "1.0.0",

    "DesignerPreview": {"SizeMode": "FillScreen", "ZoomLevel": 14, "ShowGrid": true},

    "Hierarchy": {
        "Root": {
            "Type": "CanvasPanel",
            "Name": "RootCanvas",
            "Children": [
                {"Type": "Overlay", "Name": "BackgroundOverlay", "Slot": {"Anchors": {"Min": {"X": 0, "Y": 0}, "Max": {"X": 1, "Y": 1}}, "Offsets": {"Left": 0, "Top": 0, "Right": 0, "Bottom": 0}}},
                {
                    "Type": "VerticalBox",
                    "Name": "SettingsContainer",
                    "Slot": {"Anchors": {"Min": {"X": 0.5, "Y": 0.5}, "Max": {"X": 0.5, "Y": 0.5}}, "Alignment": {"X": 0.5, "Y": 0.5}},
                    "Children": [
                        {"Type": "TextBlock", "Name": "SettingsTitle", "Text": "SETTINGS", "Font": {"Size": 28, "Typeface": "Bold"}, "Slot": {"HAlign": "Center", "Padding": {"Bottom": 20}}},
                        {"Type": "Button", "Name": "InputButton", "BindingType": "Required", "Slot": {"HAlign": "Fill", "Padding": {"Bottom": 10}}, "Children": [{"Type": "TextBlock", "Name": "InputLabel", "Text": "INPUT", "FontSize": 18, "Justification": "Center"}]},
                        {"Type": "Button", "Name": "AudioButton", "BindingType": "Optional", "Slot": {"HAlign": "Fill", "Padding": {"Bottom": 10}}, "Children": [{"Type": "TextBlock", "Name": "AudioLabel", "Text": "AUDIO", "FontSize": 18, "Justification": "Center"}]},
                        {"Type": "Button", "Name": "GraphicsButton", "BindingType": "Optional", "Slot": {"HAlign": "Fill", "Padding": {"Bottom": 20}}, "Children": [{"Type": "TextBlock", "Name": "GraphicsLabel", "Text": "GRAPHICS", "FontSize": 18, "Justification": "Center"}]},
                        {"Type": "Button", "Name": "BackButton", "BindingType": "Required", "Slot": {"HAlign": "Center"}, "Children": [{"Type": "TextBlock", "Name": "BackLabel", "Text": "BACK", "FontSize": 16, "Justification": "Center"}]}
                    ]
                }
            ]
        }
    },

    "Bindings": {
        "Required": [
            {"Name": "InputButton", "Type": "UButton"},
            {"Name": "BackButton", "Type": "UButton"}
        ],
        "Optional": [
            {"Name": "AudioButton", "Type": "UButton"},
            {"Name": "GraphicsButton", "Type": "UButton"}
        ]
    },

    "Dependencies": [
        {"Class": "UMF_InputSettings", "Blueprint": "WBP_MF_InputSettings", "Required": false, "Order": 1},
        {"Class": "UMF_AudioSettings", "Blueprint": "WBP_MF_AudioSettings", "Required": false, "Order": 2},
        {"Class": "UMF_GraphicsSettings", "Blueprint": "WBP_MF_GraphicsSettings", "Required": false, "Order": 3}
    ]
})JSON";

    return Spec;
}

void UMF_MainSettings::NativeConstruct()
{
    Super::NativeConstruct();

    if (InputButton)
    {
        InputButton->OnClicked.AddDynamic(this, &UMF_MainSettings::HandleInputClicked);
    }
    if (AudioButton)
    {
        AudioButton->OnClicked.AddDynamic(this, &UMF_MainSettings::HandleAudioClicked);
    }
    if (GraphicsButton)
    {
        GraphicsButton->OnClicked.AddDynamic(this, &UMF_MainSettings::HandleGraphicsClicked);
    }
    if (BackButton)
    {
        BackButton->OnClicked.AddDynamic(this, &UMF_MainSettings::HandleBackClicked);
    }

    SetVisibility(ESlateVisibility::Collapsed);
}

void UMF_MainSettings::NativeDestruct()
{
    if (InputButton)
    {
        InputButton->OnClicked.RemoveDynamic(this, &UMF_MainSettings::HandleInputClicked);
    }
    if (AudioButton)
    {
        AudioButton->OnClicked.RemoveDynamic(this, &UMF_MainSettings::HandleAudioClicked);
    }
    if (GraphicsButton)
    {
        GraphicsButton->OnClicked.RemoveDynamic(this, &UMF_MainSettings::HandleGraphicsClicked);
    }
    if (BackButton)
    {
        BackButton->OnClicked.RemoveDynamic(this, &UMF_MainSettings::HandleBackClicked);
    }

    Super::NativeDestruct();
}

void UMF_MainSettings::Show()
{
    SetVisibility(ESlateVisibility::Visible);
}

void UMF_MainSettings::Hide()
{
    SetVisibility(ESlateVisibility::Collapsed);
    OnClosed.Broadcast();
}

void UMF_MainSettings::HandleInputClicked()
{
    if (!InputSettings)
    {
        TSubclassOf<UMF_InputSettings> ClassToCreate = InputSettingsClass;
        if (!ClassToCreate && GEngine)
        {
            if (UMF_WidgetConfigurationSubsystem *WidgetConfig = GEngine->GetEngineSubsystem<UMF_WidgetConfigurationSubsystem>())
            {
                const TSubclassOf<UUserWidget> Resolved = WidgetConfig->GetWidgetClass(EMF_WidgetType::InputSettings);
                if (Resolved)
                {
                    ClassToCreate = Resolved.Get();
                }
            }
        }
        if (!ClassToCreate)
        {
            ClassToCreate = UMF_InputSettings::StaticClass();
        }

        InputSettings = CreateWidget<UMF_InputSettings>(GetOwningPlayer(), ClassToCreate);
        if (InputSettings)
        {
            InputSettings->OnClosed.AddDynamic(this, &UMF_MainSettings::HandleChildClosed);
            InputSettings->AddToViewport(2000);
        }
    }

    if (InputSettings)
    {
        InputSettings->Show();
    }
}

void UMF_MainSettings::HandleAudioClicked()
{
    if (!AudioSettings)
    {
        TSubclassOf<UMF_AudioSettings> ClassToCreate = AudioSettingsClass;
        if (!ClassToCreate && GEngine)
        {
            if (UMF_WidgetConfigurationSubsystem *WidgetConfig = GEngine->GetEngineSubsystem<UMF_WidgetConfigurationSubsystem>())
            {
                const TSubclassOf<UUserWidget> Resolved = WidgetConfig->GetWidgetClass(EMF_WidgetType::AudioSettings);
                if (Resolved)
                {
                    ClassToCreate = Resolved.Get();
                }
            }
        }
        if (!ClassToCreate)
        {
            ClassToCreate = UMF_AudioSettings::StaticClass();
        }

        AudioSettings = CreateWidget<UMF_AudioSettings>(GetOwningPlayer(), ClassToCreate);
        if (AudioSettings)
        {
            AudioSettings->OnClosed.AddDynamic(this, &UMF_MainSettings::HandleChildClosed);
            AudioSettings->AddToViewport(2000);
        }
    }

    if (AudioSettings)
    {
        AudioSettings->Show();
    }
}

void UMF_MainSettings::HandleGraphicsClicked()
{
    if (!GraphicsSettings)
    {
        TSubclassOf<UMF_GraphicsSettings> ClassToCreate = GraphicsSettingsClass;
        if (!ClassToCreate && GEngine)
        {
            if (UMF_WidgetConfigurationSubsystem *WidgetConfig = GEngine->GetEngineSubsystem<UMF_WidgetConfigurationSubsystem>())
            {
                const TSubclassOf<UUserWidget> Resolved = WidgetConfig->GetWidgetClass(EMF_WidgetType::GraphicsSettings);
                if (Resolved)
                {
                    ClassToCreate = Resolved.Get();
                }
            }
        }
        if (!ClassToCreate)
        {
            ClassToCreate = UMF_GraphicsSettings::StaticClass();
        }

        GraphicsSettings = CreateWidget<UMF_GraphicsSettings>(GetOwningPlayer(), ClassToCreate);
        if (GraphicsSettings)
        {
            GraphicsSettings->OnClosed.AddDynamic(this, &UMF_MainSettings::HandleChildClosed);
            GraphicsSettings->AddToViewport(2000);
        }
    }

    if (GraphicsSettings)
    {
        GraphicsSettings->Show();
    }
}

void UMF_MainSettings::HandleBackClicked()
{
    Hide();
}

void UMF_MainSettings::HandleChildClosed()
{
    // Child overlay closed; keep main settings visible.
    Show();
}
