/*
 * @Author: Punal Manalan
 * @Description: MF_InputActionRow - Runtime-created input binding row implementation
 * @Date: 14/12/2025
 */

#include "MF_InputActionRow.h"

#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"

void UMF_InputActionRow::NativeConstruct()
{
    Super::NativeConstruct();

    BuildWidgetTreeIfNeeded();

    if (RebindButton)
    {
        RebindButton->OnClicked.AddDynamic(this, &UMF_InputActionRow::HandleRebindClicked);
    }
}

void UMF_InputActionRow::BuildWidgetTreeIfNeeded()
{
    if (RootRow)
    {
        return;
    }

    // Build a minimal row:
    // [ActionLabel] [ModeLabel] [KeyLabel] [RebindButton]
    RootRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("RootRow"));
    WidgetTree->RootWidget = RootRow;

    ActionLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ActionLabel"));
    ModeLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ModeLabel"));
    KeyLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("KeyLabel"));
    RebindButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RebindButton"));

    UTextBlock *RebindText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RebindText"));
    RebindText->SetText(FText::FromString(TEXT("Rebind")));
    RebindText->SetJustification(ETextJustify::Center);
    RebindButton->AddChild(RebindText);

    RootRow->AddChildToHorizontalBox(ActionLabel);
    RootRow->AddChildToHorizontalBox(ModeLabel);
    RootRow->AddChildToHorizontalBox(KeyLabel);
    RootRow->AddChildToHorizontalBox(RebindButton);

    // Basic layout widths
    if (UHorizontalBoxSlot *RowSlot = Cast<UHorizontalBoxSlot>(ActionLabel->Slot))
    {
        RowSlot->SetSize(ESlateSizeRule::Fill);
        RowSlot->SetPadding(FMargin(4.f, 2.f));
        RowSlot->SetHorizontalAlignment(HAlign_Left);
        RowSlot->SetVerticalAlignment(VAlign_Center);
    }

    if (UHorizontalBoxSlot *RowSlot = Cast<UHorizontalBoxSlot>(ModeLabel->Slot))
    {
        RowSlot->SetSize(ESlateSizeRule::Automatic);
        RowSlot->SetPadding(FMargin(8.f, 2.f));
        RowSlot->SetHorizontalAlignment(HAlign_Left);
        RowSlot->SetVerticalAlignment(VAlign_Center);
    }

    if (UHorizontalBoxSlot *RowSlot = Cast<UHorizontalBoxSlot>(KeyLabel->Slot))
    {
        RowSlot->SetSize(ESlateSizeRule::Automatic);
        RowSlot->SetPadding(FMargin(8.f, 2.f));
        RowSlot->SetHorizontalAlignment(HAlign_Left);
        RowSlot->SetVerticalAlignment(VAlign_Center);
    }

    if (UHorizontalBoxSlot *RowSlot = Cast<UHorizontalBoxSlot>(RebindButton->Slot))
    {
        RowSlot->SetSize(ESlateSizeRule::Automatic);
        RowSlot->SetPadding(FMargin(12.f, 2.f));
        RowSlot->SetHorizontalAlignment(HAlign_Right);
        RowSlot->SetVerticalAlignment(VAlign_Center);
    }
}

void UMF_InputActionRow::SetActionBinding(const FS_InputActionBinding &InBinding, bool bIsToggleMode)
{
    BuildWidgetTreeIfNeeded();

    bIsAxis = false;
    BindingName = InBinding.InputActionName;

    if (ActionLabel)
    {
        const FText Label = !InBinding.DisplayName.IsEmpty() ? InBinding.DisplayName : FText::FromName(InBinding.InputActionName);
        ActionLabel->SetText(Label);
    }

    SetModeDisplay(bIsToggleMode ? FText::FromString(TEXT("(Toggle)")) : FText::GetEmpty());
}

void UMF_InputActionRow::SetAxisBinding(const FS_InputAxisBinding &InBinding)
{
    BuildWidgetTreeIfNeeded();

    bIsAxis = true;
    BindingName = InBinding.InputAxisName;

    if (ActionLabel)
    {
        const FText Label = !InBinding.DisplayName.IsEmpty() ? InBinding.DisplayName : FText::FromName(InBinding.InputAxisName);
        ActionLabel->SetText(Label);
    }

    SetModeDisplay(FText::GetEmpty());
}

void UMF_InputActionRow::SetKeyDisplay(const FText &InKeyDisplay)
{
    BuildWidgetTreeIfNeeded();

    if (KeyLabel)
    {
        KeyLabel->SetText(InKeyDisplay);
    }
}

void UMF_InputActionRow::SetModeDisplay(const FText &InModeDisplay)
{
    BuildWidgetTreeIfNeeded();

    if (ModeLabel)
    {
        ModeLabel->SetText(InModeDisplay);
    }
}

void UMF_InputActionRow::SetRebinding(bool bInRebinding)
{
    bIsRebinding = bInRebinding;

    if (RebindButton)
    {
        RebindButton->SetIsEnabled(!bIsRebinding);
    }

    if (KeyLabel && bIsRebinding)
    {
        KeyLabel->SetText(FText::FromString(TEXT("[Press a key...]")));
    }
}

void UMF_InputActionRow::HandleRebindClicked()
{
    OnRebindRequested.Broadcast(bIsAxis, BindingName);
}
