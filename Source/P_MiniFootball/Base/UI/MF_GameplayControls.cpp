/*
 * @Author: Punal Manalan
 * @Description: MF_GameplayControls - Container for active gameplay UI controls implementation
 * @Date: 10/12/2025
 */

#include "MF_GameplayControls.h"
#include "MF_VirtualJoystick.h"
#include "MF_ActionButton.h"
#include "MF_SprintButton.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Overlay.h"
#include "Player/MF_PlayerController.h"
#include "GenericPlatform/GenericPlatformMisc.h"

FString UMF_GameplayControls::GetWidgetSpec()
{
    static FString Spec = R"JSON({
    "WidgetClass": "UMF_GameplayControls",
    "BlueprintName": "WBP_MF_GameplayControls",
    "ParentClass": "/Script/P_MiniFootball.MF_GameplayControls",
    "Category": "MF|UI|HUD",
    "Description": "Touch controls container with joystick and action buttons",
    "Version": "1.0.0",

    "DesignerPreview": {
        "SizeMode": "FillScreen",
        "ZoomLevel": 10,
        "ShowGrid": true
    },
    
    "Hierarchy": {
        "Root": {
            "Type": "CanvasPanel",
            "Name": "RootCanvas",
            "Children": [
                {
                    "Type": "Overlay",
                    "Name": "LeftControlContainer",
                    "BindingType": "Optional",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0, "Y": 1}, "Max": {"X": 0, "Y": 1}},
                        "Position": {"X": 50, "Y": -50},
                        "Size": {"X": 200, "Y": 200},
                        "Alignment": {"X": 0, "Y": 1}
                    },
                    "Children": [
                        {
                            "Type": "UserWidget",
                            "Name": "MovementJoystick",
                            "BindingType": "Required",
                            "WidgetClass": "/Script/P_MiniFootball.MF_VirtualJoystick"
                        }
                    ]
                },
                {
                    "Type": "Overlay",
                    "Name": "RightControlContainer",
                    "BindingType": "Optional",
                    "Slot": {
                        "Anchors": {"Min": {"X": 1, "Y": 1}, "Max": {"X": 1, "Y": 1}},
                        "Position": {"X": -50, "Y": -50},
                        "Size": {"X": 200, "Y": 200},
                        "Alignment": {"X": 1, "Y": 1}
                    },
                    "Children": [
                        {
                            "Type": "UserWidget",
                            "Name": "ActionButton",
                            "BindingType": "Required",
                            "WidgetClass": "/Script/P_MiniFootball.MF_ActionButton",
                            "Slot": {"HAlign": "Right", "VAlign": "Bottom"}
                        },
                        {
                            "Type": "UserWidget",
                            "Name": "SprintButton",
                            "BindingType": "Optional",
                            "WidgetClass": "/Script/P_MiniFootball.MF_SprintButton",
                            "Slot": {"HAlign": "Right", "VAlign": "Top"}
                        }
                    ]
                }
            ]
        }
    },
    
    "Design": {
        "LeftControlContainer": {
            "Note": "Contains joystick on left side"
        },
        "RightControlContainer": {
            "Note": "Contains action buttons on right side"
        }
    },
    
    "Bindings": {
        "Required": [
            {"Name": "MovementJoystick", "Type": "UMF_VirtualJoystick", "Purpose": "Movement control"},
            {"Name": "ActionButton", "Type": "UMF_ActionButton", "Purpose": "Primary action"}
        ],
        "Optional": [
            {"Name": "SprintButton", "Type": "UMF_SprintButton", "Purpose": "Sprint toggle"},
            {"Name": "LeftControlContainer", "Type": "UOverlay", "Purpose": "Left-side container"},
            {"Name": "RightControlContainer", "Type": "UOverlay", "Purpose": "Right-side container"}
        ]
    },
    
    "Delegates": [],
    
    "Dependencies": [
        {"Class": "UMF_VirtualJoystick", "Blueprint": "WBP_MF_VirtualJoystick", "Required": true},
        {"Class": "UMF_ActionButton", "Blueprint": "WBP_MF_ActionButton", "Required": true},
        {"Class": "UMF_SprintButton", "Blueprint": "WBP_MF_SprintButton", "Required": false}
    ],
    
    "Comments": {
        "Header": "MF Gameplay Controls - Mobile touch control overlay",
        "Usage": "Shown in MF_HUD when player is on a team"
    },
    
    "PythonSnippets": {
        "CreateRoot": "root = creator.add_widget('CanvasPanel', 'RootCanvas', None)",
        "CreateLeftContainer": "left = creator.add_widget('Overlay', 'LeftControlContainer', root, slot_data={'anchors': 'bottom_left'})",
        "CreateRightContainer": "right = creator.add_widget('Overlay', 'RightControlContainer', root, slot_data={'anchors': 'bottom_right'})",
        "CreateJoystick": "joystick = creator.add_widget('UserWidget', 'MovementJoystick', left, widget_class='WBP_MF_VirtualJoystick')",
        "CreateAction": "action = creator.add_widget('UserWidget', 'ActionButton', right, widget_class='WBP_MF_ActionButton')"
    }
})JSON";
    return Spec;
}

void UMF_GameplayControls::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind joystick events
    if (MovementJoystick)
    {
        MovementJoystick->OnJoystickMoved.AddDynamic(this, &UMF_GameplayControls::HandleJoystickMoved);
        MovementJoystick->OnJoystickReleased.AddDynamic(this, &UMF_GameplayControls::HandleJoystickReleased);
    }

    // Bind action button events
    if (ActionButton)
    {
        ActionButton->OnActionPressed.AddDynamic(this, &UMF_GameplayControls::HandleActionPressed);
        ActionButton->OnActionReleased.AddDynamic(this, &UMF_GameplayControls::HandleActionReleased);
    }

    // Bind sprint button events
    if (SprintButton)
    {
        SprintButton->OnSprintStateChanged.AddDynamic(this, &UMF_GameplayControls::HandleSprintStateChanged);
    }

    // Initial refresh
    RefreshControlLayout();
}

void UMF_GameplayControls::NativeDestruct()
{
    // Unbind all delegates
    if (MovementJoystick)
    {
        MovementJoystick->OnJoystickMoved.RemoveDynamic(this, &UMF_GameplayControls::HandleJoystickMoved);
        MovementJoystick->OnJoystickReleased.RemoveDynamic(this, &UMF_GameplayControls::HandleJoystickReleased);
    }

    if (ActionButton)
    {
        ActionButton->OnActionPressed.RemoveDynamic(this, &UMF_GameplayControls::HandleActionPressed);
        ActionButton->OnActionReleased.RemoveDynamic(this, &UMF_GameplayControls::HandleActionReleased);
    }

    if (SprintButton)
    {
        SprintButton->OnSprintStateChanged.RemoveDynamic(this, &UMF_GameplayControls::HandleSprintStateChanged);
    }

    CachedPlayerController = nullptr;

    Super::NativeDestruct();
}

void UMF_GameplayControls::NativeTick(const FGeometry &MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Could update action context periodically here if needed
}

void UMF_GameplayControls::RefreshControlLayout()
{
    UpdateTouchVisibility();
    UpdateActionContext();
}

void UMF_GameplayControls::SetControlsEnabled(bool bEnabled)
{
    bControlsEnabled = bEnabled;

    // Enable/disable individual controls
    if (MovementJoystick)
    {
        MovementJoystick->SetIsEnabled(bEnabled);
        MovementJoystick->SetVisibility(bEnabled ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }

    if (ActionButton)
    {
        ActionButton->SetIsEnabled(bEnabled);
    }

    if (SprintButton)
    {
        SprintButton->SetIsEnabled(bEnabled);
    }

    // If disabling, ensure we release any held inputs
    if (!bEnabled)
    {
        HandleJoystickReleased();
        HandleSprintStateChanged(false);
    }
}

void UMF_GameplayControls::SetSprintButtonVisible(bool bVisible)
{
    if (SprintButton)
    {
        SprintButton->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UMF_GameplayControls::UpdateActionContext()
{
    if (!ActionButton)
    {
        return;
    }

    AMF_PlayerController *PC = GetMFPlayerController();
    if (!PC)
    {
        ActionButton->SetActionContext(EMF_ActionContext::None);
        return;
    }

    // Determine context based on player state
    // This could be expanded based on proximity to ball, goal, etc.
    ActionButton->SetActionContext(EMF_ActionContext::None);
}

void UMF_GameplayControls::HandleJoystickMoved(FVector2D Direction)
{
    if (!bControlsEnabled)
    {
        return;
    }

    AMF_PlayerController *PC = GetMFPlayerController();
    if (PC)
    {
        // Apply movement input through player controller
        PC->ApplyMobileMovementInput(Direction);
    }
}

void UMF_GameplayControls::HandleJoystickReleased()
{
    AMF_PlayerController *PC = GetMFPlayerController();
    if (PC)
    {
        // Zero out movement
        PC->ApplyMobileMovementInput(FVector2D::ZeroVector);
    }
}

void UMF_GameplayControls::HandleActionPressed()
{
    if (!bControlsEnabled)
    {
        return;
    }

    AMF_PlayerController *PC = GetMFPlayerController();
    if (PC)
    {
        PC->OnMobileActionPressed();
    }
}

void UMF_GameplayControls::HandleActionReleased(float HoldDuration)
{
    AMF_PlayerController *PC = GetMFPlayerController();
    if (PC)
    {
        PC->OnMobileActionReleased();
    }
}

void UMF_GameplayControls::HandleSprintStateChanged(bool bSprinting)
{
    if (!bControlsEnabled && bSprinting)
    {
        return;
    }

    AMF_PlayerController *PC = GetMFPlayerController();
    if (PC)
    {
        PC->SetMobileSprintState(bSprinting);
    }
}

AMF_PlayerController *UMF_GameplayControls::GetMFPlayerController()
{
    if (!CachedPlayerController)
    {
        APlayerController *PC = GetOwningPlayer();
        CachedPlayerController = Cast<AMF_PlayerController>(PC);
    }

    return CachedPlayerController;
}

bool UMF_GameplayControls::IsTouchDevice() const
{
    // Check if platform supports touch
    return FGenericPlatformMisc::SupportsTouchInput();
}

void UMF_GameplayControls::UpdateTouchVisibility()
{
    bool bShowTouchControls = IsTouchDevice();

    // Hide all touch controls on non-touch platforms
    if (LeftControlContainer)
    {
        LeftControlContainer->SetVisibility(bShowTouchControls ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    if (RightControlContainer)
    {
        RightControlContainer->SetVisibility(bShowTouchControls ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    // Individual control visibility
    if (MovementJoystick)
    {
        MovementJoystick->SetVisibility(bShowTouchControls ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    if (ActionButton)
    {
        ActionButton->SetVisibility(bShowTouchControls ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    if (SprintButton)
    {
        SprintButton->SetVisibility(bShowTouchControls ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}
