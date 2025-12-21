/*
 * @Author: Punal Manalan
 * @Description: MF_TransitionOverlay - Loading/transition screen implementation
 * @Date: 10/12/2025
 */

#include "MF_TransitionOverlay.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Throbber.h"
#include "Animation/WidgetAnimation.h"

FString UMF_TransitionOverlay::GetWidgetSpec()
{
    static FString Spec = R"JSON({
    "WidgetClass": "UMF_TransitionOverlay",
    "BlueprintName": "WBP_MF_TransitionOverlay",
    "ParentClass": "/Script/P_MiniFootball.MF_TransitionOverlay",
    "Category": "MF|UI|Overlays",
    "Description": "Loading/transition screen with status message",
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
                    "Type": "Image",
                    "Name": "BackgroundOverlay",
                    "BindingType": "Optional",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0, "Y": 0}, "Max": {"X": 1, "Y": 1}},
                        "Offsets": {"Left": 0, "Top": 0, "Right": 0, "Bottom": 0},
                        "Position": {"X": 0, "Y": 0},
                        "Size": {"X": 0, "Y": 0},
                        "Alignment": {"X": 0, "Y": 0}
                    }
                },
                {
                    "Type": "VerticalBox",
                    "Name": "ContentBox",
                    "Properties": {"SizeToContent": true},
                    "Slot": {
                        "Anchors": {"Min": {"X": 0.5, "Y": 0.5}, "Max": {"X": 0.5, "Y": 0.5}},
                        "Position": {"X": 0, "Y": 0},
                        "Size": {"X": 100, "Y": 30},
                        "Alignment": {"X": 0.5, "Y": 0.5},
                        "AutoSize": true
                    },
                    "Children": [
                        {
                            "Type": "Throbber",
                            "Name": "LoadingThrobber",
                            "BindingType": "Optional",
                            "Slot": {"HAlign": "Center", "VAlign": "Fill", "Size": {"Rule": "Auto"}, "Padding": {"Bottom": 20}}
                        },
                        {
                            "Type": "TextBlock",
                            "Name": "StatusText",
                            "BindingType": "Required",
                            "Text": "LOADING...",
                            "FontSize": 24,
                            "Justification": "Center",
                            "Slot": {"HAlign": "Center", "VAlign": "Fill", "Size": {"Rule": "Auto"}}
                        }
                    ]
                }
            ]
        }
    },
    
    "Design": {
        "BackgroundOverlay": {
            "Brush": {"DrawAs": "Box"},
            "Size": {"X": 32, "Y": 32},
            "ColorAndOpacity": {"R": 0, "G": 0, "B": 0, "A": 0.85}
        },
        "StatusText": {
            "Font": {"Size": 24, "Typeface": "Regular"},
            "ColorAndOpacity": {"R": 1.0, "G": 1.0, "B": 1.0, "A": 1.0}
        }
    },
    
    "Bindings": {
        "Required": [
            {"Name": "StatusText", "Type": "UTextBlock", "Purpose": "Loading status message"}
        ],
        "Optional": [
            {"Name": "LoadingThrobber", "Type": "UThrobber", "Purpose": "Loading spinner"},
            {"Name": "BackgroundOverlay", "Type": "UImage", "Purpose": "Dark background"}
        ]
    },
    
    "Delegates": [],
    
    "Dependencies": [
        "/Engine/EngineFonts/Roboto.Roboto"
    ],
    
    "Comments": {
        "Header": "MF Transition Overlay - Blocking loading screen",
        "Usage": "Shown during team join/leave transitions"
    },
    
    "PythonSnippets": {
        "CreateRoot": "root = creator.add_widget('CanvasPanel', 'RootCanvas', None)",
        "CreateBG": "bg = creator.add_widget('Image', 'BackgroundOverlay', root, slot_data={'anchors': 'fill'})",
        "CreateContent": "vbox = creator.add_widget('VerticalBox', 'ContentBox', root); creator.add_widget('TextBlock', 'StatusText', vbox)"
    }
})JSON";
    return Spec;
}

void UMF_TransitionOverlay::NativeConstruct()
{
    Super::NativeConstruct();

    // CRITICAL: Start hidden by default - only show when explicitly requested
    SetVisibility(ESlateVisibility::Hidden);

    // Set default message (for when it does get shown)
    SetStatusText(DefaultStatusMessage);
}

void UMF_TransitionOverlay::SetStatusText(const FString &InStatusText)
{
    if (StatusText)
    {
        StatusText->SetText(FText::FromString(InStatusText));
    }
}

void UMF_TransitionOverlay::ShowOverlay()
{
    SetVisibility(ESlateVisibility::Visible);
    PlayFadeAnimation(true);
}

void UMF_TransitionOverlay::HideOverlay()
{
    PlayFadeAnimation(false);
    // Note: In a full implementation, we'd use an animation callback to set visibility to Hidden
    // For now, we'll set it immediately after starting the fade
    SetVisibility(ESlateVisibility::Hidden);
}

void UMF_TransitionOverlay::ShowWithMessage(const FString &Message)
{
    SetStatusText(Message);
    ShowOverlay();
}

void UMF_TransitionOverlay::PlayFadeAnimation(bool bFadeIn)
{
    // Simple opacity change - in a full implementation, you'd use UWidgetAnimation
    float TargetOpacity = bFadeIn ? 1.0f : 0.0f;
    SetRenderOpacity(TargetOpacity);

    // If using widget animations in UMG designer, you would call:
    // UWidgetAnimation* FadeAnim = bFadeIn ? FadeInAnimation : FadeOutAnimation;
    // if (FadeAnim)
    // {
    //     PlayAnimation(FadeAnim);
    // }
}
