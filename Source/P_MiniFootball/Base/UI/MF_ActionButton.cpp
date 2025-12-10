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
