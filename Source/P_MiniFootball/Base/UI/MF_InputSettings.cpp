/*
 * @Author: Punal Manalan
 * @Description: MF_InputSettings - Modular input rebinding overlay implementation
 * @Date: 14/12/2025
 */

#include "MF_InputSettings.h"

#include "MF_InputActionRow.h"

#include "UI/Configuration/MF_WidgetConfigurationSubsystem.h"
#include "UI/Configuration/MF_WidgetTypes.h"

#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"

#include "Manager/CPP_InputBindingManager.h"

namespace
{
    static UCPP_InputBindingManager *GetMEISManager()
    {
        return GEngine ? GEngine->GetEngineSubsystem<UCPP_InputBindingManager>() : nullptr;
    }

    static FName MakeTimestampTemplateName()
    {
        const FString Stamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
        return FName(*FString::Printf(TEXT("Player_%s"), *Stamp));
    }

    static FString JoinStrings(const TArray<FString> &Items, const FString &Separator)
    {
        FString Out;
        for (int32 Index = 0; Index < Items.Num(); ++Index)
        {
            if (Index > 0)
            {
                Out += Separator;
            }
            Out += Items[Index];
        }
        return Out;
    }
}

FString UMF_InputSettings::GetWidgetSpec()
{
    static FString Spec = R"JSON({
    "WidgetClass": "UMF_InputSettings",
    "BlueprintName": "WBP_MF_InputSettings",
    "ParentClass": "/Script/P_MiniFootball.MF_InputSettings",
    "Category": "MF|UI|Menus",
    "Description": "Input settings overlay with dynamic action list",
    "Version": "1.0.0",

    "DesignerPreview": {"SizeMode": "FillScreen", "ZoomLevel": 14, "ShowGrid": true},

    "Hierarchy": {
        "Root": {
            "Type": "CanvasPanel",
            "Name": "RootCanvas",
            "Children": [
                {"Type": "Overlay", "Name": "BackgroundOverlay", "Slot": {"Anchors": {"Min": {"X": 0, "Y": 0}, "Max": {"X": 1, "Y": 1}}, "Offsets": {"Left": 0, "Top": 0, "Right": 0, "Bottom": 0}}},
                {
                    "Type": "VerticalBox",
                    "Name": "InputContainer",
                    "Slot": {"Anchors": {"Min": {"X": 0.5, "Y": 0.5}, "Max": {"X": 0.5, "Y": 0.5}}, "Alignment": {"X": 0.5, "Y": 0.5}},
                    "Children": [
                        {"Type": "TextBlock", "Name": "InputSettingsTitle", "BindingType": "Optional", "Text": "INPUT SETTINGS", "Font": {"Size": 28, "Typeface": "Bold"}, "Slot": {"HAlign": "Center", "Padding": {"Bottom": 10}}},
                        {"Type": "ScrollBox", "Name": "ActionListScroll", "BindingType": "Required", "Slot": {"HAlign": "Fill", "Padding": {"Bottom": 12}}},
                        {
                            "Type": "HorizontalBox",
                            "Name": "ButtonRow",
                            "Slot": {"HAlign": "Center"},
                            "Children": [
                                {"Type": "Button", "Name": "SaveButton", "BindingType": "Required", "Slot": {"HAlign": "Center", "Padding": {"Right": 10}}, "Children": [{"Type": "TextBlock", "Name": "SaveLabel", "Text": "SAVE", "Justification": "Center"}]},
                                {"Type": "Button", "Name": "CancelButton", "BindingType": "Required", "Slot": {"HAlign": "Center"}, "Children": [{"Type": "TextBlock", "Name": "CancelLabel", "Text": "CANCEL", "Justification": "Center"}]}
                            ]
                        }
                    ]
                }
            ]
        }
    },

    "Bindings": {
        "Required": [
            {"Name": "ActionListScroll", "Type": "UScrollBox"},
            {"Name": "SaveButton", "Type": "UButton"},
            {"Name": "CancelButton", "Type": "UButton"}
        ],
        "Optional": [
            {"Name": "InputSettingsTitle", "Type": "UTextBlock"}
        ]
    }
})JSON";

    return Spec;
}

void UMF_InputSettings::NativeConstruct()
{
    Super::NativeConstruct();

    SetIsFocusable(true);

    if (SaveButton)
    {
        SaveButton->OnClicked.AddDynamic(this, &UMF_InputSettings::HandleSaveClicked);
    }
    if (CancelButton)
    {
        CancelButton->OnClicked.AddDynamic(this, &UMF_InputSettings::HandleCancelClicked);
    }

    LoadProfileForEditing();
    RebuildRows();

    SetVisibility(ESlateVisibility::Collapsed);
}

void UMF_InputSettings::NativeDestruct()
{
    if (SaveButton)
    {
        SaveButton->OnClicked.RemoveDynamic(this, &UMF_InputSettings::HandleSaveClicked);
    }
    if (CancelButton)
    {
        CancelButton->OnClicked.RemoveDynamic(this, &UMF_InputSettings::HandleCancelClicked);
    }

    Super::NativeDestruct();
}

FReply UMF_InputSettings::NativeOnKeyDown(const FGeometry &InGeometry, const FKeyEvent &InKeyEvent)
{
    if (RebindMode == ERebindMode::None)
    {
        return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
    }

    const FKey PressedKey = InKeyEvent.GetKey();
    if (PressedKey == EKeys::Escape)
    {
        CancelRebind();
        RebuildRows();
        SetKeyboardFocus();
        return FReply::Handled();
    }

    ApplyCapturedKey(PressedKey);
    CancelRebind();
    RebuildRows();
    SetKeyboardFocus();
    return FReply::Handled();
}

void UMF_InputSettings::Show()
{
    SetVisibility(ESlateVisibility::Visible);
    LoadProfileForEditing();
    RebuildRows();
    SetKeyboardFocus();
}

void UMF_InputSettings::Hide()
{
    CancelRebind();
    SetVisibility(ESlateVisibility::Collapsed);
    OnClosed.Broadcast();
}

void UMF_InputSettings::LoadProfileForEditing()
{
    bHasPendingProfile = false;

    UCPP_InputBindingManager *Manager = GetMEISManager();
    APlayerController *PC = GetOwningPlayer();
    if (!Manager || !PC)
    {
        return;
    }

    if (!Manager->HasPlayerRegistered(PC))
    {
        Manager->RegisterPlayer(PC);
    }

    FS_InputProfile *Profile = Manager->GetProfileRefForPlayer(PC);
    if (!Profile)
    {
        return;
    }

    PendingProfile = *Profile;
    bHasPendingProfile = true;
}

void UMF_InputSettings::RebuildRows()
{
    if (!ActionListScroll)
    {
        return;
    }

    ActionListScroll->ClearChildren();

    if (!bHasPendingProfile)
    {
        return;
    }

    TSubclassOf<UMF_InputActionRow> RowClass = InputActionRowClassOverride;
    if (!RowClass)
    {
        if (GEngine)
        {
            if (UMF_WidgetConfigurationSubsystem *WidgetConfig = GEngine->GetEngineSubsystem<UMF_WidgetConfigurationSubsystem>())
            {
                const TSubclassOf<UUserWidget> AnyRowClass = WidgetConfig->GetWidgetClass(EMF_WidgetType::InputActionRow);
                if (AnyRowClass && AnyRowClass->IsChildOf(UMF_InputActionRow::StaticClass()))
                {
                    RowClass = AnyRowClass.Get();
                }
            }
        }
    }
    if (!RowClass)
    {
        RowClass = UMF_InputActionRow::StaticClass();
    }

    // Actions
    for (int32 Index = 0; Index < PendingProfile.ActionBindings.Num(); ++Index)
    {
        const FS_InputActionBinding &Binding = PendingProfile.ActionBindings[Index];
        if (!Binding.bEnabled)
        {
            continue;
        }

        UMF_InputActionRow *Row = nullptr;
        if (APlayerController *PC = GetOwningPlayer())
        {
            Row = CreateWidget<UMF_InputActionRow>(PC, RowClass);
        }
        else if (UWorld *World = GetWorld())
        {
            Row = CreateWidget<UMF_InputActionRow>(World, RowClass);
        }
        if (!Row)
        {
            continue;
        }

        const bool bToggleMode = IsActionToggleMode(Binding.InputActionName);
        Row->SetActionBinding(Binding, bToggleMode);
        Row->SetKeyDisplay(MakeActionKeyDisplay(Binding.KeyBindings));
        Row->OnRebindRequested.AddDynamic(this, &UMF_InputSettings::HandleRowRebindRequested);

        ActionListScroll->AddChild(Row);

        if (RebindMode == ERebindMode::Action && PendingIndex == Index)
        {
            Row->SetRebinding(true);
            PendingRow = Row;
        }
    }

    // Axes
    const int32 AxisBaseIndex = PendingProfile.ActionBindings.Num();
    for (int32 AxisIndex = 0; AxisIndex < PendingProfile.AxisBindings.Num(); ++AxisIndex)
    {
        const FS_InputAxisBinding &Binding = PendingProfile.AxisBindings[AxisIndex];
        if (!Binding.bEnabled)
        {
            continue;
        }

        UMF_InputActionRow *Row = nullptr;
        if (APlayerController *PC = GetOwningPlayer())
        {
            Row = CreateWidget<UMF_InputActionRow>(PC, RowClass);
        }
        else if (UWorld *World = GetWorld())
        {
            Row = CreateWidget<UMF_InputActionRow>(World, RowClass);
        }
        if (!Row)
        {
            continue;
        }

        Row->SetAxisBinding(Binding);
        Row->SetKeyDisplay(MakeAxisKeyDisplay(Binding.AxisBindings));
        Row->OnRebindRequested.AddDynamic(this, &UMF_InputSettings::HandleRowRebindRequested);

        ActionListScroll->AddChild(Row);

        if (RebindMode == ERebindMode::Axis && PendingIndex == AxisIndex)
        {
            Row->SetRebinding(true);
            PendingRow = Row;
        }
    }
}

FText UMF_InputSettings::MakeActionKeyDisplay(const TArray<FS_KeyBinding> &Keys) const
{
    if (Keys.Num() <= 0)
    {
        return FText::FromString(TEXT("[Unbound]"));
    }

    TArray<FString> Parts;
    Parts.Reserve(Keys.Num());

    for (const FS_KeyBinding &KeyBinding : Keys)
    {
        Parts.Add(KeyBinding.Key.GetDisplayName().ToString());
    }

    return FText::FromString(FString::Printf(TEXT("[%s]"), *JoinStrings(Parts, TEXT(","))));
}

FText UMF_InputSettings::MakeAxisKeyDisplay(const TArray<FS_AxisKeyBinding> &Keys) const
{
    if (Keys.Num() <= 0)
    {
        return FText::FromString(TEXT("[Unbound]"));
    }

    TArray<FString> Parts;
    Parts.Reserve(Keys.Num());

    for (const FS_AxisKeyBinding &KeyBinding : Keys)
    {
        Parts.Add(KeyBinding.Key.GetDisplayName().ToString());
    }

    return FText::FromString(FString::Printf(TEXT("[%s]"), *JoinStrings(Parts, TEXT(","))));
}

bool UMF_InputSettings::IsActionToggleMode(const FName &ActionName) const
{
    return bHasPendingProfile && PendingProfile.ToggleModeActions.Contains(ActionName);
}

void UMF_InputSettings::BeginRebindAction(int32 ActionIndex)
{
    CancelRebind();
    RebindMode = ERebindMode::Action;
    PendingIndex = ActionIndex;
}

void UMF_InputSettings::BeginRebindAxis(int32 AxisIndex)
{
    CancelRebind();
    RebindMode = ERebindMode::Axis;
    PendingIndex = AxisIndex;
}

void UMF_InputSettings::CancelRebind()
{
    RebindMode = ERebindMode::None;
    PendingIndex = INDEX_NONE;
    PendingRow = nullptr;
}

void UMF_InputSettings::ApplyCapturedKey(const FKey &PressedKey)
{
    if (!bHasPendingProfile)
    {
        return;
    }

    if (RebindMode == ERebindMode::Action)
    {
        if (!PendingProfile.ActionBindings.IsValidIndex(PendingIndex))
        {
            return;
        }

        FS_InputActionBinding &Binding = PendingProfile.ActionBindings[PendingIndex];
        if (Binding.KeyBindings.Num() <= 0)
        {
            Binding.KeyBindings.Add(FS_KeyBinding());
        }

        Binding.KeyBindings[0].Key = PressedKey;
        return;
    }

    if (RebindMode == ERebindMode::Axis)
    {
        if (!PendingProfile.AxisBindings.IsValidIndex(PendingIndex))
        {
            return;
        }

        FS_InputAxisBinding &Binding = PendingProfile.AxisBindings[PendingIndex];
        if (Binding.AxisBindings.Num() <= 0)
        {
            Binding.AxisBindings.Add(FS_AxisKeyBinding());
        }

        Binding.AxisBindings[0].Key = PressedKey;
        return;
    }
}

void UMF_InputSettings::HandleRowRebindRequested(bool bIsAxisBinding, FName BindingName)
{
    if (!bHasPendingProfile)
    {
        return;
    }

    if (bIsAxisBinding)
    {
        for (int32 Index = 0; Index < PendingProfile.AxisBindings.Num(); ++Index)
        {
            if (PendingProfile.AxisBindings[Index].InputAxisName == BindingName)
            {
                BeginRebindAxis(Index);
                RebuildRows();
                SetKeyboardFocus();
                return;
            }
        }
        return;
    }

    for (int32 Index = 0; Index < PendingProfile.ActionBindings.Num(); ++Index)
    {
        if (PendingProfile.ActionBindings[Index].InputActionName == BindingName)
        {
            BeginRebindAction(Index);
            RebuildRows();
            SetKeyboardFocus();
            return;
        }
    }
}

void UMF_InputSettings::HandleSaveClicked()
{
    UCPP_InputBindingManager *Manager = GetMEISManager();
    APlayerController *PC = GetOwningPlayer();
    if (!Manager || !PC || !bHasPendingProfile)
    {
        HandleCancelClicked();
        return;
    }

    if (!Manager->HasPlayerRegistered(PC))
    {
        Manager->RegisterPlayer(PC);
    }

    FS_InputProfile *LiveProfile = Manager->GetProfileRefForPlayer(PC);
    if (!LiveProfile)
    {
        HandleCancelClicked();
        return;
    }

    *LiveProfile = PendingProfile;

    const FName TemplateToSave = TargetTemplateName.IsNone() ? MakeTimestampTemplateName() : TargetTemplateName;
    Manager->SavePlayerProfileAsTemplate(PC, TemplateToSave);
    Manager->ApplyPlayerProfileToEnhancedInput(PC);

    Hide();
}

void UMF_InputSettings::HandleCancelClicked()
{
    Hide();
}
