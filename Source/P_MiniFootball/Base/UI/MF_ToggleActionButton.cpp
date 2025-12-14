/*
 * @Author: Punal Manalan
 * @Description: MF_ToggleActionButton - Generic hold/toggle button implementation
 * @Date: 14/12/2025
 */

#include "MF_ToggleActionButton.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Manager/CPP_InputBindingManager.h"
#include "InputBinding/FS_InputProfile.h"

FString UMF_ToggleActionButton::GetWidgetSpec()
{
    static FString Spec = R"JSON({
    "WidgetClass": "UMF_ToggleActionButton",
    "BlueprintName": "WBP_MF_ToggleActionButton",
    "ParentClass": "/Script/P_MiniFootball.MF_ToggleActionButton",
    "Category": "MF|UI|Controls",
    "Description": "Generic hold/toggle button bound to a P_MEIS action name",
    "Version": "1.0.0",

    "Hierarchy": {
        "Root": {
            "Type": "CanvasPanel",
            "Name": "RootCanvas",
            "Children": [
                {
                    "Type": "Button",
                    "Name": "ActionButton",
                    "BindingType": "Required",
                    "Slot": {
                        "Anchors": {"Min": {"X": 0.5, "Y": 0.5}, "Max": {"X": 0.5, "Y": 0.5}},
                        "Position": {"X": 0, "Y": 0},
                        "Size": {"X": 80, "Y": 80},
                        "Alignment": {"X": 0.5, "Y": 0.5}
                    },
                    "Children": [
                        {
                            "Type": "Overlay",
                            "Name": "ButtonContent",
                            "Children": [
                                {"Type": "Image", "Name": "ActionIcon", "BindingType": "Optional", "Slot": {"HAlign": "Center", "VAlign": "Center"}},
                                {"Type": "TextBlock", "Name": "ActionText", "BindingType": "Optional", "Text": "ACTION", "FontSize": 12, "Justification": "Center", "Slot": {"HAlign": "Center", "VAlign": "Bottom"}}
                            ]
                        }
                    ]
                }
            ]
        }
    },

    "Bindings": {
        "Required": [
            {"Name": "ActionButton", "Type": "UButton", "Purpose": "Main action button"}
        ],
        "Optional": [
            {"Name": "ActionIcon", "Type": "UImage", "Purpose": "Icon"},
            {"Name": "ActionText", "Type": "UTextBlock", "Purpose": "Label"}
        ]
    }
})JSON";
    return Spec;
}

void UMF_ToggleActionButton::NativeConstruct()
{
    Super::NativeConstruct();

    RefreshToggleModeFromProfile();
    RefreshActiveStateFromProfile();

    if (ActionButton)
    {
        ActionButton->OnPressed.AddDynamic(this, &UMF_ToggleActionButton::HandleButtonPressed);
        ActionButton->OnReleased.AddDynamic(this, &UMF_ToggleActionButton::HandleButtonReleased);
    }

    UpdateVisualState();
}

void UMF_ToggleActionButton::NativeDestruct()
{
    if (ActionButton)
    {
        ActionButton->OnPressed.RemoveDynamic(this, &UMF_ToggleActionButton::HandleButtonPressed);
        ActionButton->OnReleased.RemoveDynamic(this, &UMF_ToggleActionButton::HandleButtonReleased);
    }

    Super::NativeDestruct();
}

void UMF_ToggleActionButton::HandleButtonPressed()
{
    RefreshToggleModeFromProfile();

    if (ActionName.IsNone())
    {
        return;
    }

    if (bUseToggleMode)
    {
        SetActiveState(!bIsActive, true);
        return;
    }

    SetActiveState(true, true);
}

void UMF_ToggleActionButton::HandleButtonReleased()
{
    RefreshToggleModeFromProfile();

    if (ActionName.IsNone())
    {
        return;
    }

    if (bUseToggleMode)
    {
        return;
    }

    SetActiveState(false, true);
}

void UMF_ToggleActionButton::RefreshToggleModeFromProfile()
{
    bUseToggleMode = false;

    if (ActionName.IsNone())
    {
        return;
    }

    APlayerController *PC = GetOwningPlayer();
    if (!PC)
    {
        return;
    }

    UCPP_InputBindingManager *Manager = GEngine ? GEngine->GetEngineSubsystem<UCPP_InputBindingManager>() : nullptr;
    if (!Manager)
    {
        return;
    }

    FS_InputProfile *Profile = Manager->GetProfileRefForPlayer(PC);
    if (!Profile)
    {
        return;
    }

    bUseToggleMode = Profile->ToggleModeActions.Contains(ActionName);
}

void UMF_ToggleActionButton::RefreshActiveStateFromProfile()
{
    if (ActionName.IsNone())
    {
        return;
    }

    APlayerController *PC = GetOwningPlayer();
    if (!PC)
    {
        return;
    }

    UCPP_InputBindingManager *Manager = GEngine ? GEngine->GetEngineSubsystem<UCPP_InputBindingManager>() : nullptr;
    if (!Manager)
    {
        return;
    }

    FS_InputProfile *Profile = Manager->GetProfileRefForPlayer(PC);
    if (!Profile)
    {
        return;
    }

    const bool *bState = Profile->ToggleActionStates.Find(ActionName);
    if (bState)
    {
        bIsActive = *bState;
    }
}

void UMF_ToggleActionButton::SetActiveState(bool bNewActive, bool bBroadcast)
{
    if (bIsActive == bNewActive)
    {
        return;
    }

    bIsActive = bNewActive;

    // Persist to the owning player's profile.
    APlayerController *PC = GetOwningPlayer();
    if (PC)
    {
        UCPP_InputBindingManager *Manager = GEngine ? GEngine->GetEngineSubsystem<UCPP_InputBindingManager>() : nullptr;
        if (Manager)
        {
            FS_InputProfile *Profile = Manager->GetProfileRefForPlayer(PC);
            if (Profile && !ActionName.IsNone())
            {
                Profile->ToggleActionStates.Add(ActionName, bIsActive);
            }
        }
    }

    UpdateVisualState();

    if (bBroadcast)
    {
        OnStateChanged.Broadcast(bIsActive);
    }
}

void UMF_ToggleActionButton::UpdateVisualState()
{
    const FLinearColor Color = bIsActive ? ActiveColor : InactiveColor;

    if (ActionButton)
    {
        ActionButton->SetColorAndOpacity(Color);
    }

    if (ActionIcon)
    {
        ActionIcon->SetColorAndOpacity(Color);
    }

    if (ActionText)
    {
        ActionText->SetColorAndOpacity(Color);
    }
}
