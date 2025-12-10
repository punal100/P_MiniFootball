/*
 * @Author: Punal Manalan
 * @Description: MF_VirtualJoystick - Touch joystick implementation
 * @Date: 10/12/2025
 */

#include "MF_VirtualJoystick.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"

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
