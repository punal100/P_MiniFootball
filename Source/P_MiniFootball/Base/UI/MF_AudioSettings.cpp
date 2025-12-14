/*
 * @Author: Punal Manalan
 * @Description: MF_AudioSettings - Audio settings overlay (stub) implementation
 * @Date: 14/12/2025
 */

#include "MF_AudioSettings.h"

#include "Components/Button.h"

FString UMF_AudioSettings::GetWidgetSpec()
{
    static FString Spec = R"JSON({
    "WidgetClass": "UMF_AudioSettings",
    "BlueprintName": "WBP_MF_AudioSettings",
    "ParentClass": "/Script/P_MiniFootball.MF_AudioSettings",
    "Category": "MF|UI|Menus",
    "Description": "Audio settings overlay (stub)",
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
                    "Name": "AudioContainer",
                    "Slot": {"Anchors": {"Min": {"X": 0.5, "Y": 0.5}, "Max": {"X": 0.5, "Y": 0.5}}, "Alignment": {"X": 0.5, "Y": 0.5}},
                    "Children": [
                        {"Type": "TextBlock", "Name": "AudioTitle", "Text": "AUDIO SETTINGS", "Font": {"Size": 28, "Typeface": "Bold"}, "Slot": {"HAlign": "Center", "Padding": {"Bottom": 20}}},
                        {"Type": "Button", "Name": "BackButton", "BindingType": "Required", "Slot": {"HAlign": "Center"}, "Children": [{"Type": "TextBlock", "Name": "BackLabel", "Text": "BACK", "FontSize": 16, "Justification": "Center"}]}
                    ]
                }
            ]
        }
    },

    "Bindings": {
        "Required": [
            {"Name": "BackButton", "Type": "UButton"}
        ]
    }
})JSON";

    return Spec;
}

void UMF_AudioSettings::NativeConstruct()
{
    Super::NativeConstruct();

    if (BackButton)
    {
        BackButton->OnClicked.AddDynamic(this, &UMF_AudioSettings::HandleBackClicked);
    }

    SetVisibility(ESlateVisibility::Collapsed);
}

void UMF_AudioSettings::NativeDestruct()
{
    if (BackButton)
    {
        BackButton->OnClicked.RemoveDynamic(this, &UMF_AudioSettings::HandleBackClicked);
    }

    Super::NativeDestruct();
}

void UMF_AudioSettings::Show()
{
    SetVisibility(ESlateVisibility::Visible);
}

void UMF_AudioSettings::Hide()
{
    SetVisibility(ESlateVisibility::Collapsed);
    OnClosed.Broadcast();
}

void UMF_AudioSettings::HandleBackClicked()
{
    Hide();
}
