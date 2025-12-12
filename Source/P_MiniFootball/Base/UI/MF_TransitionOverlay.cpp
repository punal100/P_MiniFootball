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
    
    "DesignerToolbar": {
        "DesiredSize": {"Width": 800, "Height": 600},
        "ZoomLevel": "1:2",
        "ShowGrid": false
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
                        "Offsets": {"Left": 0, "Top": 0, "Right": 0, "Bottom": 0}
                    }
                },
                {
                    "Type": "VerticalBox",
                    "Name": "ContentBox",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0.5, "Y": 0.5}, "Max": {"X": 0.5, "Y": 0.5}},
                        "Alignment": {"X": 0.5, "Y": 0.5}
                    },
                    "Children": [
                        {
                            "Type": "Throbber",
                            "Name": "LoadingThrobber",
                            "BindingType": "Optional",
                            "Slot": {"HAlign": "Center", "Padding": {"Bottom": 20}}
                        },
                        {
                            "Type": "TextBlock",
                            "Name": "StatusText",
                            "BindingType": "Required",
                            "Slot": {"HAlign": "Center"}
                        }
                    ]
                }
            ]
        }
    },
    
    "Design": {
        "BackgroundOverlay": {
            "Brush": {"DrawAs": "Box"},
            "ColorAndOpacity": {"R": 0, "G": 0, "B": 0, "A": 0.85}
        },
        "LoadingThrobber": {
            "NumberOfPieces": 8
        },
        "StatusText": {
            "Font": {"Size": 24, "Typeface": "Regular"},
            "ColorAndOpacity": {"R": 1.0, "G": 1.0, "B": 1.0, "A": 1.0},
            "Justification": "Center",
            "Text": "Loading..."
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
    
    "Dependencies": [],
    
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

    // Set default message
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
