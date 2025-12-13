/*
 * @Author: Punal Manalan
 * @Description: MF_VirtualJoystick - Touch joystick implementation
 * @Date: 10/12/2025
 */

#include "MF_VirtualJoystick.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"

FString UMF_VirtualJoystick::GetWidgetSpec()
{
    static FString Spec = R"JSON({
    "WidgetClass": "UMF_VirtualJoystick",
    "BlueprintName": "WBP_MF_VirtualJoystick",
    "ParentClass": "/Script/P_MiniFootball.MF_VirtualJoystick",
    "Category": "MF|UI|Controls",
    "Description": "Touch-friendly virtual joystick for movement input",
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
                    "Name": "JoystickBase",
                    "BindingType": "Required",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0.5, "Y": 0.5}, "Max": {"X": 0.5, "Y": 0.5}},
                        "Position": {"X": 0, "Y": 0},
                        "Size": {"X": 150, "Y": 150},
                        "Alignment": {"X": 0.5, "Y": 0.5}
                    }
                },
                {
                    "Type": "Image",
                    "Name": "JoystickThumb",
                    "BindingType": "Required",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0.5, "Y": 0.5}, "Max": {"X": 0.5, "Y": 0.5}},
                        "Position": {"X": 0, "Y": 0},
                        "Size": {"X": 60, "Y": 60},
                        "Alignment": {"X": 0.5, "Y": 0.5}
                    }
                }
            ]
        }
    },
    
    "Design": {
        "JoystickBase": {
            "Brush": {"DrawAs": "Image", "TintColor": {"R": 0.3, "G": 0.3, "B": 0.3, "A": 0.6}},
            "Size": {"X": 150, "Y": 150}
        },
        "JoystickThumb": {
            "Brush": {"DrawAs": "Image", "TintColor": {"R": 0.8, "G": 0.8, "B": 0.8, "A": 0.9}},
            "Size": {"X": 60, "Y": 60}
        }
    },
    
    "Bindings": {
        "Required": [
            {"Name": "JoystickBase", "Type": "UImage", "Purpose": "Background circle/base of joystick"},
            {"Name": "JoystickThumb", "Type": "UImage", "Purpose": "Movable thumb/stick indicator"}
        ],
        "Optional": []
    },
    
    "Delegates": [
        {
            "Name": "OnJoystickMoved",
            "Type": "FMF_OnJoystickMoved",
            "Signature": "void(FVector2D Direction, float Magnitude)",
            "Description": "Fired continuously while joystick is being moved"
        },
        {
            "Name": "OnJoystickReleased",
            "Type": "FMF_OnJoystickReleased",
            "Signature": "void()",
            "Description": "Fired when joystick is released"
        }
    ],
    
    "Dependencies": [],
    
    "Comments": {
        "Header": "MF Virtual Joystick - Movement control for mobile touch input",
        "Usage": "Place in GameplayControls left side for player movement"
    },
    
    "PythonSnippets": {
        "CreateRoot": "root = creator.add_widget('CanvasPanel', 'RootCanvas', None)",
        "CreateBase": "base = creator.add_widget('Image', 'JoystickBase', root, slot_data={'anchors': 'center', 'size': (150, 150)})",
        "CreateThumb": "thumb = creator.add_widget('Image', 'JoystickThumb', base, slot_data={'anchors': 'center', 'size': (60, 60)})",
        "BindWidgets": "creator.bind_widget('JoystickBase', '/Script/UMG.Image'); creator.bind_widget('JoystickThumb', '/Script/UMG.Image')"
    }
})JSON";
    return Spec;
}

void UMF_VirtualJoystick::NativeConstruct()
{
    Super::NativeConstruct();

    // Store center position
    CenterPosition = FVector2D::ZeroVector;
}

FReply UMF_VirtualJoystick::NativeOnTouchStarted(const FGeometry &InGeometry, const FPointerEvent &InGestureEvent)
{
    bIsPressed = true;

    // Calculate center of widget
    FVector2D LocalSize = InGeometry.GetLocalSize();
    CenterPosition = LocalSize * 0.5f;

    // Process touch position
    FVector2D TouchPos = InGeometry.AbsoluteToLocal(InGestureEvent.GetScreenSpacePosition());
    UpdateJoystickPosition(TouchPos, InGeometry);

    return FReply::Handled().CaptureMouse(GetCachedWidget().ToSharedRef());
}

FReply UMF_VirtualJoystick::NativeOnTouchMoved(const FGeometry &InGeometry, const FPointerEvent &InGestureEvent)
{
    if (bIsPressed)
    {
        FVector2D TouchPos = InGeometry.AbsoluteToLocal(InGestureEvent.GetScreenSpacePosition());
        UpdateJoystickPosition(TouchPos, InGeometry);
    }

    return FReply::Handled();
}

FReply UMF_VirtualJoystick::NativeOnTouchEnded(const FGeometry &InGeometry, const FPointerEvent &InGestureEvent)
{
    bIsPressed = false;
    ResetJoystick();
    OnJoystickReleased.Broadcast();

    return FReply::Handled().ReleaseMouseCapture();
}

void UMF_VirtualJoystick::UpdateJoystickPosition(const FVector2D &TouchPosition, const FGeometry &Geometry)
{
    // Calculate offset from center
    FVector2D LocalSize = Geometry.GetLocalSize();
    FVector2D Center = LocalSize * 0.5f;
    FVector2D Offset = TouchPosition - Center;

    // Calculate magnitude
    float Distance = Offset.Size();
    float MaxDistance = MaxThumbOffset;

    // Clamp to max distance
    if (Distance > MaxDistance)
    {
        Offset = Offset.GetSafeNormal() * MaxDistance;
        Distance = MaxDistance;
    }

    // Calculate normalized magnitude (0-1)
    CurrentMagnitude = Distance / MaxDistance;

    // Apply dead zone
    if (CurrentMagnitude < DeadZone)
    {
        CurrentDirection = FVector2D::ZeroVector;
        CurrentMagnitude = 0.0f;
        UpdateThumbVisual(FVector2D::ZeroVector);
        return;
    }

    // Remap magnitude to account for dead zone
    CurrentMagnitude = (CurrentMagnitude - DeadZone) / (1.0f - DeadZone);
    CurrentMagnitude = FMath::Clamp(CurrentMagnitude, 0.0f, 1.0f);

    // Calculate direction
    CurrentDirection = Offset.GetSafeNormal();

    // Update visual
    UpdateThumbVisual(Offset);

    // Broadcast direction (includes magnitude in the direction vector)
    OnJoystickMoved.Broadcast(CurrentDirection * CurrentMagnitude);
}

void UMF_VirtualJoystick::ResetJoystick()
{
    CurrentDirection = FVector2D::ZeroVector;
    CurrentMagnitude = 0.0f;
    UpdateThumbVisual(FVector2D::ZeroVector);
}

void UMF_VirtualJoystick::UpdateThumbVisual(const FVector2D &Offset)
{
    if (!JoystickThumb)
    {
        return;
    }

    // Move thumb to new position
    // This assumes the thumb is a child of a canvas panel with anchors at center
    UCanvasPanelSlot *ThumbSlot = Cast<UCanvasPanelSlot>(JoystickThumb->Slot);
    if (ThumbSlot)
    {
        ThumbSlot->SetPosition(Offset);
    }
    else
    {
        // Alternative: Use render transform
        JoystickThumb->SetRenderTranslation(Offset);
    }
}
