#include "Input/MF_DefaultInputTemplates.h"

#include "Core/MF_Types.h"

#include "InputCoreTypes.h"

#include "InputBinding/FS_InputProfile.h"
#include "InputBinding/FS_InputAxisBinding.h"
#include "InputBinding/FS_InputActionBinding.h"

FS_InputProfile MF_DefaultInputTemplates::BuildDefaultInputTemplate(const FName TemplateName)
{
    FS_InputProfile Profile;
    Profile.ProfileName = TemplateName;
    Profile.ProfileDescription = FText::FromString(TEXT("MiniFootball default input bindings"));
    Profile.CreatedBy = FString(TEXT("P_MiniFootball"));
    Profile.bIsDefault = true;

    // Axis: Move (Axis2D) - WASD + Gamepad Left Stick
    {
        FS_InputAxisBinding MoveAxis;
        MoveAxis.InputAxisName = MF_InputActions::Move;
        MoveAxis.DisplayName = FText::FromString(TEXT("Move"));
        MoveAxis.Category = FName(TEXT("Movement"));
        MoveAxis.Description = FText::FromString(TEXT("Move the player"));
        MoveAxis.ValueType = EInputActionValueType::Axis2D;

        // Keyboard: W/S affect Y (swizzle X->Y), A/D affect X.
        {
            FS_AxisKeyBinding W;
            W.Key = EKeys::W;
            W.Scale = 1.0f;
            W.bSwizzleYXZ = true;
            MoveAxis.AxisBindings.Add(W);
        }
        {
            FS_AxisKeyBinding S;
            S.Key = EKeys::S;
            S.Scale = -1.0f;
            S.bSwizzleYXZ = true;
            MoveAxis.AxisBindings.Add(S);
        }
        {
            FS_AxisKeyBinding A;
            A.Key = EKeys::A;
            A.Scale = -1.0f;
            A.bSwizzleYXZ = false;
            MoveAxis.AxisBindings.Add(A);
        }
        {
            FS_AxisKeyBinding D;
            D.Key = EKeys::D;
            D.Scale = 1.0f;
            D.bSwizzleYXZ = false;
            MoveAxis.AxisBindings.Add(D);
        }

        // Gamepad: Left stick.
        {
            FS_AxisKeyBinding StickX;
            StickX.Key = EKeys::Gamepad_LeftX;
            StickX.Scale = 1.0f;
            StickX.bSwizzleYXZ = false;
            MoveAxis.AxisBindings.Add(StickX);
        }
        {
            FS_AxisKeyBinding StickY;
            StickY.Key = EKeys::Gamepad_LeftY;
            // Unreal gamepad Y is typically inverted (up = -1), so invert to match WASD (W = +Y).
            StickY.Scale = -1.0f;
            StickY.bSwizzleYXZ = true;
            MoveAxis.AxisBindings.Add(StickY);
        }

        Profile.AxisBindings.Add(MoveAxis);
    }

    auto AddAction = [&Profile](const FName ActionName, const TCHAR *DisplayName, const TCHAR *Category, const TCHAR *Description, const TArray<FKey> &Keys)
    {
        FS_InputActionBinding Action;
        Action.InputActionName = ActionName;
        Action.DisplayName = FText::FromString(DisplayName);
        Action.Category = FName(Category);
        Action.Description = FText::FromString(Description);

        for (const FKey &Key : Keys)
        {
            if (!Key.IsValid())
            {
                continue;
            }

            FS_KeyBinding KB;
            KB.Key = Key;
            Action.KeyBindings.Add(KB);
        }

        Profile.ActionBindings.Add(Action);
    };

    AddAction(MF_InputActions::Action,
              TEXT("Action"),
              TEXT("Gameplay"),
              TEXT("Primary gameplay action"),
              {EKeys::LeftMouseButton, EKeys::SpaceBar, EKeys::Gamepad_FaceButton_Bottom});

    AddAction(MF_InputActions::Sprint,
              TEXT("Sprint"),
              TEXT("Movement"),
              TEXT("Sprint while held"),
              {EKeys::LeftShift, EKeys::Gamepad_LeftShoulder});

    AddAction(MF_InputActions::SwitchPlayer,
              TEXT("Switch Player"),
              TEXT("Gameplay"),
              TEXT("Switch controlled player"),
              {EKeys::Q, EKeys::Gamepad_DPad_Left, EKeys::Gamepad_DPad_Right});

    AddAction(MF_InputActions::Pause,
              TEXT("Pause"),
              TEXT("UI"),
              TEXT("Pause menu"),
              {EKeys::Escape, EKeys::Gamepad_Special_Right});

    return Profile;
}
