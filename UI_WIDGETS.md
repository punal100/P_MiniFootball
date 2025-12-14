# P_MiniFootball - UI Widget System Documentation

Complete documentation for the C++ UI widget system including binding reference, visual design specifications, and a MWCS-driven Widget Blueprint creation/repair/validation workflow.

This project uses **P_MWCS** (Modular Widget Creation System) as the **spec-driven** and **deterministic** source of truth for generating/repairing Widget Blueprints from the JSON returned by each C++ widget’s `GetWidgetSpec()`.

---

## 🗺️ Where root widgets are spawned

This project keeps menu UI and gameplay UI separate by using different PlayerControllers.

- `L_MainMenu` (menu-only): use `BP_MF_MenuGameMode` + `BP_MF_MenuPlayerController`
  - The menu controller creates `WBP_MF_MainMenu` in `BeginPlay()` and switches to UI-only input.
- `L_MiniFootball` (gameplay): use `BP_MF_GameMode` + `BP_MF_PlayerController`
  - Gameplay controller is responsible for creating `WBP_MF_HUD` (Blueprint or C++ subclass).

## 📝 Complete Widget Binding Reference

Each widget has **REQUIRED** bindings (⚠️) that MUST exist and **OPTIONAL** bindings (✅) that can be omitted.

### WBP_MF_MatchInfo Bindings

| Widget Name      | Type        | Required | Notes             |
| ---------------- | ----------- | -------- | ----------------- |
| `TeamAScoreText` | `TextBlock` | ⚠️ YES   | Team A score      |
| `TeamBScoreText` | `TextBlock` | ⚠️ YES   | Team B score      |
| `MatchTimerText` | `TextBlock` | ⚠️ YES   | Time remaining    |
| `MatchPhaseText` | `TextBlock` | ✅ No    | "Kickoff", "Goal" |
| `TeamANameText`  | `TextBlock` | ✅ No    | "TEAM A"          |
| `TeamBNameText`  | `TextBlock` | ✅ No    | "TEAM B"          |

### WBP_MF_TeamIndicator Bindings

| Widget Name       | Type        | Required | Notes                  |
| ----------------- | ----------- | -------- | ---------------------- |
| `TeamText`        | `TextBlock` | ⚠️ YES   | "Team A" / "Spectator" |
| `TeamColorBorder` | `Border`    | ✅ No    | Background color       |
| `TeamIcon`        | `Image`     | ✅ No    | Team logo              |

### WBP_MF_TransitionOverlay Bindings

| Widget Name         | Type        | Required | Notes              |
| ------------------- | ----------- | -------- | ------------------ |
| `StatusText`        | `TextBlock` | ⚠️ YES   | "Loading..."       |
| `LoadingThrobber`   | `Throbber`  | ✅ No    | Spinning indicator |
| `BackgroundOverlay` | `Image`     | ✅ No    | Dark background    |

### WBP_MF_VirtualJoystick Bindings

| Widget Name     | Type    | Required | Notes             |
| --------------- | ------- | -------- | ----------------- |
| `JoystickBase`  | `Image` | ⚠️ YES   | Background circle |
| `JoystickThumb` | `Image` | ⚠️ YES   | Movable thumb     |

### WBP_MF_ActionButton Bindings

| Widget Name    | Type        | Required | Notes                |
| -------------- | ----------- | -------- | -------------------- |
| `ActionButton` | `Button`    | ⚠️ YES   | The clickable button |
| `ActionIcon`   | `Image`     | ✅ No    | Kick/Pass icon       |
| `ActionText`   | `TextBlock` | ✅ No    | "KICK" / "PASS"      |

### WBP_MF_ToggleActionButton Bindings

Generic hold/toggle button used for touch controls (e.g. Sprint).

| Widget Name    | Type        | Required | Notes                |
| -------------- | ----------- | -------- | -------------------- |
| `ActionButton` | `Button`    | ⚠️ YES   | The clickable button |
| `ActionIcon`   | `Image`     | ✅ No    | Optional icon        |
| `ActionText`   | `TextBlock` | ✅ No    | Optional label       |

### WBP_MF_MainMenu Bindings

| Widget Name      | Type        | Required | Notes                              |
| ---------------- | ----------- | -------- | ---------------------------------- |
| `NewGameButton`  | `Button`    | ⚠️ YES   | Open settings-first New Game flow  |
| `ContinueButton` | `Button`    | ⚠️ YES   | Continue with most recent template |
| `SettingsButton` | `Button`    | ⚠️ YES   | Open settings overlay              |
| `QuitButton`     | `Button`    | ⚠️ YES   | Quit game                          |
| `VersionText`    | `TextBlock` | ✅ No    | Optional version label             |

### WBP_MF_MainSettings Bindings

| Widget Name      | Type     | Required | Notes                        |
| ---------------- | -------- | -------- | ---------------------------- |
| `InputButton`    | `Button` | ⚠️ YES   | Open InputSettings           |
| `AudioButton`    | `Button` | ✅ No    | Open AudioSettings (stub)    |
| `GraphicsButton` | `Button` | ✅ No    | Open GraphicsSettings (stub) |
| `BackButton`     | `Button` | ⚠️ YES   | Close settings               |

### WBP_MF_InputSettings Bindings

| Widget Name          | Type        | Required | Notes                    |
| -------------------- | ----------- | -------- | ------------------------ |
| `InputSettingsTitle` | `TextBlock` | ✅ No    | Optional title text      |
| `ActionListScroll`   | `ScrollBox` | ⚠️ YES   | Dynamic action/axis rows |
| `SaveButton`         | `Button`    | ⚠️ YES   | Save + apply profile     |
| `CancelButton`       | `Button`    | ⚠️ YES   | Close without applying   |

### UMF_InputActionRow (Runtime-Created, Not MWCS)

`UMF_InputActionRow` is instantiated via `NewObject<UMF_InputActionRow>()` by `UMF_InputSettings` and builds its widget tree in C++.

- There is no MWCS-generated `WBP_MF_InputActionRow` asset.
- It does not have `GetWidgetSpec()` and should not be added to `SpecProviderClasses`.

Key API points:

- `SetActionBinding(...)` / `SetAxisBinding(...)` configure the row contents.
- `OnRebindRequested(bool bIsAxisBinding, FName BindingName)` is emitted when the user clicks Rebind.
- `SetRebinding(true/false)` toggles the visual rebinding state.

### WBP_MF_AudioSettings Bindings

| Widget Name  | Type     | Required | Notes         |
| ------------ | -------- | -------- | ------------- |
| `BackButton` | `Button` | ⚠️ YES   | Close overlay |

### WBP_MF_GraphicsSettings Bindings

| Widget Name  | Type     | Required | Notes         |
| ------------ | -------- | -------- | ------------- |
| `BackButton` | `Button` | ⚠️ YES   | Close overlay |

### WBP_MF_TeamPanel Bindings

| Widget Name       | Type          | Required | Notes             |
| ----------------- | ------------- | -------- | ----------------- |
| `TeamNameText`    | `TextBlock`   | ⚠️ YES   | "TEAM A"          |
| `PlayerCountText` | `TextBlock`   | ⚠️ YES   | "Players: 2/3"    |
| `PlayerListBox`   | `VerticalBox` | ⚠️ YES   | Player names list |
| `JoinButton`      | `Button`      | ⚠️ YES   | Join team button  |
| `PanelBorder`     | `Border`      | ✅ No    | Team color border |
| `JoinButtonText`  | `TextBlock`   | ✅ No    | "JOIN TEAM A"     |

### WBP_MF_QuickTeamPanel Bindings

| Widget Name        | Type          | Required | Notes               |
| ------------------ | ------------- | -------- | ------------------- |
| `TeamNameText`     | `TextBlock`   | ⚠️ YES   | "TEAM A"            |
| `PlayerCountText`  | `TextBlock`   | ⚠️ YES   | "(2)"               |
| `QuickJoinButton`  | `Button`      | ⚠️ YES   | Quick join button   |
| `PanelBorder`      | `Border`      | ✅ No    | Team color          |
| `PlayerListBox`    | `VerticalBox` | ✅ No    | Compact player list |
| `ShortcutHintText` | `TextBlock`   | ✅ No    | "(1)" shortcut      |

### WBP_MF_SpectatorControls Bindings

**ALL BINDINGS ARE OPTIONAL** - This widget is very flexible!

| Widget Name            | Type                    | Required | Notes               |
| ---------------------- | ----------------------- | -------- | ------------------- |
| `SpectatingLabel`      | `TextBlock`             | ✅ No    | "👁 SPECTATING"      |
| `CameraModeText`       | `TextBlock`             | ✅ No    | Camera mode display |
| `QuickTeamA`           | `WBP_MF_QuickTeamPanel` | ✅ No    | Team A preview      |
| `QuickTeamB`           | `WBP_MF_QuickTeamPanel` | ✅ No    | Team B preview      |
| `OpenTeamSelectButton` | `Button`                | ✅ No    | Open full selection |
| `ControlHintsText`     | `TextBlock`             | ✅ No    | "[F] Camera [TAB]"  |

### WBP_MF_GameplayControls Bindings

| Widget Name             | Type                        | Required | Notes                                   |
| ----------------------- | --------------------------- | -------- | --------------------------------------- |
| `MovementJoystick`      | `WBP_MF_VirtualJoystick`    | ⚠️ YES   | Touch movement                          |
| `ActionButton`          | `WBP_MF_ActionButton`       | ⚠️ YES   | Kick/Pass/Tackle                        |
| `SprintButton`          | `WBP_MF_ToggleActionButton` | ✅ No    | Toggle/hold action button (e.g. Sprint) |
| `LeftControlContainer`  | `Overlay`                   | ✅ No    | Left side wrapper                       |
| `RightControlContainer` | `Overlay`                   | ✅ No    | Right side wrapper                      |

### WBP_MF_TeamSelectionPopup Bindings

| Widget Name         | Type               | Required | Notes             |
| ------------------- | ------------------ | -------- | ----------------- |
| `TeamAPanel`        | `WBP_MF_TeamPanel` | ⚠️ YES   | Team A info       |
| `TeamBPanel`        | `WBP_MF_TeamPanel` | ⚠️ YES   | Team B info       |
| `CloseButton`       | `Button`           | ⚠️ YES   | Close/Cancel      |
| `TitleText`         | `TextBlock`        | ✅ No    | "SELECT TEAM"     |
| `AutoAssignButton`  | `Button`           | ✅ No    | Auto-balance join |
| `BackgroundOverlay` | `Overlay`          | ✅ No    | Modal background  |
| `StatusText`        | `TextBlock`        | ✅ No    | Error messages    |

### WBP_MF_PauseMenu Bindings

| Widget Name         | Type          | Required | Notes               |
| ------------------- | ------------- | -------- | ------------------- |
| `ResumeButton`      | `Button`      | ⚠️ YES   | Resume game         |
| `LeaveTeamButton`   | `Button`      | ⚠️ YES   | Return to spectator |
| `QuitButton`        | `Button`      | ⚠️ YES   | Quit to menu        |
| `TitleText`         | `TextBlock`   | ✅ No    | "⏸ PAUSED"          |
| `CurrentTeamText`   | `TextBlock`   | ✅ No    | "Team A"            |
| `ChangeTeamButton`  | `Button`      | ✅ No    | Open team select    |
| `SettingsButton`    | `Button`      | ✅ No    | Settings            |
| `MenuContainer`     | `VerticalBox` | ✅ No    | Menu items wrapper  |
| `BackgroundOverlay` | `Overlay`     | ✅ No    | Modal background    |

### WBP_MF_HUD Bindings (Main Container)

| Widget Name          | Type                        | Required | Notes              |
| -------------------- | --------------------------- | -------- | ------------------ |
| `MatchInfo`          | `WBP_MF_MatchInfo`          | ⚠️ YES   | Score/Time display |
| `TeamIndicator`      | `WBP_MF_TeamIndicator`      | ⚠️ YES   | Current team       |
| `ModeSwitcher`       | `WidgetSwitcher`            | ⚠️ YES   | Mode switching     |
| `SpectatorControls`  | `WBP_MF_SpectatorControls`  | ⚠️ YES   | Spectator UI       |
| `GameplayControls`   | `WBP_MF_GameplayControls`   | ⚠️ YES   | Player controls    |
| `TransitionOverlay`  | `WBP_MF_TransitionOverlay`  | ✅ No    | Loading screen     |
| `TeamSelectionPopup` | `WBP_MF_TeamSelectionPopup` | ✅ No    | Team selection     |
| `PauseMenu`          | `WBP_MF_PauseMenu`          | ✅ No    | Pause menu         |
| `RootCanvas`         | `CanvasPanel`               | ✅ No    | Root container     |

---

## 🚀 MWCS Workflow (Recommended)

Use this workflow to **Create Missing**, **Repair**, and **Validate** UI Widget Blueprints from C++ specs (no Python).

### 1) Enable + Configure MWCS

1. Enable the plugin: **P_MWCS**.
2. Open **Project Settings → MWCS**.
3. Add the C++ widget classes (or other configured spec providers) that expose `static FString GetWidgetSpec()` to the allowlist.

Recommended baseline allowlist for the MiniFootball UI set:

In this repo, the canonical list (and ordering) lives in `Config/DefaultEditor.ini`.

Recommended allowlist order (leaf → containers):

- `/Script/P_MiniFootball.MF_TransitionOverlay`
- `/Script/P_MiniFootball.MF_MatchInfo`
- `/Script/P_MiniFootball.MF_TeamIndicator`
- `/Script/P_MiniFootball.MF_ScorePopup`
- `/Script/P_MiniFootball.MF_ActionButton`
- `/Script/P_MiniFootball.MF_ToggleActionButton`
- `/Script/P_MiniFootball.MF_SprintButton` (legacy/compat)
- `/Script/P_MiniFootball.MF_VirtualJoystick`
- `/Script/P_MiniFootball.MF_TeamPanel`
- `/Script/P_MiniFootball.MF_QuickTeamPanel`
- `/Script/P_MiniFootball.MF_SpectatorControls`
- `/Script/P_MiniFootball.MF_TeamSelectionPopup`
- `/Script/P_MiniFootball.MF_InputSettings`
- `/Script/P_MiniFootball.MF_AudioSettings`
- `/Script/P_MiniFootball.MF_GraphicsSettings`
- `/Script/P_MiniFootball.MF_MainSettings`
- `/Script/P_MiniFootball.MF_PauseMenu`
- `/Script/P_MiniFootball.MF_MainMenu`
- `/Script/P_MiniFootball.MF_SettingsMenu`
- `/Script/P_MiniFootball.MF_GameplayControls`
- `/Script/P_MiniFootball.MF_HUD`

4. Set **OutputRootPath** to where you want `WBP_MF_*` assets created.

### 2) Generate / Repair / Validate

1. Open: **Tools → MWCS → Open Widget Creation System**.
2. Run **Create Missing** (first time) or **Repair** (for existing assets).
3. Run **Validate** and review the report/log output.

Notes:

- MWCS focuses on deterministic **structure + binding variables**. Layout/styling fields in specs may be ignored depending on widget type support; use the Designer to finalize visuals as needed.
- Some widgets include nested widgets ("UserWidget") in their hierarchy. MWCS will best-effort resolve them via binding type metadata.

---

## 🧰 Troubleshooting (Bindings)

The manual, step-by-step creation process is deprecated. Generate/repair these widgets via MWCS so the names/types and `Is Variable` flags match what the C++ class expects.

If `MWCS_ValidateWidgets` reports a binding issue, it almost always means the blueprint was edited manually after generation, or an older widget tree/member variable is still present.

### Common binding errors & fixes

| Error               | Cause                   | Fix                                               |
| ------------------- | ----------------------- | ------------------------------------------------- |
| "Widget not found"  | Name doesn't match      | Check spelling exactly matches C++ property name  |
| "Type mismatch"     | Wrong widget type       | Use correct type (Button, TextBlock, Image, etc.) |
| "Not a variable"    | "Is Variable" unchecked | Select widget → Details → Check "Is Variable"     |
| "Multiple bindings" | Duplicate names         | Each name must be unique in the widget            |

---

## 🎨 Visual Design Specifications

This section provides exact design specs for each widget element.

### Color Palette

```
Primary Colors:
├── Team A Red:      #CC3333 (RGB: 204, 51, 51)
├── Team B Blue:     #3333CC (RGB: 51, 51, 204)
├── Spectator Gray:  #4D4D4D (RGB: 77, 77, 77)
└── Neutral Dark:    #1A1A1A (RGB: 26, 26, 26)

UI Colors:
├── Background:      #000000 @ 70% opacity
├── Panel BG:        #2A2A2A (RGB: 42, 42, 42)
├── Button Normal:   #3D3D3D (RGB: 61, 61, 61)
├── Button Hover:    #4D4D4D (RGB: 77, 77, 77)
├── Button Pressed:  #2A2A2A (RGB: 42, 42, 42)
├── Button Disabled: #1A1A1A @ 50% opacity
└── Accent Green:    #33CC33 (RGB: 51, 204, 51)

Text Colors:
├── Primary White:   #FFFFFF (RGB: 255, 255, 255)
├── Secondary Gray:  #B3B3B3 (RGB: 179, 179, 179)
├── Hint Gray:       #666666 (RGB: 102, 102, 102)
└── Warning Yellow:  #FFCC00 (RGB: 255, 204, 0)
```

### Font Specifications

```
Roboto Font Family (UE5 Default):
├── Score Display:    Roboto Bold, 48-72px
├── Timer Display:    Roboto Bold, 36-48px
├── Headers/Titles:   Roboto Bold, 24-32px
├── Button Text:      Roboto Medium, 18-24px
├── Body Text:        Roboto Regular, 16-18px
├── Hints/Labels:     Roboto Regular, 12-14px
└── Player Names:     Roboto Regular, 14-16px
```

### Widget Sizing Reference

#### ⚠️ IMPORTANT: Two different size concepts in UE5

There are **TWO SEPARATE** places where size is configured:

| Concept                  | Where to Set                                         | What It Controls                                  |
| ------------------------ | ---------------------------------------------------- | ------------------------------------------------- |
| **1. WBP preview size**  | Designer **preview controls** (screen-size dropdown) | The design-time size of this WBP's canvas         |
| **2. Child Widget Slot** | Details Panel → **Slot (Canvas Panel Slot)**         | Position/anchors of widgets INSIDE a Canvas Panel |

---

#### 1️⃣ WBP preview size (Designer preview controls)

When you open a Widget Blueprint, the preview controls at the top of the Designer show:

```
[ Screen Size ▼ ] [ Custom ▼ ] [ Width: 800 ] [ Height: 80 ]
```

| Setting         | Description                                        |
| --------------- | -------------------------------------------------- |
| **Custom**      | You set a fixed Width × Height (e.g., 800 × 80)    |
| **Desired**     | WBP auto-sizes based on content (rare for roots)   |
| **Fill Screen** | WBP matches the selected Screen Size preset        |
| **Screen Size** | Dropdown with common resolutions (1920×1080, etc.) |

This sets the design canvas size for your WBP. It does NOT have anchors because it's the root widget.

---

#### 2️⃣ Child Widget Slot (Canvas Panel Slot)

When you place a widget **INSIDE a Canvas Panel**, that widget gets a **Slot** with these properties:

| Slot Property       | Description                                          |
| ------------------- | ---------------------------------------------------- |
| **Anchors**         | Min/Max points (0-1) defining anchor on parent       |
| **Position**        | Offset in pixels from anchor point                   |
| **Size**            | Width/Height when NOT using Size To Content          |
| **Size To Content** | ✅ ON = auto-size to children, ❌ OFF = use Size X/Y |
| **Alignment**       | Pivot point (0-1) for positioning                    |
| **Auto Size**       | Additional auto-sizing options                       |

---

#### Common Anchor Presets (for Child Widgets)

| Anchor Name            | Min (X, Y) | Max (X, Y) | Description               |
| ---------------------- | ---------- | ---------- | ------------------------- |
| **Top-Left**           | (0.0, 0.0) | (0.0, 0.0) | Fixed to top-left corner  |
| **Top-Center**         | (0.5, 0.0) | (0.5, 0.0) | Centered at top edge      |
| **Top-Right**          | (1.0, 0.0) | (1.0, 0.0) | Fixed to top-right corner |
| **Center-Left**        | (0.0, 0.5) | (0.0, 0.5) | Centered on left edge     |
| **Center**             | (0.5, 0.5) | (0.5, 0.5) | Dead center of parent     |
| **Center-Right**       | (1.0, 0.5) | (1.0, 0.5) | Centered on right edge    |
| **Bottom-Left**        | (0.0, 1.0) | (0.0, 1.0) | Fixed to bottom-left      |
| **Bottom-Center**      | (0.5, 1.0) | (0.5, 1.0) | Centered at bottom edge   |
| **Bottom-Right**       | (1.0, 1.0) | (1.0, 1.0) | Fixed to bottom-right     |
| **Stretch**            | (0.0, 0.0) | (1.0, 1.0) | Fill entire parent        |
| **Stretch Horizontal** | (0.0, 0.0) | (1.0, 0.0) | Fill width, fixed Y       |
| **Stretch Vertical**   | (0.0, 0.0) | (0.0, 1.0) | Fill height, fixed X      |

---

### 📊 Widget Configuration Summary

#### WBP preview size (set in the Designer preview controls)

| Widget Blueprint            | Preview setting | Width  | Height | Notes                     |
| --------------------------- | --------------- | ------ | ------ | ------------------------- |
| `WBP_MF_HUD`                | **Fill Screen** | (auto) | (auto) | Main HUD, fills viewport  |
| `WBP_MF_MatchInfo`          | **Custom**      | 800 px | 80 px  | Fixed-size info bar       |
| `WBP_MF_TeamIndicator`      | **Custom**      | 180 px | 40 px  | Small team badge          |
| `WBP_MF_TransitionOverlay`  | **Fill Screen** | (auto) | (auto) | Full-screen overlay       |
| `WBP_MF_VirtualJoystick`    | **Custom**      | 200 px | 200 px | Square touch area         |
| `WBP_MF_ActionButton`       | **Custom**      | 120 px | 120 px | Large round button        |
| `WBP_MF_ToggleActionButton` | **Custom**      | 80 px  | 80 px  | Medium round button       |
| `WBP_MF_TeamPanel`          | **Custom**      | 280 px | 350 px | Full team roster          |
| `WBP_MF_QuickTeamPanel`     | **Custom**      | 220 px | 50 px  | Compact team bar          |
| `WBP_MF_SpectatorControls`  | **Fill Screen** | (auto) | (auto) | Full-screen control layer |
| `WBP_MF_GameplayControls`   | **Fill Screen** | (auto) | (auto) | Full-screen control layer |
| `WBP_MF_TeamSelectionPopup` | **Fill Screen** | (auto) | (auto) | Full overlay with dialog  |
| `WBP_MF_PauseMenu`          | **Fill Screen** | (auto) | (auto) | Full overlay with menu    |
| `WBP_MF_MainMenu`           | **Fill Screen** | (auto) | (auto) | Main menu                 |
| `WBP_MF_MainSettings`       | **Fill Screen** | (auto) | (auto) | Settings hub overlay      |
| `WBP_MF_InputSettings`      | **Fill Screen** | (auto) | (auto) | Input rebinding overlay   |
| `WBP_MF_AudioSettings`      | **Fill Screen** | (auto) | (auto) | Audio overlay (stub)      |
| `WBP_MF_GraphicsSettings`   | **Fill Screen** | (auto) | (auto) | Graphics overlay (stub)   |

#### Child Placement in HUD (Canvas Panel Slot)

When these WBPs are added as children to `WBP_MF_HUD`'s Canvas Panel:

| Child Widget                | Anchor (in HUD)      | Position Offset  | Alignment |
| --------------------------- | -------------------- | ---------------- | --------- |
| `WBP_MF_MatchInfo`          | Top-Center (0.5, 0)  | X: 0, Y: 20      | (0.5, 0)  |
| `WBP_MF_TeamIndicator`      | Top-Left (0, 0)      | X: 20, Y: 110    | (0, 0)    |
| `WBP_MF_TransitionOverlay`  | Stretch (0,0)→(1,1)  | All: 0           | (0, 0)    |
| `WBP_MF_VirtualJoystick`    | Bottom-Left (0, 1)   | X: 40, Y: -240   | (0, 1)    |
| `WBP_MF_ActionButton`       | Bottom-Right (1, 1)  | X: -160, Y: -160 | (1, 1)    |
| `WBP_MF_ToggleActionButton` | Bottom-Right (1, 1)  | X: -140, Y: -280 | (1, 1)    |
| `WBP_MF_QuickTeamPanel`     | Center-Left (0, 0.5) | X: 10, Y: 0      | (0, 0.5)  |
| `WBP_MF_SpectatorControls`  | Stretch (0,0)→(1,1)  | All: 0           | (0, 0)    |
| `WBP_MF_GameplayControls`   | Stretch (0,0)→(1,1)  | All: 0           | (0, 0)    |

---

## 🔧 Integrating the HUD

### Option 1: From PlayerController (Recommended)

```cpp
// In your PlayerController's BeginPlay or SetupUI
void AMF_PlayerController::CreateHUD()
{
    if (HUDWidgetClass)
    {
        HUDWidget = CreateWidget<UMF_HUD>(this, HUDWidgetClass);
        if (HUDWidget)
        {
            HUDWidget->AddToViewport();
        }
    }
}
```

### Option 2: From HUD Class

```cpp
// In your custom AHUD class
void AMF_HUDClass::BeginPlay()
{
    Super::BeginPlay();

    if (HUDWidgetClass)
    {
        MainWidget = CreateWidget<UMF_HUD>(GetOwningPlayerController(), HUDWidgetClass);
        MainWidget->AddToViewport();
    }
}
```

### Option 3: From Blueprint

1. Open your PlayerController Blueprint
2. In BeginPlay, use **Create Widget** node
3. Select `WBP_MF_HUD` as the widget class
4. Call **Add to Viewport**

---

## 🎭 HUD Mode Switching

The HUD automatically switches between modes based on player state:

```cpp
// HUD modes (EMF_HUDMode)
Spectator,   // Spectating the match
Gameplay,    // Active player on a team
Menu,        // Modal popup open (Team Selection, Pause)
Transition   // Loading/transitioning
```

**Automatic Switching:**

- When player joins a team → Switches to `Gameplay` mode
- When player leaves team → Switches to `Spectator` mode
- When popup opens → Switches to `Menu` mode

**Manual Control:**

```cpp
// From C++
HUDWidget->SetHUDMode(EMF_HUDMode::Spectator);
HUDWidget->ShowTeamSelectionPopup();
HUDWidget->TogglePauseMenu();

// From Blueprint
Call "Set HUD Mode" on the HUD widget reference
```
