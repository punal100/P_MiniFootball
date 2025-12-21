/*
 * @Author: Punal Manalan
 * @Description: MF_MainSettings - Main settings overlay with WidgetSwitcher implementation
 * @Date: 14/12/2025
 * @Updated: 21/12/2025 - Added WidgetSwitcher for panel-based settings navigation
 */

#include "MF_MainSettings.h"

#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/VerticalBox.h"
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
    "Description": "Main settings overlay with WidgetSwitcher navigation",
    "Version": "2.0.0",

    "DesignerPreview": {"SizeMode": "FillScreen", "ZoomLevel": 14, "ShowGrid": true},

    "Hierarchy": {
        "Root": {
            "Type": "CanvasPanel",
            "Name": "RootCanvas",
            "Children": [
                {"Type": "Overlay", "Name": "BackgroundOverlay", "Slot": {"Anchors": {"Min": {"X": 0, "Y": 0}, "Max": {"X": 1, "Y": 1}}, "Offsets": {"Left": 0, "Top": 0, "Right": 0, "Bottom": 0}}},
                {
                    "Type": "WidgetSwitcher",
                    "Name": "SettingsSwitcher",
                    "BindingType": "Required",
                    "Slot": {"Anchors": {"Min": {"X": 0, "Y": 0}, "Max": {"X": 1, "Y": 1}}, "Offsets": {"Left": 0, "Top": 0, "Right": 0, "Bottom": 0}},
                    "Children": [
                        {
                            "Type": "VerticalBox",
                            "Name": "SettingsMenuPanel",
                            "BindingType": "Required",
                            "Slot": {"HAlign": "Center", "VAlign": "Center"},
                            "Children": [
                                {"Type": "TextBlock", "Name": "SettingsTitle", "Text": "SETTINGS", "Font": {"Size": 28, "Typeface": "Bold"}, "Slot": {"HAlign": "Center", "Padding": {"Bottom": 20}}},
                                {"Type": "Button", "Name": "InputButton", "BindingType": "Required", "Slot": {"HAlign": "Fill", "Padding": {"Bottom": 10}}, "Children": [{"Type": "TextBlock", "Name": "InputLabel", "Text": "INPUT", "FontSize": 18, "Justification": "Center"}]},
                                {"Type": "Button", "Name": "AudioButton", "BindingType": "Optional", "Slot": {"HAlign": "Fill", "Padding": {"Bottom": 10}}, "Children": [{"Type": "TextBlock", "Name": "AudioLabel", "Text": "AUDIO", "FontSize": 18, "Justification": "Center"}]},
                                {"Type": "Button", "Name": "GraphicsButton", "BindingType": "Optional", "Slot": {"HAlign": "Fill", "Padding": {"Bottom": 20}}, "Children": [{"Type": "TextBlock", "Name": "GraphicsLabel", "Text": "GRAPHICS", "FontSize": 18, "Justification": "Center"}]},
                                {"Type": "Button", "Name": "BackButton", "BindingType": "Required", "Slot": {"HAlign": "Center"}, "Children": [{"Type": "TextBlock", "Name": "BackLabel", "Text": "BACK", "FontSize": 16, "Justification": "Center"}]}
                            ]
                        },
                        {
                            "Type": "VerticalBox",
                            "Name": "InputSettingsPanel",
                            "BindingType": "Required",
                            "Slot": {"HAlign": "Fill", "VAlign": "Fill"},
                            "Children": [
                                {"Type": "TextBlock", "Name": "InputSettingsTitle", "Text": "INPUT SETTINGS", "Font": {"Size": 28, "Typeface": "Bold"}, "Slot": {"HAlign": "Center", "Padding": {"Bottom": 20}}},
                                {"Type": "TextBlock", "Name": "InputSettingsPlaceholder", "Text": "(Input binding controls embedded at runtime)", "FontSize": 14, "Slot": {"HAlign": "Center", "Padding": {"Bottom": 20}}},
                                {"Type": "Button", "Name": "InputBackButton", "BindingType": "Optional", "Slot": {"HAlign": "Center", "Padding": {"Top": 10}}, "Children": [{"Type": "TextBlock", "Name": "InputBackLabel", "Text": "BACK", "FontSize": 16, "Justification": "Center"}]}
                            ]
                        },
                        {
                            "Type": "VerticalBox",
                            "Name": "AudioSettingsPanel",
                            "BindingType": "Optional",
                            "Slot": {"HAlign": "Center", "VAlign": "Center"},
                            "Children": [
                                {"Type": "TextBlock", "Name": "AudioSettingsTitle", "Text": "AUDIO SETTINGS", "Font": {"Size": 28, "Typeface": "Bold"}, "Slot": {"HAlign": "Center", "Padding": {"Bottom": 20}}},
                                {"Type": "TextBlock", "Name": "AudioPlaceholder", "Text": "(Audio controls here)", "FontSize": 14, "Slot": {"HAlign": "Center", "Padding": {"Bottom": 20}}},
                                {"Type": "Button", "Name": "AudioBackButton", "BindingType": "Optional", "Slot": {"HAlign": "Center"}, "Children": [{"Type": "TextBlock", "Name": "AudioBackLabel", "Text": "BACK", "FontSize": 16, "Justification": "Center"}]}
                            ]
                        },
                        {
                            "Type": "VerticalBox",
                            "Name": "GraphicsSettingsPanel",
                            "BindingType": "Optional",
                            "Slot": {"HAlign": "Center", "VAlign": "Center"},
                            "Children": [
                                {"Type": "TextBlock", "Name": "GraphicsSettingsTitle", "Text": "GRAPHICS SETTINGS", "Font": {"Size": 28, "Typeface": "Bold"}, "Slot": {"HAlign": "Center", "Padding": {"Bottom": 20}}},
                                {"Type": "TextBlock", "Name": "GraphicsPlaceholder", "Text": "(Graphics controls here)", "FontSize": 14, "Slot": {"HAlign": "Center", "Padding": {"Bottom": 20}}},
                                {"Type": "Button", "Name": "GraphicsBackButton", "BindingType": "Optional", "Slot": {"HAlign": "Center"}, "Children": [{"Type": "TextBlock", "Name": "GraphicsBackLabel", "Text": "BACK", "FontSize": 16, "Justification": "Center"}]}
                            ]
                        }
                    ]
                }
            ]
        }
    },

    "Bindings": {
        "Required": [
            {"Name": "SettingsSwitcher", "Type": "UWidgetSwitcher"},
            {"Name": "SettingsMenuPanel", "Type": "UVerticalBox"},
            {"Name": "InputButton", "Type": "UButton"},
            {"Name": "BackButton", "Type": "UButton"},
            {"Name": "InputSettingsPanel", "Type": "UVerticalBox"}
        ],
        "Optional": [
            {"Name": "AudioButton", "Type": "UButton"},
            {"Name": "GraphicsButton", "Type": "UButton"},
            {"Name": "InputBackButton", "Type": "UButton"},
            {"Name": "AudioSettingsPanel", "Type": "UVerticalBox"},
            {"Name": "AudioBackButton", "Type": "UButton"},
            {"Name": "GraphicsSettingsPanel", "Type": "UVerticalBox"},
            {"Name": "GraphicsBackButton", "Type": "UButton"}
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

    UE_LOG(LogTemp, Log, TEXT("MF_MainSettings::NativeConstruct - Initializing with WidgetSwitcher"));

    BindButtonEvents();

    // Start on settings menu panel
    SwitchToPanel(EMF_SettingsPanel::SettingsMenu);

    // Pre-create embedded input settings if we have a slot
    EnsureInputSettingsCreated();

    SetVisibility(ESlateVisibility::Collapsed);
}

void UMF_MainSettings::NativeDestruct()
{
    UnbindButtonEvents();
    Super::NativeDestruct();
}

void UMF_MainSettings::BindButtonEvents()
{
    // Settings Menu buttons
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

    // Panel back buttons
    if (InputBackButton)
    {
        InputBackButton->OnClicked.AddDynamic(this, &UMF_MainSettings::HandleInputBackClicked);
    }
    if (AudioBackButton)
    {
        AudioBackButton->OnClicked.AddDynamic(this, &UMF_MainSettings::HandleAudioBackClicked);
    }
    if (GraphicsBackButton)
    {
        GraphicsBackButton->OnClicked.AddDynamic(this, &UMF_MainSettings::HandleGraphicsBackClicked);
    }
}

void UMF_MainSettings::UnbindButtonEvents()
{
    // Settings Menu buttons
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

    // Panel back buttons
    if (InputBackButton)
    {
        InputBackButton->OnClicked.RemoveDynamic(this, &UMF_MainSettings::HandleInputBackClicked);
    }
    if (AudioBackButton)
    {
        AudioBackButton->OnClicked.RemoveDynamic(this, &UMF_MainSettings::HandleAudioBackClicked);
    }
    if (GraphicsBackButton)
    {
        GraphicsBackButton->OnClicked.RemoveDynamic(this, &UMF_MainSettings::HandleGraphicsBackClicked);
    }
}

// ==================== Visibility ====================

void UMF_MainSettings::Show()
{
    SetVisibility(ESlateVisibility::Visible);
    // Always show settings menu when showing
    SwitchToPanel(EMF_SettingsPanel::SettingsMenu);
}

void UMF_MainSettings::Hide()
{
    SetVisibility(ESlateVisibility::Collapsed);
    OnClosed.Broadcast();
}

// ==================== Panel Navigation ====================

void UMF_MainSettings::SwitchToPanel(EMF_SettingsPanel Panel)
{
    if (!SettingsSwitcher)
    {
        UE_LOG(LogTemp, Error, TEXT("MF_MainSettings::SwitchToPanel - SettingsSwitcher is NULL!"));
        return;
    }

    const int32 PanelIndex = static_cast<int32>(Panel);
    UE_LOG(LogTemp, Log, TEXT("MF_MainSettings::SwitchToPanel - Switching from %d to %d"),
        static_cast<int32>(CurrentPanel), PanelIndex);

    CurrentPanel = Panel;
    SettingsSwitcher->SetActiveWidgetIndex(PanelIndex);

    // Ensure embedded widgets are created when navigating to their panels
    switch (Panel)
    {
        case EMF_SettingsPanel::InputSettings:
            EnsureInputSettingsCreated();
            if (EmbeddedInputSettings)
            {
                EmbeddedInputSettings->Show();
            }
            break;
        case EMF_SettingsPanel::AudioSettings:
            EnsureAudioSettingsCreated();
            break;
        case EMF_SettingsPanel::GraphicsSettings:
            EnsureGraphicsSettingsCreated();
            break;
        default:
            break;
    }
}

// ==================== Embedded Widget Creation ====================

void UMF_MainSettings::EnsureInputSettingsCreated()
{
    if (EmbeddedInputSettings)
    {
        return;
    }

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

    EmbeddedInputSettings = CreateWidget<UMF_InputSettings>(GetOwningPlayer(), ClassToCreate);
    if (EmbeddedInputSettings && InputSettingsPanel)
    {
        // Insert before the back button (which is last child)
        InputSettingsPanel->AddChild(EmbeddedInputSettings);
        EmbeddedInputSettings->SetVisibility(ESlateVisibility::Visible);
        UE_LOG(LogTemp, Log, TEXT("MF_MainSettings - Embedded InputSettings into InputSettingsPanel"));
    }
}

void UMF_MainSettings::EnsureAudioSettingsCreated()
{
    // Placeholder for future audio settings implementation
}

void UMF_MainSettings::EnsureGraphicsSettingsCreated()
{
    // Placeholder for future graphics settings implementation
}

// ==================== Button Handlers ====================

void UMF_MainSettings::HandleInputClicked()
{
    UE_LOG(LogTemp, Log, TEXT("MF_MainSettings::HandleInputClicked - Switching to Input Settings"));
    SwitchToPanel(EMF_SettingsPanel::InputSettings);
}

void UMF_MainSettings::HandleAudioClicked()
{
    UE_LOG(LogTemp, Log, TEXT("MF_MainSettings::HandleAudioClicked - Switching to Audio Settings"));
    SwitchToPanel(EMF_SettingsPanel::AudioSettings);
}

void UMF_MainSettings::HandleGraphicsClicked()
{
    UE_LOG(LogTemp, Log, TEXT("MF_MainSettings::HandleGraphicsClicked - Switching to Graphics Settings"));
    SwitchToPanel(EMF_SettingsPanel::GraphicsSettings);
}

void UMF_MainSettings::HandleBackClicked()
{
    UE_LOG(LogTemp, Log, TEXT("MF_MainSettings::HandleBackClicked - Hiding settings"));
    Hide();
}

void UMF_MainSettings::HandleInputBackClicked()
{
    UE_LOG(LogTemp, Log, TEXT("MF_MainSettings::HandleInputBackClicked - Returning to menu"));
    GoBackToMenu();
}

void UMF_MainSettings::HandleAudioBackClicked()
{
    UE_LOG(LogTemp, Log, TEXT("MF_MainSettings::HandleAudioBackClicked - Returning to menu"));
    GoBackToMenu();
}

void UMF_MainSettings::HandleGraphicsBackClicked()
{
    UE_LOG(LogTemp, Log, TEXT("MF_MainSettings::HandleGraphicsBackClicked - Returning to menu"));
    GoBackToMenu();
}
