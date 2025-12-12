# P_MiniFootball - Mini Football System

A gameplay plugin for Unreal Engine 5.5 implementing a fast-paced arcade-style mini football game with full multiplayer support.

---

## 🎮 Key Features

- **Math-Based Ball Physics**: Custom velocity-based movement (NO UE Physics Engine) with gravity, friction, and bounce mechanics
- **Server Authoritative Networking**: Full multiplayer support for Listen Server and Dedicated Server configurations
- **P_MEIS Integration**: All input handling via P_MEIS plugin - no hardcoded key bindings
- **Possession System**: Ball attaches to possessing player with automatic pickup radius
- **Kick Mechanics**: Shoot and Pass actions with directional control and power
- **Tackle System**: Server-validated tackles with cooldown and stun mechanics
- **Match Flow**: Complete match lifecycle (Kickoff, Playing, GoalScored, HalfTime, MatchEnd)
- **Team Management**: 3v3 format with team rosters and character switching
- **Spectator System**: Players start as spectators and request to join teams via widgets
- **Blueprint Interfaces**: Clean separation between GameMode (server) and PlayerController (client)

---

## 📋 Implementation Status

| Phase    | Description                         | Status         |
| -------- | ----------------------------------- | -------------- |
| Phase 1  | Core Plugin Setup                   | ✅ COMPLETE    |
| Phase 2  | Player Controller & Movement (+Net) | ✅ COMPLETE    |
| Phase 3  | Ball System & Possession (+Net)     | ✅ COMPLETE    |
| Phase 4  | Shooting & Passing (+Net)           | ✅ COMPLETE    |
| Phase 5  | Goal & Scoring System (+Net)        | ✅ COMPLETE    |
| Phase 6  | Match Flow & Game Modes (+Net)      | ✅ COMPLETE    |
| Phase 7  | Simple AI (Optional)                | ⏳ DEFERRED    |
| Phase 8  | Polish & Mobile Optimization        | ❌ NOT STARTED |
| Phase 9  | Spectator & Team Assignment (+Net)  | ✅ COMPLETE    |
| Phase 10 | UI Widget System (C++)              | ✅ COMPLETE    |

---

## 🔧 Technical Specifications

### Ball Physics (Math-Based)

| Property           | Value     |
| ------------------ | --------- |
| Shoot Speed        | 2500 cm/s |
| Pass Speed         | 1200 cm/s |
| Friction           | 500 cm/s² |
| Gravity            | 980 cm/s² |
| Bounce Restitution | 0.7       |
| Pickup Radius      | 150 cm    |

### Player Movement

| Property     | Value      |
| ------------ | ---------- |
| Walk Speed   | 400 cm/s   |
| Sprint Speed | 600 cm/s   |
| Acceleration | 2000 cm/s² |
| Turn Rate    | 540 °/s    |

### Field Dimensions

| Property     | Value         |
| ------------ | ------------- |
| Field Length | 4000 cm (40m) |
| Field Width  | 2500 cm (25m) |
| Goal Width   | 400 cm (4m)   |
| Goal Height  | 200 cm (2m)   |

---

## 👤 Author

**Punal Manalan**

---

## 📦 Requirements

- Unreal Engine 5.5+
- P_MEIS Plugin (Modular Enhanced Input System)

---

## 🚀 Installation

1. Clone or copy this plugin to your project's `Plugins/` folder
2. Ensure P_MEIS plugin is also installed
3. Regenerate project files
4. Enable the plugin in your project settings
5. Add `"P_MiniFootball"` to your module's `.Build.cs` dependencies

```csharp
PublicDependencyModuleNames.AddRange(new string[] { "P_MiniFootball" });
```

---

## 📁 Folder Structure

```
P_MiniFootball/
├── P_MiniFootball.uplugin     # Plugin descriptor
├── README.md                   # This file
├── UI_WIDGETS.md              # UI widget binding reference & visual specs
├── Resources/
│   └── Icon128.png            # Plugin icon
├── Scripts/
│   ├── MF_WidgetBlueprintCreator.py  # Automated WBP creation from JSON specs
│   └── EDITOR_UTILITY_WIDGET_GUIDE.md # EUW setup instructions
└── Source/
    └── P_MiniFootball/
        ├── P_MiniFootball.Build.cs    # Module build configuration
        ├── Public/
        │   └── P_MiniFootball.h       # Module interface
        ├── Private/
        │   └── P_MiniFootball.cpp     # Module implementation
        └── Base/
            ├── Core/
            │   └── MF_Types.h         # Enums, constants, replication structs
            ├── Interfaces/
            │   ├── MF_TeamInterface.h          # GameMode team interface
            │   └── MF_PlayerControllerInterface.h  # PlayerController callbacks
            ├── Player/
            │   ├── MF_InputHandler.h/.cpp      # P_MEIS integration
            │   ├── MF_PlayerCharacter.h/.cpp   # Replicated player (top-down camera)
            │   └── MF_PlayerController.h/.cpp  # Network controller + team RPCs
            ├── Spectator/
            │   └── MF_Spectator.h/.cpp         # Spectator pawn
            ├── Ball/
            │   └── MF_Ball.h/.cpp     # Math-based ball physics
            ├── Match/
            │   ├── MF_GameMode.h/.cpp    # Server-only game mode + team management
            │   ├── MF_GameState.h/.cpp   # Replicated match state + team rosters
            │   └── MF_Goal.h/.cpp        # Goal trigger volume
            ├── UI/                        # Self-describing widget classes (JSON specs)
            │   ├── MF_HUD.h/.cpp          # Main HUD + GetWidgetSpec()
            │   ├── MF_MatchInfo.h/.cpp    # Score/time + GetWidgetSpec()
            │   ├── MF_TeamIndicator.h/.cpp
            │   ├── MF_TransitionOverlay.h/.cpp
            │   ├── MF_SpectatorControls.h/.cpp
            │   ├── MF_GameplayControls.h/.cpp
            │   ├── MF_VirtualJoystick.h/.cpp
            │   ├── MF_ActionButton.h/.cpp
            │   ├── MF_SprintButton.h/.cpp
            │   ├── MF_TeamSelectionPopup.h/.cpp
            │   ├── MF_TeamPanel.h/.cpp
            │   ├── MF_QuickTeamPanel.h/.cpp
            │   └── MF_PauseMenu.h/.cpp
            └── Network/               # Reserved for future use
```

---

## 🎯 Core Classes

| Class                  | Description                                            |
| ---------------------- | ------------------------------------------------------ |
| `AMF_PlayerCharacter`  | Replicated player with top-down camera and Server RPCs |
| `AMF_PlayerController` | Network-aware controller with character switching      |
| `UMF_InputHandler`     | P_MEIS integration component                           |
| `AMF_Ball`             | Math-based ball with possession and kick mechanics     |
| `AMF_GameMode`         | Server-only match management                           |
| `AMF_GameState`        | Replicated scores, time, and match phase               |
| `AMF_Goal`             | Goal trigger volume for detecting ball entry           |
| `AMF_Spectator`        | Spectator pawn for viewing matches before joining      |

---

## 🎭 Spectator & Team Assignment System (Phase 9)

Players start as spectators and request to join teams through widgets. The system uses Blueprint Interfaces for clean separation between server (GameMode) and client (PlayerController) logic.

### Flow Overview

```
Player Joins → Spawns as AMF_Spectator → Opens Team Widget → Server_RequestJoinTeam()
                                                                    ↓
                                         GameMode.HandleJoinTeamRequest() (Server)
                                                                    ↓
                                         Validate → Assign Character → Possess
                                                                    ↓
                                         Client_OnTeamAssignmentResponse() → Widget closes
```

### Key Blueprint Interfaces

#### IMF_TeamInterface (GameMode implements)

| Function                 | Description                               |
| ------------------------ | ----------------------------------------- |
| `HandleJoinTeamRequest`  | Process team join request, returns result |
| `HandleLeaveTeamRequest` | Process team leave request                |
| `CanPlayerJoinTeam`      | Check if player can join specific team    |
| `IsTeamFull`             | Check if team is at capacity              |
| `GetTeamPlayerCount`     | Get number of human players on team       |
| `GetAvailableTeams`      | Get teams player can join (balance rules) |
| `GetMaxPlayersPerTeam`   | Get max human players per team            |
| `IsMidMatchJoinAllowed`  | Check if mid-match joining is enabled     |

#### IMF_PlayerControllerInterface (PlayerController implements)

| Function                   | Description                                 |
| -------------------------- | ------------------------------------------- |
| `OnTeamAssignmentResponse` | Called when server responds to join/leave   |
| `OnTeamStateChanged`       | Called when team or spectator state changes |
| `OnPossessedPawnChanged`   | Called when possessed pawn changes          |
| `GetCurrentTeamID`         | Get current team assignment                 |
| `GetCurrentSpectatorState` | Get current state (Spectating/Playing)      |
| `IsSpectating`             | Quick check if currently spectating         |

### Server RPCs (Call from Widgets)

```cpp
// In your Widget Blueprint:

// Join Team A button clicked:
GetOwningPlayer() → Cast to AMF_PlayerController → Server_RequestJoinTeam(EMF_TeamID::TeamA)

// Leave Team button clicked:
GetOwningPlayer() → Cast to AMF_PlayerController → Server_RequestLeaveTeam()
```

### Events for Widget Feedback

```cpp
// Bind these in your Widget to receive server responses:

UPROPERTY(BlueprintAssignable)
FOnTeamAssignmentResponseDelegate OnTeamAssignmentResponseReceived;
// Params: bool bSuccess, EMF_TeamID Team, FString ErrorMessage

UPROPERTY(BlueprintAssignable)
FOnSpectatorStateChanged OnSpectatorStateChanged;
// Params: AMF_PlayerController* Controller, EMF_SpectatorState NewState
```

### Team Balance Rules

- **Equal teams**: Player can join either team
- **Uneven teams**: Player can ONLY join team with fewer players
- **Team full**: Cannot join, widget shows available team instead
- **Mid-match join**: Controlled by `bAllowMidMatchJoin` (default: true)

---

## 🌐 Network Architecture

- **Server Authoritative**: All game logic validated on server
- **Client RPCs**: `Server_RequestShoot`, `Server_RequestPass`, `Server_RequestTackle`
- **Replication**: `DOREPLIFETIME` macros with `ReplicatedUsing` for rep notifies
- **Interpolation**: Client-side ball position smoothing

---

## 🎨 UI Widget System (Phase 10)

The plugin includes a complete C++ UI widget system for HUD, spectator controls, team selection, and gameplay controls. All widgets are designed as C++ `UUserWidget` base classes that can be extended in Blueprint.

**Each widget class is self-describing** — containing embedded JSON specifications that define hierarchy, bindings, layout, and design rules.

### Widget Architecture

```
UMF_HUD (Main Container)
├── UMF_MatchInfo (Score/Time - Always visible)
├── UMF_TeamIndicator (Current team display)
├── UMF_TransitionOverlay (Loading/state transitions)
├── UWidgetSwitcher
│   ├── [Spectator Mode]
│   │   └── UMF_SpectatorControls
│   │       ├── UMF_QuickTeamPanel (Team A)
│   │       └── UMF_QuickTeamPanel (Team B)
│   └── [Gameplay Mode]
│       └── UMF_GameplayControls
│           ├── UMF_VirtualJoystick
│           ├── UMF_ActionButton
│           └── UMF_SprintButton
├── UMF_TeamSelectionPopup (Modal)
│   ├── UMF_TeamPanel (Team A)
│   └── UMF_TeamPanel (Team B)
└── UMF_PauseMenu (Modal)
```

### Widget Classes Reference

| Widget Class             | File                           | Description                             |
| ------------------------ | ------------------------------ | --------------------------------------- |
| `UMF_HUD`                | `MF_HUD.h/.cpp`                | Main HUD container with widget switcher |
| `UMF_MatchInfo`          | `MF_MatchInfo.h/.cpp`          | Score and time display                  |
| `UMF_TeamIndicator`      | `MF_TeamIndicator.h/.cpp`      | Current team indicator                  |
| `UMF_TransitionOverlay`  | `MF_TransitionOverlay.h/.cpp`  | Loading/transition screen               |
| `UMF_SpectatorControls`  | `MF_SpectatorControls.h/.cpp`  | Spectator mode UI                       |
| `UMF_GameplayControls`   | `MF_GameplayControls.h/.cpp`   | Mobile touch controls container         |
| `UMF_VirtualJoystick`    | `MF_VirtualJoystick.h/.cpp`    | Touch joystick for movement             |
| `UMF_ActionButton`       | `MF_ActionButton.h/.cpp`       | Context-sensitive action button         |
| `UMF_SprintButton`       | `MF_SprintButton.h/.cpp`       | Hold-to-sprint button                   |
| `UMF_TeamSelectionPopup` | `MF_TeamSelectionPopup.h/.cpp` | Modal team selection dialog             |
| `UMF_TeamPanel`          | `MF_TeamPanel.h/.cpp`          | Reusable team info panel                |
| `UMF_QuickTeamPanel`     | `MF_QuickTeamPanel.h/.cpp`     | Compact team preview with quick join    |
| `UMF_PauseMenu`          | `MF_PauseMenu.h/.cpp`          | In-game pause menu                      |

> **📖 Full UI Documentation:** See [UI_WIDGETS.md](./UI_WIDGETS.md) for complete widget binding reference, visual design specs, and step-by-step creation guide.

---

## 🧬 Self-Describing Widget System (JSON Specifications)

Each UMF widget class contains an embedded **JSON specification** accessible via `GetWidgetSpec()`. This makes widgets self-documenting and enables automated Widget Blueprint creation.

> Note: `GetWidgetSpec()` is exposed via `UFUNCTION` and should return **by value** (`FString`), not `const FString&`, for reflection compatibility (Blueprint/Python).

### Key Benefits

| Feature                    | Description                                         |
| -------------------------- | --------------------------------------------------- |
| **Self-Documentation**     | Widget structure defined alongside C++ code         |
| **Automated WBP Creation** | Python script reads JSON to build Widget Blueprints |
| **Single Source of Truth** | No duplicate definitions in Python or documentation |
| **Designer-Friendly**      | Clear binding contracts visible in code             |
| **Dependency Management**  | Build order automatically resolved from JSON        |

### JSON Specification Contents

Each widget's JSON includes:

- **WidgetClass** — C++ class name
- **BlueprintName** — Target WBP asset name (e.g., `WBP_MF_ActionButton`)
- **ParentClass** — Full path to C++ parent class
- **DesignerToolbar** — Size mode, dimensions for UMG Designer
- **Hierarchy** — Complete widget tree structure
- **Bindings** — Required and optional widget bindings
- **Design** — Colors, fonts, styles per widget
- **Delegates** — Events exposed to Blueprint
- **Dependencies** — Other WBPs required before this one
- **PythonSnippets** — Code examples for automation

### Accessing Widget Specs

```cpp
// C++ - Get JSON spec from any UMF widget class
FString JsonSpec = UMF_ActionButton::GetWidgetSpec();
FString HudSpec = UMF_HUD::GetWidgetSpec();

// The JSON can be parsed to get widget structure
// Useful for tooling, validation, and automation
```

```python
# Python (UE5 Editor) - Read spec for automation
import unreal
import json

# Get spec from widget class CDO
action_btn_cdo = unreal.get_default_object(unreal.MF_ActionButton)
json_str = unreal.MF_ActionButton.get_widget_spec()
spec = json.loads(json_str)

print(spec["BlueprintName"])  # "WBP_MF_ActionButton"
print(spec["Bindings"]["Required"])  # ["ActionButton"]
```

### Example: UMF_ActionButton JSON Spec

```json
{
  "WidgetClass": "UMF_ActionButton",
  "BlueprintName": "WBP_MF_ActionButton",
  "ParentClass": "/Script/P_MiniFootball.MF_ActionButton",
  "Category": "MF|UI|Controls",
  "Description": "Context-sensitive action button for Shoot/Pass/Tackle",

  "DesignerToolbar": {
    "DesiredSize": { "Width": 120, "Height": 120 }
  },

  "Hierarchy": {
    "Root": { "Type": "CanvasPanel", "Name": "RootPanel" },
    "Children": [
      {
        "Name": "ActionButton",
        "Type": "Button",
        "BindingType": "Required",
        "Slot": {
          "Anchors": { "Min": [0, 0], "Max": [1, 1] },
          "Offsets": { "Left": 0, "Top": 0, "Right": 0, "Bottom": 0 }
        }
      },
      {
        "Name": "ActionIcon",
        "Type": "Image",
        "BindingType": "Optional"
      },
      {
        "Name": "ActionText",
        "Type": "TextBlock",
        "BindingType": "Optional"
      }
    ]
  },

  "Bindings": {
    "Required": ["ActionButton"],
    "Optional": ["ActionIcon", "ActionText"]
  },

  "Delegates": [
    { "Name": "OnActionPressed", "Type": "FOnButtonClickedEvent" },
    { "Name": "OnActionReleased", "Type": "FOnButtonReleasedEvent" }
  ],

  "Dependencies": {
    "RequiredWidgets": [],
    "OptionalWidgets": []
  }
}
```

### Widget Build Order

The `UMF_HUD::GetWidgetSpec()` contains the complete build order for all widgets:

1. `WBP_MF_ActionButton` — No dependencies
2. `WBP_MF_VirtualJoystick` — No dependencies
3. `WBP_MF_SprintButton` — No dependencies
4. `WBP_MF_MatchInfo` — No dependencies
5. `WBP_MF_TeamIndicator` — No dependencies
6. `WBP_MF_TransitionOverlay` — No dependencies
7. `WBP_MF_QuickTeamPanel` — No dependencies
8. `WBP_MF_TeamPanel` — No dependencies
9. `WBP_MF_SpectatorControls` — Requires QuickTeamPanel
10. `WBP_MF_GameplayControls` — Requires VirtualJoystick, ActionButton, SprintButton
11. `WBP_MF_TeamSelectionPopup` — Requires TeamPanel
12. `WBP_MF_PauseMenu` — No dependencies
13. `WBP_MF_HUD` — Requires all above (master widget)

### Automated WBP Creation

A Python script (`Scripts/MF_WidgetBlueprintCreator.py`) reads these JSON specs to:

- Create Widget Blueprints with correct parent classes
- Build widget hierarchies automatically
- Apply layout rules (anchors, positions, sizes)
- Apply theme colors and fonts
- Validate required bindings
- Generate binding documentation

#### Option 1: Editor Utility Widget (Recommended)

Create a visual tool with buttons to run the script:

1. Create **Editor Utility Widget** in Content Browser
2. Add buttons for each action (Preview, Create, Validate)
3. Use **Execute Python Script** node with commands below
4. Run via right-click → "Run Editor Utility Widget"

> **📖 Setup Guide:** See [Scripts/EDITOR_UTILITY_WIDGET_GUIDE.md](./Scripts/EDITOR_UTILITY_WIDGET_GUIDE.md) for step-by-step EUW creation.

You can also auto-create/repair the EUW **asset shell** (correct C++ parent) via:

> Note: the script does not generate or inspect the EUW designer widget tree. Build the EUW UI layout manually using the guide.

```python
exec(open("D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); MF_WBP.run_create_euw()
```

#### Option 2: Python Console

```python
# Run in UE5 Python console (Output Log → switch to Python mode)
exec(open("D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read())

# The script defines an `MF_WBP` module alias for convenience in the same console session.

# Quick functions via the MF_WBP alias (EUW-friendly helpers):
MF_WBP.run_dry()            # Preview what would happen
MF_WBP.run_create()         # Create missing widgets
MF_WBP.run_create_euw()     # Create/repair the EUW tool asset
run_only_euw()              # Create ONLY the EUW (skip all WBPs)
MF_WBP.run_validate()       # Validate existing widgets
MF_WBP.run_force_recreate() # Recreate all widgets (CAUTION!)
print(MF_WBP.get_status_text())  # Optional status dump for EUW

# Diagnostic functions (for troubleshooting widget tree/binding issues):
diagnose_all_mf_widgets()   # Check all MF widgets for issues
diagnose_widget_blueprint("/P_MiniFootball/BP/Widget/Components/WBP_MF_HUD")
diagnose_missing_bindings("/P_MiniFootball/BP/Widget/Components/WBP_MF_HUD", "/Script/P_MiniFootball.MF_HUD")
```

> **📖 Automation Documentation:** See [PLAN_WBP_SCRIPT.md](../../PLAN_WBP_SCRIPT.md) for complete automation system documentation.

---

## 📝 Version History

| Version | Date       | Changes                                                      |
| ------- | ---------- | ------------------------------------------------------------ |
| 1.0     | 07/12/2025 | Initial plugin implementation (Phases 1-6)                   |
| 1.1     | 11/12/2025 | Added self-describing JSON widget specifications (Phase 10)  |
| 1.2     | 11/12/2025 | Added Editor Utility Widget guide for WBP Creator tool       |
| 1.3     | 12/12/2025 | Added diagnostic functions and only_euw mode for WBP Creator |
| 1.4     | 12/12/2025 | EUW is shell-only (skip EUW validation/tree/diagnostics)     |

---

## 📄 License

This plugin is part of the A_MiniFootball project.
