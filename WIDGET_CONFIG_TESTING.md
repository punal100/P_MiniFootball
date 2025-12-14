# P_MiniFootball - Widget Configuration Testing Checklist

## Goal

Validate that widgets are resolved dynamically (no hard-coded blueprint paths in C++), and can be swapped via:

- Project Settings (`UMF_WidgetClassSettings`)
- JSON overrides (`UMF_WidgetConfigurationSubsystem` + `MF.WidgetConfig.Reload`)
- Runtime overrides (Blueprint/C++ registration)

## 1) Project Settings override

1. Open **Project Settings → Game → MF Widget Class Settings**.
2. Change one of these:
   - `MainMenuClass`
   - `MainHUDClass`
   - `PauseMenuClass`
3. Play the relevant level:
   - `L_MainMenu` should spawn `MainMenuClass`
   - Gameplay map should spawn HUD (if your controller/HUD BP uses the subsystem)

Expected: new widget class is used without code changes.

## 2) JSON override

1. Copy `Plugins/P_MiniFootball/Resources/WidgetConfig.template.json` to `Saved/WidgetConfig.json`.
2. Set `bAutoLoadJsonConfig=True` in `Config/DefaultGame.ini` (or toggle in Project Settings).
3. Change a value under `WidgetClasses` (e.g. `MainMenu`).
4. In PIE console, run: `MF.WidgetConfig.Reload`.

Expected: subsystem reloads JSON and uses the new classes for subsequent widget creation.

## 3) Blueprint string-key widgets

1. In Blueprint, get Engine Subsystem `MF_WidgetConfigurationSubsystem`.
2. Call `RegisterWidgetClassByKey("MyWidgetKey", SomeWidgetClass)`.
3. Resolve with `GetWidgetClassByKey("MyWidgetKey")` (or via `UMF_WidgetLoadingUtilities`).

Expected: custom widgets can be registered/created without changing the enum.

## 4) InputActionRow override (optional)

1. Create a BP subclass of `UMF_InputActionRow` (optional).
2. Set either:
   - `UMF_InputSettings.InputActionRowClassOverride` in your `WBP_MF_InputSettings`, or
   - `InputActionRowClass` in Project Settings / JSON.

Expected: `UMF_InputSettings` creates rows using the overridden class.
