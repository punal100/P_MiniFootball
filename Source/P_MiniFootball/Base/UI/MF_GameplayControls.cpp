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
