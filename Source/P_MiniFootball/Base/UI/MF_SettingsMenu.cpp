/*
 * @Author: Punal Manalan
 * @Description: MF_SettingsMenu - Settings menu with Input configuration
 * @Date: 14/12/2025
 */

#include "MF_SettingsMenu.h"
#include "MF_Utilities.h"

#include "Core/MF_Types.h"

#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/Engine.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"

#include "Manager/CPP_InputBindingManager.h"

using namespace MF_Utilities;

FString UMF_SettingsMenu::GetWidgetSpec()
{
    static FString Spec = R"JSON({
        "WidgetClass": "UMF_SettingsMenu",
        "BlueprintName": "WBP_MF_SettingsMenu",
        "ParentClass": "/Script/P_MiniFootball.MF_SettingsMenu",
        "Category": "MF|UI|Menus",
        "Description": "Settings menu with Input section",
        "Version": "1.0.0",
        "DesignerPreview": {
            "SizeMode": "FillScreen",
            "ZoomLevel": 14,
            "ShowGrid": true
        },
        "Hierarchy": {
            "Root": {
                "Type": "CanvasPanel",
                "Name": "RootCanvas",
                "Children": [
                    {
                        "Type": "Overlay",
                        "Name": "BackgroundOverlay",
                        "Slot": {
                            "Anchors": {"Min": {"X": 0, "Y": 0}, "Max": {"X": 1, "Y": 1}},
                            "Offsets": {"Left": 0, "Top": 0, "Right": 0, "Bottom": 0}
                        }
                    },
                    {
                        "Type": "VerticalBox",
                        "Name": "SettingsContainer",
                        "Slot": {
                            "Anchors": {"Min": {"X": 0.5, "Y": 0.5}, "Max": {"X": 0.5, "Y": 0.5}},
                            "Alignment": {"X": 0.5, "Y": 0.5}
                        },
                        "Children": [
                            {
                                "Type": "TextBlock",
                                "Name": "SettingsTitle",
                                "Text": "SETTINGS",
                                "Font": {"Size": 32, "Typeface": "Bold"},
                                "Slot": {"HAlign": "Center", "Padding": {"Bottom": 30}}
                            },
                            {
                                "Type": "TextBlock",
                                "Name": "InputSectionTitle",
                                "Text": "INPUT",
                                "Font": {"Size": 20, "Typeface": "Bold"},
                                "Slot": {"HAlign": "Left", "Padding": {"Bottom": 15}}
                            },
                            {
                                "Type": "HorizontalBox",
                                "Name": "SprintModeRow",
                                "Slot": {"HAlign": "Left", "Padding": {"Bottom": 10}},
                                "Children": [
                                    {
                                        "Type": "TextBlock",
                                        "Name": "SprintModeLabel",
                                        "Text": "Sprint Mode:",
                                        "Font": {"Size": 14},
                                        "Slot": {"VAlign": "Center", "Padding": {"Right": 10}}
                                    },
                                    {
                                        "Type": "ComboBoxString",
                                        "Name": "SprintModeCombo",
                                        "BindingType": "Optional",
                                        "Slot": {"HAlign": "Fill"}
                                    }
                                ]
                            },
                            {
                                "Type": "VerticalBox",
                                "Name": "ActionBindingsList",
                                "BindingType": "Optional",
                                "Slot": {"HAlign": "Fill", "Padding": {"Top": 20, "Bottom": 20}}
                            },
                            {
                                "Type": "HorizontalBox",
                                "Name": "ButtonRow",
                                "Slot": {"HAlign": "Center", "Padding": {"Top": 20}},
                                "Children": [
                                    {
                                        "Type": "Button",
                                        "Name": "SaveButton",
                                        "BindingType": "Required",
                                        "Slot": {"HAlign": "Center", "Padding": {"Right": 10}},
                                        "Children": [
                                            {
                                                "Type": "TextBlock",
                                                "Name": "SaveLabel",
                                                "Text": "SAVE",
                                                "Slot": {"HAlign": "Center", "VAlign": "Center"}
                                            }
                                        ]
                                    },
                                    {
                                        "Type": "Button",
                                        "Name": "CancelButton",
                                        "BindingType": "Required",
                                        "Slot": {"HAlign": "Center"},
                                        "Children": [
                                            {
                                                "Type": "TextBlock",
                                                "Name": "CancelLabel",
                                                "Text": "CANCEL",
                                                "Slot": {"HAlign": "Center", "VAlign": "Center"}
                                            }
                                        ]
                                    }
                                ]
                            }
                        ]
                    }
                ]
            }
        },
        "Bindings": {
            "Required": [
                {"Name": "SaveButton", "Type": "UButton", "Purpose": "Save settings"},
                {"Name": "CancelButton", "Type": "UButton", "Purpose": "Cancel settings"}
            ],
            "Optional": [
                {"Name": "SprintModeCombo", "Type": "UComboBoxString", "Purpose": "Sprint mode selector"},
                {"Name": "ActionBindingsList", "Type": "UVerticalBox", "Purpose": "Action bindings container"}
            ]
        },
        "Delegates": [],
        "Dependencies": [],
        "Comments": {"Header": "MF Settings Menu - Player settings (input, sprint mode)"}
    })JSON";

    return Spec;
}

void UMF_SettingsMenu::NativeConstruct()
{
    Super::NativeConstruct();

    SetIsFocusable(true);

    if (SprintModeCombo)
    {
        SprintModeCombo->ClearOptions();
        SprintModeCombo->AddOption(TEXT("Hold"));
        SprintModeCombo->AddOption(TEXT("Toggle"));
        SprintModeCombo->OnSelectionChanged.AddDynamic(this, &UMF_SettingsMenu::HandleSprintModeChanged);
    }

    if (SaveButton)
    {
        SaveButton->OnClicked.AddDynamic(this, &UMF_SettingsMenu::HandleSaveSettings);
    }

    if (CancelButton)
    {
        CancelButton->OnClicked.AddDynamic(this, &UMF_SettingsMenu::HandleCancelSettings);
    }

    RefreshFromProfile();

    // Ensure we can capture keys for rebinding.
    SetKeyboardFocus();
}

void UMF_SettingsMenu::NativeDestruct()
{
    if (SprintModeCombo)
    {
        SprintModeCombo->OnSelectionChanged.RemoveDynamic(this, &UMF_SettingsMenu::HandleSprintModeChanged);
    }

    if (SaveButton)
    {
        SaveButton->OnClicked.RemoveDynamic(this, &UMF_SettingsMenu::HandleSaveSettings);
    }

    if (CancelButton)
    {
        CancelButton->OnClicked.RemoveDynamic(this, &UMF_SettingsMenu::HandleCancelSettings);
    }

    Super::NativeDestruct();
}

FReply UMF_SettingsMenu::NativeOnKeyDown(const FGeometry &InGeometry, const FKeyEvent &InKeyEvent)
{
    if (RebindMode == ERebindMode::None)
    {
        return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
    }

    const FKey PressedKey = InKeyEvent.GetKey();
    if (PressedKey == EKeys::Escape)
    {
        CancelRebind();
        PopulateActionList();
        SetKeyboardFocus();
        return FReply::Handled();
    }

    ApplyCapturedKeyToPendingProfile(InKeyEvent);
    PopulateActionList();
    SetKeyboardFocus();
    return FReply::Handled();
}

void UMF_SettingsMenu::HandleSprintModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    // Ignore programmatic changes
    if (SelectionType == ESelectInfo::Direct)
    {
        return;
    }

    bPendingSprintToggleMode = (SelectedItem == TEXT("Toggle"));

    if (bHasPendingProfile)
    {
        if (bPendingSprintToggleMode)
        {
            PendingProfile.ToggleModeActions.AddUnique(MF_InputActions::Sprint);
        }
        else
        {
            PendingProfile.ToggleModeActions.Remove(MF_InputActions::Sprint);
            PendingProfile.ToggleActionStates.Remove(MF_InputActions::Sprint);
        }
    }
}

void UMF_SettingsMenu::HandleSaveSettings()
{
    APlayerController *PC = GetOwningPlayer();
    if (!PC || !PC->IsLocalController())
    {
        HandleCancelSettings();
        return;
    }

    UCPP_InputBindingManager *Manager = GetMEISManager();
    if (!Manager)
    {
        HandleCancelSettings();
        return;
    }

    // Ensure player is registered so we have an editable profile.
    if (!Manager->HasPlayerRegistered(PC))
    {
        Manager->RegisterPlayer(PC);
    }

    FS_InputProfile *Profile = Manager->GetProfileRefForPlayer(PC);
    if (Profile)
    {
        CancelRebind();

        if (bHasPendingProfile)
        {
            Profile->ActionBindings = PendingProfile.ActionBindings;
            Profile->AxisBindings = PendingProfile.AxisBindings;
            Profile->ToggleModeActions = PendingProfile.ToggleModeActions;
            Profile->ToggleActionStates = PendingProfile.ToggleActionStates;
        }
        else
        {
            // Fallback: at least persist sprint toggle/hold.
            if (bPendingSprintToggleMode)
            {
                Profile->ToggleModeActions.AddUnique(MF_InputActions::Sprint);
            }
            else
            {
                Profile->ToggleModeActions.Remove(MF_InputActions::Sprint);
                Profile->ToggleActionStates.Remove(MF_InputActions::Sprint);
            }
        }

        // Persist to disk as a per-player template.
        Manager->SavePlayerProfileAsTemplate(PC, Profile->ProfileName);

        // Re-apply profile so gameplay sees updated preferences immediately.
        Manager->ApplyPlayerProfileToEnhancedInput(PC);
    }

    HandleCancelSettings();
}

void UMF_SettingsMenu::HandleCancelSettings()
{
    SetVisibility(ESlateVisibility::Collapsed);
    OnSettingsClosed.Broadcast();
}

void UMF_SettingsMenu::RefreshFromProfile()
{
    LoadProfileForEditing();
    RefreshSprintMode();
    PopulateActionList();
}

void UMF_SettingsMenu::RefreshSprintMode()
{
    if (!SprintModeCombo)
    {
        return;
    }

    bLoadedSprintToggleMode = false;
    bPendingSprintToggleMode = false;

    if (bHasPendingProfile)
    {
        bLoadedSprintToggleMode = PendingProfile.ToggleModeActions.Contains(MF_InputActions::Sprint);
        bPendingSprintToggleMode = bLoadedSprintToggleMode;
    }

    SprintModeCombo->SetSelectedOption(bLoadedSprintToggleMode ? TEXT("Toggle") : TEXT("Hold"));
}

void UMF_SettingsMenu::PopulateActionList()
{
    if (!ActionBindingsList)
    {
        return;
    }

    ActionBindingsList->ClearChildren();

    const FS_InputProfile *Profile = bHasPendingProfile ? &PendingProfile : nullptr;

    struct FActionRow
    {
        FText Label;
        FName Name;
        bool bIsAxis;
    };

    const TArray<FActionRow> Rows = {
        {FText::FromString(TEXT("Move")), MF_InputActions::Move, true},
        {FText::FromString(TEXT("Action")), MF_InputActions::Action, false},
        {FText::FromString(TEXT("Sprint")), MF_InputActions::Sprint, false},
        {FText::FromString(TEXT("Switch Player")), MF_InputActions::SwitchPlayer, false},
        {FText::FromString(TEXT("Pause")), MF_InputActions::Pause, false},
    };

    auto GetBindingsText = [this, Profile](const FActionRow &Row) -> FString
    {
        if (!Profile)
        {
            return TEXT("(not initialized)");
        }

        if (RebindMode != ERebindMode::None && PendingRebindName == Row.Name)
        {
            if (RebindMode == ERebindMode::ActionSingle)
            {
                return TEXT("Press a key... (Esc to cancel)");
            }

            if (RebindMode == ERebindMode::MoveWASD)
            {
                switch (PendingMoveStep)
                {
                case 0:
                    return TEXT("Press key for Up (W)... (Esc to cancel)");
                case 1:
                    return TEXT("Press key for Down (S)... (Esc to cancel)");
                case 2:
                    return TEXT("Press key for Left (A)... (Esc to cancel)");
                case 3:
                    return TEXT("Press key for Right (D)... (Esc to cancel)");
                default:
                    break;
                }
            }
        }

        if (Row.bIsAxis)
        {
            const FS_InputAxisBinding *Axis = Profile->AxisBindings.FindByPredicate([&Row](const FS_InputAxisBinding &B)
                                                                                    { return B.InputAxisName == Row.Name; });

            if (!Axis)
            {
                return TEXT("(unbound)");
            }

            TArray<FString> Parts;
            for (const FS_AxisKeyBinding &B : Axis->AxisBindings)
            {
                Parts.Add(FString::Printf(TEXT("%s(%.2f)"), *B.Key.ToString(), B.Scale));
            }

            return Parts.Num() > 0 ? JoinStrings(Parts, TEXT(", ")) : TEXT("(unbound)");
        }

        const FS_InputActionBinding *Action = Profile->ActionBindings.FindByPredicate([&Row](const FS_InputActionBinding &B)
                                                                                      { return B.InputActionName == Row.Name; });

        if (!Action)
        {
            return TEXT("(unbound)");
        }

        TArray<FString> Parts;
        for (const FS_KeyBinding &B : Action->KeyBindings)
        {
            Parts.Add(B.Key.ToString());
        }

        return Parts.Num() > 0 ? JoinStrings(Parts, TEXT(", ")) : TEXT("(unbound)");
    };

    for (const FActionRow &Row : Rows)
    {
        UHorizontalBox *HBox = NewObject<UHorizontalBox>(this);

        UTextBlock *Label = NewObject<UTextBlock>(HBox);
        Label->SetText(Row.Label);

        UTextBlock *Bindings = NewObject<UTextBlock>(HBox);
        Bindings->SetText(FText::FromString(GetBindingsText(Row)));

        UButton *RebindButton = NewObject<UButton>(HBox);
        UTextBlock *RebindLabel = NewObject<UTextBlock>(RebindButton);
        RebindLabel->SetText(FText::FromString(TEXT("Rebind")));
        RebindButton->AddChild(RebindLabel);

        if (Row.bIsAxis)
        {
            RebindButton->OnClicked.AddDynamic(this, &UMF_SettingsMenu::HandleRebindMoveClicked);
        }
        else if (Row.Name == MF_InputActions::Action)
        {
            RebindButton->OnClicked.AddDynamic(this, &UMF_SettingsMenu::HandleRebindActionClicked);
        }
        else if (Row.Name == MF_InputActions::Sprint)
        {
            RebindButton->OnClicked.AddDynamic(this, &UMF_SettingsMenu::HandleRebindSprintClicked);
        }
        else if (Row.Name == MF_InputActions::SwitchPlayer)
        {
            RebindButton->OnClicked.AddDynamic(this, &UMF_SettingsMenu::HandleRebindSwitchPlayerClicked);
        }
        else if (Row.Name == MF_InputActions::Pause)
        {
            RebindButton->OnClicked.AddDynamic(this, &UMF_SettingsMenu::HandleRebindPauseClicked);
        }

        HBox->AddChild(Label);
        HBox->AddChild(Bindings);
        HBox->AddChild(RebindButton);

        ActionBindingsList->AddChild(HBox);
    }
}

void UMF_SettingsMenu::LoadProfileForEditing()
{
    bHasPendingProfile = false;
    PendingProfile = FS_InputProfile{};

    APlayerController *PC = GetOwningPlayer();
    UCPP_InputBindingManager *Manager = GetMEISManager();
    if (!PC || !Manager)
    {
        CancelRebind();
        return;
    }

    if (!Manager->HasPlayerRegistered(PC))
    {
        Manager->RegisterPlayer(PC);
    }

    const FS_InputProfile *Profile = Manager->GetProfileRefForPlayer(PC);
    if (!Profile)
    {
        CancelRebind();
        return;
    }

    PendingProfile = *Profile;
    bHasPendingProfile = true;

    CancelRebind();
}

void UMF_SettingsMenu::HandleRebindMoveClicked()
{
    BeginRebindMove();
}

void UMF_SettingsMenu::HandleRebindActionClicked()
{
    BeginRebindAction(MF_InputActions::Action);
}

void UMF_SettingsMenu::HandleRebindSprintClicked()
{
    BeginRebindAction(MF_InputActions::Sprint);
}

void UMF_SettingsMenu::HandleRebindSwitchPlayerClicked()
{
    BeginRebindAction(MF_InputActions::SwitchPlayer);
}

void UMF_SettingsMenu::HandleRebindPauseClicked()
{
    BeginRebindAction(MF_InputActions::Pause);
}

void UMF_SettingsMenu::BeginRebindAction(const FName &ActionName)
{
    if (!bHasPendingProfile)
    {
        LoadProfileForEditing();
    }

    RebindMode = ERebindMode::ActionSingle;
    PendingRebindName = ActionName;
    PendingMoveStep = 0;

    PopulateActionList();
    SetKeyboardFocus();
}

void UMF_SettingsMenu::BeginRebindMove()
{
    if (!bHasPendingProfile)
    {
        LoadProfileForEditing();
    }

    RebindMode = ERebindMode::MoveWASD;
    PendingRebindName = MF_InputActions::Move;
    PendingMoveStep = 0;

    PopulateActionList();
    SetKeyboardFocus();
}

void UMF_SettingsMenu::CancelRebind()
{
    RebindMode = ERebindMode::None;
    PendingRebindName = NAME_None;
    PendingMoveStep = 0;
}

void UMF_SettingsMenu::ApplyCapturedKeyToPendingProfile(const FKeyEvent &InKeyEvent)
{
    if (!bHasPendingProfile)
    {
        CancelRebind();
        return;
    }

    const FKey PressedKey = InKeyEvent.GetKey();
    if (!PressedKey.IsValid())
    {
        return;
    }

    if (RebindMode == ERebindMode::ActionSingle)
    {
        FS_InputActionBinding *Action = PendingProfile.ActionBindings.FindByPredicate([this](const FS_InputActionBinding &B)
                                                                                      { return B.InputActionName == PendingRebindName; });

        if (!Action)
        {
            FS_InputActionBinding NewBinding;
            NewBinding.InputActionName = PendingRebindName;
            NewBinding.DisplayName = FText::FromName(PendingRebindName);
            PendingProfile.ActionBindings.Add(NewBinding);
            Action = &PendingProfile.ActionBindings.Last();
        }

        FS_KeyBinding KeyBinding;
        KeyBinding.Key = PressedKey;
        KeyBinding.Value = 1.0f;
        KeyBinding.bShift = InKeyEvent.IsShiftDown();
        KeyBinding.bCtrl = InKeyEvent.IsControlDown();
        KeyBinding.bAlt = InKeyEvent.IsAltDown();
        KeyBinding.bCmd = InKeyEvent.IsCommandDown();

        Action->KeyBindings.Reset();
        Action->KeyBindings.Add(KeyBinding);

        CancelRebind();
        return;
    }

    if (RebindMode == ERebindMode::MoveWASD)
    {
        FS_InputAxisBinding *Axis = PendingProfile.AxisBindings.FindByPredicate([](const FS_InputAxisBinding &B)
                                                                                { return B.InputAxisName == MF_InputActions::Move; });

        if (!Axis)
        {
            FS_InputAxisBinding NewBinding;
            NewBinding.InputAxisName = MF_InputActions::Move;
            NewBinding.DisplayName = FText::FromString(TEXT("Move"));
            NewBinding.ValueType = EInputActionValueType::Axis2D;
            PendingProfile.AxisBindings.Add(NewBinding);
            Axis = &PendingProfile.AxisBindings.Last();
        }

        Axis->ValueType = EInputActionValueType::Axis2D;
        Axis->AxisBindings.SetNum(4);

        FS_AxisKeyBinding &AxisKeyBinding = Axis->AxisBindings[PendingMoveStep];
        AxisKeyBinding.Key = PressedKey;

        switch (PendingMoveStep)
        {
        case 0: // Up
            AxisKeyBinding.Scale = 1.0f;
            AxisKeyBinding.bSwizzleYXZ = true;
            break;
        case 1: // Down
            AxisKeyBinding.Scale = -1.0f;
            AxisKeyBinding.bSwizzleYXZ = true;
            break;
        case 2: // Left
            AxisKeyBinding.Scale = -1.0f;
            AxisKeyBinding.bSwizzleYXZ = false;
            break;
        case 3: // Right
            AxisKeyBinding.Scale = 1.0f;
            AxisKeyBinding.bSwizzleYXZ = false;
            break;
        default:
            break;
        }

        PendingMoveStep++;
        if (PendingMoveStep >= 4)
        {
            CancelRebind();
        }

        return;
    }

    CancelRebind();
}
