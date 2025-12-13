/*
 * @Author: Punal Manalan
 * @Description: MF_ScorePopup - Goal notification popup.
 * @Date: 12/12/2025
 */

#include "MF_ScorePopup.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UMF_ScorePopup::NativeConstruct()
{
    Super::NativeConstruct();
    UpdateVisibility();
}

void UMF_ScorePopup::SetScoreText(const FText &InScoreText)
{
    if (ScoreText)
    {
        ScoreText->SetText(InScoreText);
    }
    UpdateVisibility();
}

void UMF_ScorePopup::SetScorerName(const FText &InScorerName)
{
    if (ScorerNameText)
    {
        ScorerNameText->SetText(InScorerName);
    }
    UpdateVisibility();
}

void UMF_ScorePopup::SetBackgroundTint(const FLinearColor &TintColor)
{
    if (BackgroundImage)
    {
        BackgroundImage->SetColorAndOpacity(TintColor);
    }
}

void UMF_ScorePopup::UpdateVisibility()
{
    bool bHasContent = false;

    if (ScoreText)
    {
        bHasContent |= !ScoreText->GetText().IsEmpty();
    }

    if (ScorerNameText)
    {
        bHasContent |= !ScorerNameText->GetText().IsEmpty();
    }

    SetVisibility(bHasContent ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

FString UMF_ScorePopup::GetWidgetSpec()
{
    static const FString Spec = R"JSON({
    "WidgetClass": "UMF_ScorePopup",
    "BlueprintName": "WBP_MF_ScorePopup",
    "ParentClass": "/Script/P_MiniFootball.MF_ScorePopup",
    "Category": "MF|UI|Popups",
    "Description": "Transient goal score notification popup.",
    "Version": "1.0.0",

    "DesignerPreview": {
        "SizeMode": "Desired",
        "ZoomLevel": 14,
        "ShowGrid": true
    },
    "Hierarchy": {
        "Root": {
            "Type": "CanvasPanel",
            "Name": "RootCanvas",
            "Children": [
                {
                    "Type": "Image",
                    "Name": "BackgroundImage",
                    "BindingType": "Optional",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0, "Y": 0}, "Max": {"X": 1, "Y": 1}},
                        "Offsets": {"Left": 0, "Top": 0, "Right": 0, "Bottom": 0}
                    }
                },
                {
                    "Type": "VerticalBox",
                    "Name": "TextContainer",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0.5, "Y": 0.5}, "Max": {"X": 0.5, "Y": 0.5}},
                        "Alignment": {"X": 0.5, "Y": 0.5},
                        "Position": {"X": 0, "Y": 0},
                        "Size": {"X": 260, "Y": 140}
                    },
                    "Children": [
                        {
                            "Type": "TextBlock",
                            "Name": "ScoreText",
                            "BindingType": "Optional",
                            "Slot": {"HAlign": "Center", "Padding": {"Bottom": 8}}
                        },
                        {
                            "Type": "TextBlock",
                            "Name": "ScorerNameText",
                            "BindingType": "Optional",
                            "Slot": {"HAlign": "Center"}
                        }
                    ]
                }
            ]
        }
    },
    "Bindings": {
        "Required": [],
        "Optional": [
            {"Name": "ScoreText", "Type": "UTextBlock", "Purpose": "Primary score display"},
            {"Name": "ScorerNameText", "Type": "UTextBlock", "Purpose": "Scorer name"},
            {"Name": "BackgroundImage", "Type": "UImage", "Purpose": "Decorative backdrop"}
        ]
    },
    "Design": {
        "ScoreText": {"Font": {"Size": 36, "Typeface": "Bold"}, "Color": {"R": 0.95, "G": 0.9, "B": 0.3, "A": 1.0}},
        "ScorerNameText": {"Font": {"Size": 20, "Typeface": "Regular"}, "Color": {"R": 1.0, "G": 1.0, "B": 1.0, "A": 1.0}},
        "BackgroundImage": {"Brush": {"TintColor": {"R": 0.05, "G": 0.05, "B": 0.1, "A": 0.8}}}
    },
    "Dependencies": [],
    "PythonSnippets": {
        "CreatePopup": "root = creator.add_widget('CanvasPanel', 'RootCanvas', None); background = creator.add_widget('Image', 'BackgroundImage', root); container = creator.add_widget('VerticalBox', 'TextContainer', root); creator.add_widget('TextBlock', 'ScoreText', container); creator.add_widget('TextBlock', 'ScorerNameText', container)"
    }
})JSON";
    return Spec;
}
