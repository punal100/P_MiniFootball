# P_MiniFootball - UI Widget System Documentation

Complete documentation for the C++ UI widget system including binding reference, visual design specifications, and step-by-step Widget Blueprint creation guide.

---

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

### WBP_MF_SprintButton Bindings

| Widget Name    | Type        | Required | Notes                |
| -------------- | ----------- | -------- | -------------------- |
| `SprintButton` | `Button`    | ⚠️ YES   | The clickable button |
| `SprintIcon`   | `Image`     | ✅ No    | Sprint icon          |
| `SprintText`   | `TextBlock` | ✅ No    | "SPRINT"             |

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

| Widget Name             | Type                     | Required | Notes              |
| ----------------------- | ------------------------ | -------- | ------------------ |
| `MovementJoystick`      | `WBP_MF_VirtualJoystick` | ⚠️ YES   | Touch movement     |
| `ActionButton`          | `WBP_MF_ActionButton`    | ⚠️ YES   | Kick/Pass/Tackle   |
| `SprintButton`          | `WBP_MF_SprintButton`    | ✅ No    | Sprint toggle      |
| `LeftControlContainer`  | `Overlay`                | ✅ No    | Left side wrapper  |
| `RightControlContainer` | `Overlay`                | ✅ No    | Right side wrapper |

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

## 🚀 Step-by-Step: Creating WBP_MF_HUD

Follow these steps in order to avoid binding errors:

### Phase 1: Create Base Widgets (No Dependencies)

#### 1.1 Create WBP_MF_MatchInfo

**Designer Toolbar Settings:**
| Setting | Value |
| ------------- | ----------- |
| **Size Mode** | Custom |
| **Width** | 800 px |
| **Height** | 80 px |

1. Content Browser → Right-click → User Interface → Widget Blueprint
2. Parent Class: Search for `MF_MatchInfo` (or `UMF_MatchInfo`)
3. Name: `WBP_MF_MatchInfo`
4. **Set Toolbar:** Custom, 800 x 80
5. Open Designer, add these widgets:

```
[Canvas Panel] ← Root (uses WBP size from toolbar)
├── [TextBlock] Name: "TeamAScoreText"    ← REQUIRED
├── [TextBlock] Name: "TeamBScoreText"    ← REQUIRED
└── [TextBlock] Name: "MatchTimerText"    ← REQUIRED
```

5. **IMPORTANT**: For each TextBlock, check "Is Variable" in Details panel
6. Compile & Save

#### 1.2 Create WBP_MF_TeamIndicator

**Designer Toolbar Settings:**
| Setting | Value |
| ------------- | ----------- |
| **Size Mode** | Custom |
| **Width** | 180 px |
| **Height** | 40 px |

1. Create Widget Blueprint with parent `MF_TeamIndicator`
2. Name: `WBP_MF_TeamIndicator`
3. **Set Toolbar:** Custom, 180 x 40
4. Add:

```
[Canvas Panel] ← Root (uses WBP size from toolbar)
└── [TextBlock] Name: "TeamText"          ← REQUIRED
```

#### 1.3 Create WBP_MF_TransitionOverlay

**Designer Toolbar Settings:**
| Setting | Value |
| ------------- | ----------- |
| **Size Mode** | Fill Screen |

1. Create Widget Blueprint with parent `MF_TransitionOverlay`
2. Name: `WBP_MF_TransitionOverlay`
3. **Set Toolbar:** Fill Screen (or Custom with large size like 1920x1080)
4. Add:

```
[Canvas Panel] ← Root (fills screen)
└── [TextBlock] Name: "StatusText"        ← REQUIRED (Centered)
```

#### 1.4 Create WBP_MF_VirtualJoystick

**Designer Toolbar Settings:**
| Setting | Value |
| ------------- | ----------- |
| **Size Mode** | Custom |
| **Width** | 200 px |
| **Height** | 200 px |

1. Create Widget Blueprint with parent `MF_VirtualJoystick`
2. Name: `WBP_MF_VirtualJoystick`
3. **Set Toolbar:** Custom, 200 x 200
4. Add:

```
[Canvas Panel] ← Root (uses WBP size from toolbar)
├── [Image] Name: "JoystickBase"          ← REQUIRED (fill parent or 200x200)
└── [Image] Name: "JoystickThumb"         ← REQUIRED (80x80, Centered)
```

#### 1.5 Create WBP_MF_ActionButton

**Designer Toolbar Settings:**
| Setting | Value |
| ------------- | ----------- |
| **Size Mode** | Custom |
| **Width** | 120 px |
| **Height** | 120 px |

1. Create Widget Blueprint with parent `MF_ActionButton`
2. Name: `WBP_MF_ActionButton`
3. **Set Toolbar:** Custom, 120 x 120
4. Add:

```
[Canvas Panel] ← Root (uses WBP size from toolbar)
└── [Button] Name: "ActionButton"         ← REQUIRED (fill parent or 120x120)
```

#### 1.6 Create WBP_MF_SprintButton

**Designer Toolbar Settings:**
| Setting | Value |
| ------------- | ----------- |
| **Size Mode** | Custom |
| **Width** | 80 px |
| **Height** | 80 px |

1. Create Widget Blueprint with parent `MF_SprintButton`
2. Name: `WBP_MF_SprintButton`
3. **Set Toolbar:** Custom, 80 x 80
4. Add:

```
[Canvas Panel] ← Root (uses WBP size from toolbar)
└── [Button] Name: "SprintButton"         ← REQUIRED (fill parent or 80x80)
```

#### 1.7 Create WBP_MF_TeamPanel

**Designer Toolbar Settings:**
| Setting | Value |
| ------------- | ----------- |
| **Size Mode** | Custom |
| **Width** | 280 px |
| **Height** | 350 px |

1. Create Widget Blueprint with parent `MF_TeamPanel`
2. Name: `WBP_MF_TeamPanel`
3. **Set Toolbar:** Custom, 280 x 350
4. Add:

```
[Canvas Panel] ← Root (uses WBP size from toolbar)
└── [Vertical Box]
    ├── [TextBlock] Name: "TeamNameText"      ← REQUIRED
    ├── [TextBlock] Name: "PlayerCountText"   ← REQUIRED
    ├── [Vertical Box] Name: "PlayerListBox"  ← REQUIRED
    └── [Button] Name: "JoinButton"           ← REQUIRED (Fill x 50)
```

#### 1.8 Create WBP_MF_QuickTeamPanel

**Designer Toolbar Settings:**
| Setting | Value |
| ------------- | ----------- |
| **Size Mode** | Custom |
| **Width** | 220 px |
| **Height** | 50 px |

1. Create Widget Blueprint with parent `MF_QuickTeamPanel`
2. Name: `WBP_MF_QuickTeamPanel`
3. **Set Toolbar:** Custom, 220 x 50
4. Add:

```
[Canvas Panel] ← Root (uses WBP size from toolbar)
└── [Horizontal Box]
    ├── [TextBlock] Name: "TeamNameText"      ← REQUIRED
    ├── [TextBlock] Name: "PlayerCountText"   ← REQUIRED
    └── [Button] Name: "QuickJoinButton"      ← REQUIRED (60x30)
```

### Phase 2: Create Container Widgets

#### 2.1 Create WBP_MF_SpectatorControls

**Designer Toolbar Settings:**
| Setting | Value |
| ------------- | ----------- |
| **Size Mode** | Fill Screen |

1. Create Widget Blueprint with parent `MF_SpectatorControls`
2. Name: `WBP_MF_SpectatorControls`
3. **Set Toolbar:** Fill Screen
4. **All bindings are optional!** Just add a root Canvas Panel:

```
[Canvas Panel] ← Root (fills screen)
└── (Your layout - optional widgets)
```

#### 2.2 Create WBP_MF_GameplayControls

**Designer Toolbar Settings:**
| Setting | Value |
| ------------- | ----------- |
| **Size Mode** | Fill Screen |

1. Create Widget Blueprint with parent `MF_GameplayControls`
2. Name: `WBP_MF_GameplayControls`
3. **Set Toolbar:** Fill Screen
4. Add:

```
[Canvas Panel] ← Root (fills screen)
├── [WBP_MF_VirtualJoystick] Name: "MovementJoystick"  ← REQUIRED
│   └── Slot: Anchors Bottom-Left (0,1), Position X:40 Y:-240
└── [WBP_MF_ActionButton] Name: "ActionButton"        ← REQUIRED
    └── Slot: Anchors Bottom-Right (1,1), Position X:-160 Y:-160
```

**How to add custom widget:**

- In Palette, search for `WBP_MF_VirtualJoystick`
- Drag it into the hierarchy
- Rename it to exactly `MovementJoystick`
- Check "Is Variable"

#### 2.3 Create WBP_MF_TeamSelectionPopup

**Designer Toolbar Settings:**
| Setting | Value |
| ------------- | ----------- |
| **Size Mode** | Fill Screen |

1. Create Widget Blueprint with parent `MF_TeamSelectionPopup`
2. Name: `WBP_MF_TeamSelectionPopup`
3. **Set Toolbar:** Fill Screen
4. Add:

```
[Canvas Panel] ← Root (fills screen, acts as darkened background)
└── [Border] ← Dialog box (650x500)
    └── Slot: Anchors Center (0.5, 0.5), Alignment (0.5, 0.5)
    └── Children:
        ├── [WBP_MF_TeamPanel] Name: "TeamAPanel"  ← REQUIRED
        ├── [WBP_MF_TeamPanel] Name: "TeamBPanel"  ← REQUIRED
        └── [Button] Name: "CloseButton"           ← REQUIRED (40x40)
```

#### 2.4 Create WBP_MF_PauseMenu

**Designer Toolbar Settings:**
| Setting | Value |
| ------------- | ----------- |
| **Size Mode** | Fill Screen |

1. Create Widget Blueprint with parent `MF_PauseMenu`
2. Name: `WBP_MF_PauseMenu`
3. **Set Toolbar:** Fill Screen
4. Add:

```
[Canvas Panel] ← Root (fills screen, acts as darkened background)
└── [Border] ← Menu box (300x400)
    └── Slot: Anchors Center (0.5, 0.5), Alignment (0.5, 0.5)
    └── [Vertical Box]
        ├── [Button] Name: "ResumeButton"          ← REQUIRED (Fill x 50)
        ├── [Button] Name: "LeaveTeamButton"       ← REQUIRED (Fill x 50)
        └── [Button] Name: "QuitButton"            ← REQUIRED (Fill x 50)
```

### Phase 3: Create Main HUD

#### 3.1 Create WBP_MF_HUD

**Designer Toolbar Settings:**
| Setting | Value |
| ------------- | ----------- |
| **Size Mode** | Fill Screen |

1. Create Widget Blueprint with parent `MF_HUD`
2. Name: `WBP_MF_HUD`
3. **Set Toolbar:** Fill Screen
4. Add all REQUIRED widgets to the Canvas Panel:

```
[Canvas Panel] ← Root (fills screen)
│
├── [WBP_MF_MatchInfo] Name: "MatchInfo"              ← REQUIRED
│   └── Slot: Anchors Top-Center (0.5, 0), Position Y:20, Alignment (0.5, 0)
│
├── [WBP_MF_TeamIndicator] Name: "TeamIndicator"      ← REQUIRED
│   └── Slot: Anchors Top-Left (0, 0), Position X:20 Y:110
│
├── [Widget Switcher] Name: "ModeSwitcher"            ← REQUIRED
│   └── Slot: Anchors Stretch (0,0)→(1,1), Offsets: All 0
│   └── Children:
│       ├── [WBP_MF_SpectatorControls] Name: "SpectatorControls"  ← REQUIRED
│       └── [WBP_MF_GameplayControls] Name: "GameplayControls"    ← REQUIRED
```

> **Note:** The Anchors/Position are set in each child's **Slot (Canvas Panel Slot)** in the Details Panel, NOT in the WBP itself.

**Widget Switcher Setup:**

1. Drag a `Widget Switcher` from Palette
2. Rename to `ModeSwitcher`, check "Is Variable"
3. Add `WBP_MF_SpectatorControls` as child, rename to `SpectatorControls`
4. Add `WBP_MF_GameplayControls` as child, rename to `GameplayControls`

### Common Binding Errors & Fixes

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

#### ⚠️ IMPORTANT: Two Different Size Concepts in UE5

There are **TWO SEPARATE** places where size is configured:

| Concept                  | Where to Set                                 | What It Controls                                  |
| ------------------------ | -------------------------------------------- | ------------------------------------------------- |
| **1. WBP Root Size**     | Designer **Toolbar** (top of Designer)       | The design-time size of this WBP's canvas         |
| **2. Child Widget Slot** | Details Panel → **Slot (Canvas Panel Slot)** | Position/anchors of widgets INSIDE a Canvas Panel |

---

#### 1️⃣ WBP Root Size (Designer Toolbar)

When you open a Widget Blueprint, the **toolbar at the top** of the Designer shows:

```
[ Screen Size ▼ ] [ Custom ▼ ] [ Width: 800 ] [ Height: 80 ]
```

| Setting         | Description                                        |
| --------------- | -------------------------------------------------- |
| **Custom**      | You set a fixed Width × Height (e.g., 800 × 80)    |
| **Desired**     | WBP auto-sizes based on content (rare for roots)   |
| **Fill Screen** | WBP matches the selected Screen Size preset        |
| **Screen Size** | Dropdown with common resolutions (1920×1080, etc.) |

**This sets the design canvas size for your WBP** - it does NOT have anchors because it's the root widget.

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

#### WBP Root Design Size (Set in Designer Toolbar)

| Widget Blueprint            | Toolbar Setting | Width  | Height | Notes                     |
| --------------------------- | --------------- | ------ | ------ | ------------------------- |
| `WBP_MF_HUD`                | **Fill Screen** | (auto) | (auto) | Main HUD, fills viewport  |
| `WBP_MF_MatchInfo`          | **Custom**      | 800 px | 80 px  | Fixed-size info bar       |
| `WBP_MF_TeamIndicator`      | **Custom**      | 180 px | 40 px  | Small team badge          |
| `WBP_MF_TransitionOverlay`  | **Fill Screen** | (auto) | (auto) | Full-screen overlay       |
| `WBP_MF_VirtualJoystick`    | **Custom**      | 200 px | 200 px | Square touch area         |
| `WBP_MF_ActionButton`       | **Custom**      | 120 px | 120 px | Large round button        |
| `WBP_MF_SprintButton`       | **Custom**      | 80 px  | 80 px  | Medium round button       |
| `WBP_MF_TeamPanel`          | **Custom**      | 280 px | 350 px | Full team roster          |
| `WBP_MF_QuickTeamPanel`     | **Custom**      | 220 px | 50 px  | Compact team bar          |
| `WBP_MF_SpectatorControls`  | **Fill Screen** | (auto) | (auto) | Full-screen control layer |
| `WBP_MF_GameplayControls`   | **Fill Screen** | (auto) | (auto) | Full-screen control layer |
| `WBP_MF_TeamSelectionPopup` | **Fill Screen** | (auto) | (auto) | Full overlay with dialog  |
| `WBP_MF_PauseMenu`          | **Fill Screen** | (auto) | (auto) | Full overlay with menu    |

#### Child Placement in HUD (Canvas Panel Slot)

When these WBPs are added as children to `WBP_MF_HUD`'s Canvas Panel:

| Child Widget               | Anchor (in HUD)      | Position Offset  | Alignment |
| -------------------------- | -------------------- | ---------------- | --------- |
| `WBP_MF_MatchInfo`         | Top-Center (0.5, 0)  | X: 0, Y: 20      | (0.5, 0)  |
| `WBP_MF_TeamIndicator`     | Top-Left (0, 0)      | X: 20, Y: 110    | (0, 0)    |
| `WBP_MF_TransitionOverlay` | Stretch (0,0)→(1,1)  | All: 0           | (0, 0)    |
| `WBP_MF_VirtualJoystick`   | Bottom-Left (0, 1)   | X: 40, Y: -240   | (0, 1)    |
| `WBP_MF_ActionButton`      | Bottom-Right (1, 1)  | X: -160, Y: -160 | (1, 1)    |
| `WBP_MF_SprintButton`      | Bottom-Right (1, 1)  | X: -140, Y: -280 | (1, 1)    |
| `WBP_MF_QuickTeamPanel`    | Center-Left (0, 0.5) | X: 10, Y: 0      | (0, 0.5)  |
| `WBP_MF_SpectatorControls` | Stretch (0,0)→(1,1)  | All: 0           | (0, 0)    |
| `WBP_MF_GameplayControls`  | Stretch (0,0)→(1,1)  | All: 0           | (0, 0)    |

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
