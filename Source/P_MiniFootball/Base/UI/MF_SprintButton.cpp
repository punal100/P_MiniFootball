/*
 * @Author: Punal Manalan
 * @Description: MF_SprintButton - Sprint hold button implementation
 * @Date: 10/12/2025
 */

#include "MF_SprintButton.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Core/MF_Types.h"
#include "Manager/CPP_InputBindingManager.h"

FString UMF_SprintButton::GetWidgetSpec()
{
    static FString Spec = R"JSON({
    "WidgetClass": "UMF_SprintButton",
    "BlueprintName": "WBP_MF_SprintButton",
    "ParentClass": "/Script/P_MiniFootball.MF_SprintButton",
    "Category": "MF|UI|Controls",
    "Description": "Toggle button for sprint functionality",
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
                    "Name": "SprintButton",
                    "BindingType": "Required",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0.5, "Y": 0.5}, "Max": {"X": 0.5, "Y": 0.5}},
                        "Position": {"X": 0, "Y": 0},
                        "Size": {"X": 80, "Y": 80},
                        "Alignment": {"X": 0.5, "Y": 0.5}
                    },
                    "Children": [
                        {
                            "Type": "Overlay",
                            "Name": "ButtonContent",
                            "Children": [
                                {
                                    "Type": "Image",
                                    "Name": "SprintIcon",
                                    "BindingType": "Optional",
                                    "Slot": {"HAlign": "Center", "VAlign": "Center"}
                                },
                                {
                                    "Type": "TextBlock",
                                    "Name": "SprintText",
                                    "BindingType": "Optional",
                                    "Text": "SPRINT",
                                    "FontSize": 12,
                                    "Justification": "Center",
                                    "Slot": {"HAlign": "Center", "VAlign": "Bottom"}
                                }
                            ]
                        }
                    ]
                }
            ]
        }
    },
    
    "Design": {
        "SprintButton": {
            "Style": {
                "Normal": {"TintColor": {"R": 1.0, "G": 1.0, "B": 1.0, "A": 0.7}},
                "Hovered": {"TintColor": {"R": 1.0, "G": 1.0, "B": 1.0, "A": 0.85}},
                "Pressed": {"TintColor": {"R": 0.2, "G": 0.8, "B": 0.2, "A": 1.0}}
            },
            "IsFocusable": false
        },
        "SprintIcon": {
            "Size": {"X": 32, "Y": 32},
            "ColorAndOpacity": {"R": 1.0, "G": 1.0, "B": 1.0, "A": 1.0}
        },
        "SprintText": {
            "Font": {"Size": 12, "Typeface": "Regular"},
            "Text": "Sprint"
        }
    },
    
    "Bindings": {
        "Required": [
            {"Name": "SprintButton", "Type": "UButton", "Purpose": "Main toggle button for sprint"}
        ],
        "Optional": [
            {"Name": "SprintIcon", "Type": "UImage", "Purpose": "Sprint state icon"},
            {"Name": "SprintText", "Type": "UTextBlock", "Purpose": "Sprint label"}
        ]
    },
    
    "Delegates": [
        {
            "Name": "OnSprintStateChanged",
            "Type": "FMF_OnSprintStateChanged",
            "Signature": "void(bool bIsSprinting)",
            "Description": "Fired when sprint state toggles"
        }
    ],
    
    "Dependencies": [],
    
    "Comments": {
        "Header": "MF Sprint Button - Toggle control for sprint mode",
        "Usage": "Place in GameplayControls as optional sprint toggle"
    },
    
    "PythonSnippets": {
        "CreateRoot": "root = creator.add_widget('CanvasPanel', 'RootCanvas', None)",
        "CreateButton": "btn = creator.add_widget('Button', 'SprintButton', root, slot_data={'anchors': 'center', 'size': (80, 80)})",
        "BindWidgets": "creator.bind_widget('SprintButton', '/Script/UMG.Button')"
    }
})JSON";
    return Spec;
}

void UMF_SprintButton::NativeConstruct()
{
    Super::NativeConstruct();

    UpdateToggleModeFromProfile();

    // Bind button events
    if (SprintButton)
    {
        SprintButton->OnPressed.AddDynamic(this, &UMF_SprintButton::HandleButtonPressed);
        SprintButton->OnReleased.AddDynamic(this, &UMF_SprintButton::HandleButtonReleased);
    }

    // Set initial state
    UpdateVisualState();
}

void UMF_SprintButton::NativeDestruct()
{
    // Unbind button events
    if (SprintButton)
    {
        SprintButton->OnPressed.RemoveDynamic(this, &UMF_SprintButton::HandleButtonPressed);
        SprintButton->OnReleased.RemoveDynamic(this, &UMF_SprintButton::HandleButtonReleased);
    }

    Super::NativeDestruct();
}

void UMF_SprintButton::HandleButtonPressed()
{
    UpdateToggleModeFromProfile();

    if (bUseToggleMode)
    {
        bIsSprinting = !bIsSprinting;
        UpdateVisualState();
        OnSprintStateChanged.Broadcast(bIsSprinting);
        return;
    }

    if (!bIsSprinting)
    {
        bIsSprinting = true;
        UpdateVisualState();
        OnSprintStateChanged.Broadcast(true);
    }
}

void UMF_SprintButton::HandleButtonReleased()
{
    UpdateToggleModeFromProfile();

    // Toggle mode: release does not change state.
    if (bUseToggleMode)
    {
        return;
    }

    if (bIsSprinting)
    {
        bIsSprinting = false;
        UpdateVisualState();
        OnSprintStateChanged.Broadcast(false);
    }
}

void UMF_SprintButton::UpdateToggleModeFromProfile()
{
    bUseToggleMode = false;

    APlayerController *PC = GetOwningPlayer();
    if (!PC)
    {
        return;
    }

    UCPP_InputBindingManager *Manager = GEngine ? GEngine->GetEngineSubsystem<UCPP_InputBindingManager>() : nullptr;
    if (!Manager)
    {
        return;
    }

    FS_InputProfile *Profile = Manager->GetProfileRefForPlayer(PC);
    if (!Profile)
    {
        return;
    }

    bUseToggleMode = Profile->ToggleModeActions.Contains(MF_InputActions::Sprint);
}

void UMF_SprintButton::UpdateVisualState()
{
    FLinearColor Color = bIsSprinting ? SprintingColor : NormalColor;

    if (SprintButton)
    {
        SprintButton->SetColorAndOpacity(Color);
    }

    if (SprintIcon)
    {
        SprintIcon->SetColorAndOpacity(Color);
    }

    if (SprintText)
    {
        FString Text = bIsSprinting ? TEXT("SPRINTING") : TEXT("SPRINT");
        SprintText->SetText(FText::FromString(Text));
    }
}
