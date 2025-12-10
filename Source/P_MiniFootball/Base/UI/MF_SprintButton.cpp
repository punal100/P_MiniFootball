/*
 * @Author: Punal Manalan
 * @Description: MF_SprintButton - Sprint hold button implementation
 * @Date: 10/12/2025
 */

#include "MF_SprintButton.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UMF_SprintButton::NativeConstruct()
{
    Super::NativeConstruct();

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
    if (!bIsSprinting)
    {
        bIsSprinting = true;
        UpdateVisualState();
        OnSprintStateChanged.Broadcast(true);
    }
}

void UMF_SprintButton::HandleButtonReleased()
{
    if (bIsSprinting)
    {
        bIsSprinting = false;
        UpdateVisualState();
        OnSprintStateChanged.Broadcast(false);
    }
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
