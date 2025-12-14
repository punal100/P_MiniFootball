/*
 * @Author: Punal Manalan
 * @Description: MF_ActionButton - Context-sensitive action button implementation
 * @Date: 10/12/2025
 */

#include "MF_ActionButton.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

FString UMF_ActionButton::GetWidgetSpec()
{
    static FString Spec = R"JSON({
    "WidgetClass": "UMF_ActionButton",
    "BlueprintName": "WBP_MF_ActionButton",
    "ParentClass": "/Script/P_MiniFootball.MF_ActionButton",
    "Category": "MF|UI|Controls",
    "Description": "Touch-friendly action button with icon and text support",
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
                    "Type": "Button",
                    "Name": "ActionButton",
                    "BindingType": "Required",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0.5, "Y": 0.5}, "Max": {"X": 0.5, "Y": 0.5}},
                        "Position": {"X": 0, "Y": 0},
                        "Size": {"X": 100, "Y": 100},
                        "Alignment": {"X": 0.5, "Y": 0.5}
                    },
                    "Children": [
                        {
                            "Type": "Overlay",
                            "Name": "ButtonContent",
                            "Children": [
                                {
                                    "Type": "Image",
                                    "Name": "ActionIcon",
                                    "BindingType": "Optional",
                                    "Slot": {"HAlign": "Center", "VAlign": "Center"}
                                },
                                {
                                    "Type": "TextBlock",
                                    "Name": "ActionText",
                                    "BindingType": "Optional",
                                    "Text": "ACTION",
                                    "FontSize": 14,
                                    "Justification": "Center",
                                    "Slot": {"HAlign": "Center", "VAlign": "Bottom", "Padding": {"Bottom": 5}}
                                }
                            ]
                        }
                    ]
                }
            ]
        }
    },
    
    "Design": {
        "ActionButton": {
            "Style": {
                "Normal": {"TintColor": {"R": 0.2, "G": 0.6, "B": 1.0, "A": 0.8}},
                "Hovered": {"TintColor": {"R": 0.3, "G": 0.7, "B": 1.0, "A": 0.9}},
                "Pressed": {"TintColor": {"R": 0.1, "G": 0.4, "B": 0.8, "A": 1.0}}
            },
            "IsFocusable": false
        },
        "ActionIcon": {
            "Size": {"X": 48, "Y": 48},
            "ColorAndOpacity": {"R": 1.0, "G": 1.0, "B": 1.0, "A": 1.0}
        },
        "ActionText": {
            "Font": {"Size": 14, "Typeface": "Bold"},
            "ColorAndOpacity": {"R": 1.0, "G": 1.0, "B": 1.0, "A": 1.0}
        }
    },
    
    "Bindings": {
        "Required": [
            {"Name": "ActionButton", "Type": "UButton", "Purpose": "Main interactive button"}
        ],
        "Optional": [
            {"Name": "ActionIcon", "Type": "UImage", "Purpose": "Icon display"},
            {"Name": "ActionText", "Type": "UTextBlock", "Purpose": "Button label text"}
        ]
    },
    
    "Delegates": [
        {
            "Name": "OnActionPressed",
            "Type": "FMF_OnActionButtonPressed",
            "Signature": "void()",
            "Description": "Fired when button is pressed down"
        },
        {
            "Name": "OnActionReleased", 
            "Type": "FMF_OnActionButtonReleased",
            "Signature": "void()",
            "Description": "Fired when button is released"
        }
    ],
    
    "Dependencies": [],
    
    "Comments": {
        "Header": "MF Action Button - Primary action input for mobile gameplay",
        "Usage": "Place in GameplayControls or as standalone action trigger"
    },
    
    "PythonSnippets": {
        "CreateRoot": "root_canvas = creator.add_widget('CanvasPanel', 'RootCanvas', None)",
        "CreateButton": "action_btn = creator.add_widget('Button', 'ActionButton', root_canvas, slot_data={'anchors': 'center', 'size': (100, 100)})",
        "BindWidgets": "creator.bind_widget('ActionButton', '/Script/UMG.Button')"
    }
})JSON";
    return Spec;
}

void UMF_ActionButton::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button events
    if (ActionButton)
    {
        ActionButton->OnPressed.AddDynamic(this, &UMF_ActionButton::HandleButtonPressed);
        ActionButton->OnReleased.AddDynamic(this, &UMF_ActionButton::HandleButtonReleased);
    }

    // Set initial state
    UpdateIcon();
    UpdateVisualState();
}

void UMF_ActionButton::NativeDestruct()
{
    // Unbind button events
    if (ActionButton)
    {
        ActionButton->OnPressed.RemoveDynamic(this, &UMF_ActionButton::HandleButtonPressed);
        ActionButton->OnReleased.RemoveDynamic(this, &UMF_ActionButton::HandleButtonReleased);
    }

    Super::NativeDestruct();
}

void UMF_ActionButton::NativeTick(const FGeometry &MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Visual feedback could be updated here if needed
}

void UMF_ActionButton::SetActionContext(EMF_ActionContext Context)
{
    if (CurrentContext != Context)
    {
        CurrentContext = Context;
        UpdateIcon();
    }
}

float UMF_ActionButton::GetHoldDuration() const
{
    if (!bIsPressed)
    {
        return 0.0f;
    }

    UWorld *World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }

    return World->GetTimeSeconds() - PressStartTime;
}

void UMF_ActionButton::HandleButtonPressed()
{
    bIsPressed = true;

    // Record press time
    UWorld *World = GetWorld();
    if (World)
    {
        PressStartTime = World->GetTimeSeconds();
    }

    UpdateVisualState();
    OnActionPressed.Broadcast();
}

void UMF_ActionButton::HandleButtonReleased()
{
    float HoldDuration = GetHoldDuration();
    bIsPressed = false;

    UpdateVisualState();
    OnActionReleased.Broadcast(HoldDuration);
}

void UMF_ActionButton::UpdateIcon()
{
    if (!ActionIcon)
    {
        return;
    }

    UTexture2D *IconTexture = nullptr;
    FString ActionLabel;

    switch (CurrentContext)
    {
    case EMF_ActionContext::Shoot:
        IconTexture = ShootIcon;
        ActionLabel = TEXT("SHOOT");
        break;
    case EMF_ActionContext::Pass:
        IconTexture = PassIcon;
        ActionLabel = TEXT("PASS");
        break;
    case EMF_ActionContext::Tackle:
        IconTexture = TackleIcon;
        ActionLabel = TEXT("TACKLE");
        break;
    default:
        ActionLabel = TEXT("ACTION");
        break;
    }

    // Set icon texture if available
    if (IconTexture)
    {
        ActionIcon->SetBrushFromTexture(IconTexture);
        ActionIcon->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        ActionIcon->SetVisibility(ESlateVisibility::Collapsed);
    }

    // Set action text
    if (ActionText)
    {
        ActionText->SetText(FText::FromString(ActionLabel));
    }
}

void UMF_ActionButton::UpdateVisualState()
{
    if (!ActionButton)
    {
        return;
    }

    // Update color based on pressed state
    FLinearColor Color = bIsPressed ? PressedColor : NormalColor;
    ActionButton->SetColorAndOpacity(Color);
}
