/*
 * @Author: Punal Manalan
 * @Description: MF_InputSettings - Modular input rebinding overlay implementation
 * @Date: 14/12/2025
 */

#include "MF_InputSettings.h"

#include "MF_InputActionRow.h"

#include "Player/MF_PlayerController.h"

#include "UI/Configuration/MF_WidgetConfigurationSubsystem.h"
#include "UI/Configuration/MF_WidgetTypes.h"

#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
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
                    "Slot": {"Anchors": {"Min": {"X": 0.5, "Y": 0.5}, "Max": {"X": 0.5, "Y": 0.5}}, "Alignment": {"X": 0.5, "Y": 0.5}, "Size": {"X": 900, "Y": 650}},
                    "Children": [
                        {"Type": "TextBlock", "Name": "InputSettingsTitle", "BindingType": "Optional", "Text": "INPUT SETTINGS", "Font": {"Size": 28, "Typeface": "Bold"}, "Slot": {"HAlign": "Center", "Padding": {"Bottom": 10}}},
                        {
                            "Type": "HorizontalBox",
                            "Name": "ProfileRow",
                            "Slot": {"HAlign": "Center", "Padding": {"Bottom": 12}},
                            "Children": [
                                {"Type": "ComboBoxString", "Name": "ProfileSelector", "BindingType": "Optional", "Slot": {"HAlign": "Fill", "Padding": {"Right": 10}}},
                                {"Type": "Button", "Name": "ResetDefaultsButton", "BindingType": "Optional", "Children": [
                                    {"Type": "TextBlock", "Name": "ResetDefaultsLabel", "Text": "DEFAULT", "Justification": "Center", "Slot": {"HAlign": "Center", "VAlign": "Center"}}
                                ]}
                            ]
                        },
                        {"Type": "TextBlock", "Name": "EmptyStateText", "BindingType": "Optional", "Text": "No input bindings loaded.", "Justification": "Center", "Slot": {"HAlign": "Center", "Padding": {"Bottom": 12}}},
                        {
                            "Type": "ScrollBox",
                            "Name": "ActionListScroll",
                            "BindingType": "Required",
                            "Properties": {"ConsumeMouseWheel": "IfScrollingPossible", "AlwaysShowScrollbar": false},
                            "Slot": {"HAlign": "Fill", "VAlign": "Fill", "Fill": 1.0, "Padding": {"Bottom": 12}},
                            "Children": [
                                {"Type": "VerticalBox", "Name": "ActionListContentBox", "BindingType": "Optional", "Properties": {"SizeToContent": true, "Spacing": 6}}
                            ]
                        },
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
            {"Name": "InputSettingsTitle", "Type": "UTextBlock"},
            {"Name": "EmptyStateText", "Type": "UTextBlock"},
            {"Name": "ActionListContentBox", "Type": "UVerticalBox"},
            {"Name": "ProfileSelector", "Type": "UComboBoxString"},
            {"Name": "ResetDefaultsButton", "Type": "UButton"}
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

    if (ProfileSelector)
    {
        ProfileSelector->OnSelectionChanged.AddDynamic(this, &UMF_InputSettings::HandleProfileSelectionChanged);
    }

    if (ResetDefaultsButton)
    {
        ResetDefaultsButton->OnClicked.AddDynamic(this, &UMF_InputSettings::HandleResetDefaultsClicked);
    }

    PopulateProfileList();
    SyncProfileSelectorToPlayer();

    LoadProfileForEditing();
    RebuildRows();

    SetVisibility(ESlateVisibility::Collapsed);
}

void UMF_InputSettings::NativeDestruct()
{
    if (ProfileSelector)
    {
        ProfileSelector->OnSelectionChanged.RemoveDynamic(this, &UMF_InputSettings::HandleProfileSelectionChanged);
    }

    if (ResetDefaultsButton)
    {
        ResetDefaultsButton->OnClicked.RemoveDynamic(this, &UMF_InputSettings::HandleResetDefaultsClicked);
    }

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
    PopulateProfileList();
    SyncProfileSelectorToPlayer();
    LoadProfileForEditing();
    RebuildRows();
    SetKeyboardFocus();
}

void UMF_InputSettings::PopulateProfileList()
{
    if (!ProfileSelector)
    {
        return;
    }

    ProfileSelector->ClearOptions();

    UCPP_InputBindingManager *Manager = GetMEISManager();
    if (!Manager)
    {
        return;
    }

    TArray<FName> Templates;
    Manager->GetAvailableTemplates(Templates);

    Templates.Sort([](const FName &A, const FName &B)
                   { return A.LexicalLess(B); });

    // Prefer Default first if present.
    const FName DefaultName(TEXT("Default"));
    if (Templates.Contains(DefaultName))
    {
        ProfileSelector->AddOption(DefaultName.ToString());
        Templates.Remove(DefaultName);
    }

    for (const FName &TemplateName : Templates)
    {
        ProfileSelector->AddOption(TemplateName.ToString());
    }
}

void UMF_InputSettings::LoadPreset(FName PresetName)
{
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

    TargetTemplateName = PresetName;

    if (Manager->ApplyTemplateToPlayer(PC, PresetName))
    {
        Manager->ApplyPlayerProfileToEnhancedInput(PC);
    }

    LoadProfileForEditing();
    RebuildRows();
    SetKeyboardFocus();
}

void UMF_InputSettings::ResetToDefaults()
{
    LoadPreset(FName(TEXT("Default")));
}

void UMF_InputSettings::SyncProfileSelectorToPlayer()
{
    if (!ProfileSelector)
    {
        return;
    }

    UCPP_InputBindingManager *Manager = GetMEISManager();
    APlayerController *PC = GetOwningPlayer();
    if (!Manager || !PC)
    {
        return;
    }

    FName LoadedTemplate = Manager->GetPlayerLoadedTemplateName(PC);
    if (LoadedTemplate.IsNone())
    {
        LoadedTemplate = TargetTemplateName.IsNone() ? FName(TEXT("Default")) : TargetTemplateName;
    }

    const FString Option = LoadedTemplate.ToString();
    if (ProfileSelector->FindOptionIndex(Option) == INDEX_NONE)
    {
        ProfileSelector->AddOption(Option);
    }

    bSuppressProfileSelectionChanged = true;
    ProfileSelector->SetSelectedOption(Option);
    bSuppressProfileSelectionChanged = false;
}

void UMF_InputSettings::HandleProfileSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (bSuppressProfileSelectionChanged)
    {
        return;
    }

    const FString Trimmed = SelectedItem.TrimStartAndEnd();
    if (Trimmed.IsEmpty())
    {
        return;
    }

    LoadPreset(FName(*Trimmed));
}

void UMF_InputSettings::HandleResetDefaultsClicked()
{
    ResetToDefaults();
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
        UE_LOG(LogTemp, Warning, TEXT("UMF_InputSettings::LoadProfileForEditing - Missing %s%s"),
               Manager ? TEXT("") : TEXT("Manager "),
               PC ? TEXT("") : TEXT("OwningPlayer"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("UMF_InputSettings::LoadProfileForEditing - PC=%s Registered=%d"),
           *PC->GetName(), Manager->HasPlayerRegistered(PC) ? 1 : 0);

    if (!Manager->HasPlayerRegistered(PC))
    {
        UE_LOG(LogTemp, Log, TEXT("UMF_InputSettings::LoadProfileForEditing - Registering player %s"), *PC->GetName());
        Manager->RegisterPlayer(PC);
    }

    FS_InputProfile *Profile = Manager->GetProfileRefForPlayer(PC);
    if (!Profile)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMF_InputSettings::LoadProfileForEditing - No profile ref for %s"), *PC->GetName());
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("UMF_InputSettings::LoadProfileForEditing - Initial profile: Actions=%d Axes=%d"),
           Profile->ActionBindings.Num(), Profile->AxisBindings.Num());

    // If the player is registered but has no bindings yet, try to load the Default template.
    if (Profile->ActionBindings.Num() == 0 && Profile->AxisBindings.Num() == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("UMF_InputSettings::LoadProfileForEditing - Empty profile; attempting to load/apply Default template"));
        // Prefer the one-call helper on our MF controller (it can auto-create Default if missing).
        if (AMF_PlayerController *MFPC = Cast<AMF_PlayerController>(PC))
        {
            const bool bReady = MFPC->EnsureInputProfileReady(FName(TEXT("Default")), /*bCreateTemplateIfMissing*/ true, /*bApplyEvenIfNotEmpty*/ false);
            UE_LOG(LogTemp, Log, TEXT("UMF_InputSettings::LoadProfileForEditing - EnsureInputProfileReady returned %d"), bReady ? 1 : 0);
        }
        else
        {
            Manager->ApplyTemplateToPlayer(PC, FName(TEXT("Default")));
        }
        Profile = Manager->GetProfileRefForPlayer(PC);
        if (!Profile)
        {
            UE_LOG(LogTemp, Warning, TEXT("UMF_InputSettings::LoadProfileForEditing - Still no profile ref after applying Default"));
            return;
        }

        UE_LOG(LogTemp, Log, TEXT("UMF_InputSettings::LoadProfileForEditing - After Default: Actions=%d Axes=%d"),
               Profile->ActionBindings.Num(), Profile->AxisBindings.Num());
    }

    PendingProfile = *Profile;
    bHasPendingProfile = true;

    UE_LOG(LogTemp, Log, TEXT("UMF_InputSettings::LoadProfileForEditing - PendingProfile ready: Actions=%d Axes=%d"),
           PendingProfile.ActionBindings.Num(), PendingProfile.AxisBindings.Num());
}

void UMF_InputSettings::RebuildRows()
{
    if (!ActionListScroll)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMF_InputSettings::RebuildRows - ActionListScroll is null"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("UMF_InputSettings::RebuildRows - HasPending=%d Actions=%d Axes=%d"),
           bHasPendingProfile ? 1 : 0,
           PendingProfile.ActionBindings.Num(),
           PendingProfile.AxisBindings.Num());

    if (ActionListContentBox)
    {
        ActionListContentBox->ClearChildren();
    }
    else
    {
        ActionListScroll->ClearChildren();
    }

    if (EmptyStateText)
    {
        EmptyStateText->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (!bHasPendingProfile)
    {
        if (EmptyStateText)
        {
            EmptyStateText->SetText(FText::FromString(TEXT("Input profile not available yet.")));
            EmptyStateText->SetVisibility(ESlateVisibility::Visible);
        }
        return;
    }

    if (PendingProfile.ActionBindings.Num() == 0 && PendingProfile.AxisBindings.Num() == 0)
    {
        if (EmptyStateText)
        {
            EmptyStateText->SetText(FText::FromString(TEXT("No input bindings found. Ensure a profile/template is loaded (e.g. 'Default').")));
            EmptyStateText->SetVisibility(ESlateVisibility::Visible);
        }
        return;
    }

    int32 CreatedActionRows = 0;
    int32 CreatedAxisRows = 0;

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

        if (ActionListContentBox)
        {
            ActionListContentBox->AddChild(Row);
        }
        else
        {
            ActionListScroll->AddChild(Row);
        }

        ++CreatedActionRows;

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

        if (ActionListContentBox)
        {
            ActionListContentBox->AddChild(Row);
        }
        else
        {
            ActionListScroll->AddChild(Row);
        }

        ++CreatedAxisRows;

        if (RebindMode == ERebindMode::Axis && PendingIndex == AxisIndex)
        {
            Row->SetRebinding(true);
            PendingRow = Row;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("UMF_InputSettings::RebuildRows - Created rows: Actions=%d Axes=%d"), CreatedActionRows, CreatedAxisRows);
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
