/*
 * @Author: Punal Manalan
 * @Description: MF_GraphicsSettings - Graphics settings overlay (stub) implementation
 * @Date: 14/12/2025
 */

#include "MF_GraphicsSettings.h"

#include "Components/Button.h"

FString UMF_GraphicsSettings::GetWidgetSpec()
{
    static FString Spec = R"JSON({
    "WidgetClass": "UMF_GraphicsSettings",
    "BlueprintName": "WBP_MF_GraphicsSettings",
    "ParentClass": "/Script/P_MiniFootball.MF_GraphicsSettings",
    "Category": "MF|UI|Menus",
    "Description": "Graphics settings overlay (stub)",
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
                    "Name": "GraphicsContainer",
                    "Slot": {"Anchors": {"Min": {"X": 0.5, "Y": 0.5}, "Max": {"X": 0.5, "Y": 0.5}}, "Alignment": {"X": 0.5, "Y": 0.5}},
                    "Children": [
                        {"Type": "TextBlock", "Name": "GraphicsTitle", "Text": "GRAPHICS SETTINGS", "Font": {"Size": 28, "Typeface": "Bold"}, "Slot": {"HAlign": "Center", "Padding": {"Bottom": 20}}},
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

void UMF_GraphicsSettings::NativeConstruct()
{
    Super::NativeConstruct();

    if (BackButton)
    {
        BackButton->OnClicked.AddDynamic(this, &UMF_GraphicsSettings::HandleBackClicked);
    }

    SetVisibility(ESlateVisibility::Collapsed);
}

void UMF_GraphicsSettings::NativeDestruct()
{
    if (BackButton)
    {
        BackButton->OnClicked.RemoveDynamic(this, &UMF_GraphicsSettings::HandleBackClicked);
    }

    Super::NativeDestruct();
}

void UMF_GraphicsSettings::Show()
{
    SetVisibility(ESlateVisibility::Visible);
}

void UMF_GraphicsSettings::Hide()
{
    SetVisibility(ESlateVisibility::Collapsed);
    OnClosed.Broadcast();
}

void UMF_GraphicsSettings::HandleBackClicked()
{
    Hide();
}
