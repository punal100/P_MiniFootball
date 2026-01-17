# P_MiniFootball Guide

Quick checklist for running the Mini Football gameplay plugin inside the `A_MiniFootball` project.

For implementation details, see [README.md](./README.md).

## 1) Build + open

- Open `A_MiniFootball.uproject` in Unreal Engine 5.5
- Build **Development Editor**

## 2) Play (PIE)

- Menu flow: open `/P_MiniFootball/Maps/L_MainMenu` and Play In Editor
- Gameplay-only: open `/P_MiniFootball/Maps/L_MiniFootball` and Play In Editor

Recommended per-level overrides:

- `L_MainMenu`: GameMode `BP_MF_MenuGameMode`, PlayerController `BP_MF_MenuPlayerController` (spawns the configured Main Menu widget).
- `L_MiniFootball`: GameMode `BP_MF_GameMode`, PlayerController `BP_MF_PlayerController` (gameplay PC is responsible for spawning `WBP_MF_HUD`).

## Field setup (automatic)

When you place `AMF_Field` in your level, it automatically creates:

- **2 Goals**: Positioned at each end of the field
- **2 Penalty Areas**: Inside the field at each goal end

### Quick verification

1. Place `MF_Field` (or `BP_MF_Field`) in your level
2. Confirm 2 `MF_Goal` and 2 `MF_PenaltyArea` actors appear in the World Outliner
3. Enable debug visualization on the Field actor:
    - `bShowFieldDebug`
    - `bShowGoalDebug`
    - `bShowPenaltyAreaDebug`

> Note: Debug visualization is compiled out of Shipping builds.

## 3) Multiplayer (fastest)

- In Editor: set “Number of Players” to 2+
- Run PIE

## 4) Dedicated server correctness

Dedicated server needs to start on the gameplay map.

- Confirm `ServerDefaultMap=/P_MiniFootball/Maps/L_MiniFootball` in `Config/DefaultEngine.ini`

## 5) Input pipeline (unified)

- Gameplay input is handled via **P_MEIS**.
- Mobile UI (virtual joystick/buttons) injects into P_MEIS so keyboard/gamepad and UI share the same gameplay path.

Critical init order (prevents empty Input Settings list):

- Initialize P_MEIS for the local PlayerController and load/apply a template (typically `Default`) before opening Settings → Input.

Recommended (single-call):

- Call `EnsureInputProfileReady("Default")` on the local `AMF_PlayerController`.

Behavior:

- Registers the local player with P_MEIS if needed
- Creates `Saved/InputProfiles/Default.json` if it doesn't exist yet
- Applies the template to the local player (and applies to Enhanced Input)

## 6) Settings → Input

The Pause menu opens the Settings menu.

In this project, settings are implemented as overlays and MWCS commonly generates the main settings hub as:

- `/Game/UI/Widgets/WBP_MF_MainSettings`

At runtime, the class used for Main Menu / Settings widgets is resolved via `UMF_WidgetClassSettings` (Project Settings) and can optionally be overridden via JSON (see plugin README).

If you change widget specs, regenerate the widget assets via MWCS (see the top-level project docs or P_MWCS docs).

Note:

- `WBP_MF_InputSettings` builds its action list using `UMF_InputActionRow` instances created at runtime (no MWCS-generated `WBP_MF_InputActionRow`, and it does not appear in `Config/DefaultEditor.ini`).

Input Settings UX:

- `WBP_MF_InputSettings` includes a profile dropdown (templates from P_MEIS) and a **DEFAULT** button.
- If the list is empty, select `Default` (or click **DEFAULT**) and confirm the local PlayerController has initialized P_MEIS.

## Spectator ↔ Player transitions (client UI)

If your HUD needs to react cleanly when a player joins/leaves a team (spectator → playing and back), bind to events on `AMF_PlayerController`:

- `OnSpectatorStateChanged(Controller, NewState)`
- `OnPlayerRoleChanged(Controller, bIsPlaying)` (high-level “spectating vs playing” switch)
- `OnMFPossessedPawnChanged(Controller, NewPawn)` (fires for OnPossess/OnUnPossess)

## UI ownership (recommended)

- Prefer spawning/owning widgets on the local `AMF_PlayerController` (client) using `CreateGameplayUI`, `CreateSpectatorUI`, and `ClearUI`.
- `AMF_GameMode` also exposes optional Blueprint hooks (`CreateGlobalUI`, `CreatePlayerUI`) but GameMode is server-only; keep client widget creation on the owning client.

---

## AI Characters (P_EAIS Integration)

All match characters are `AMF_AICharacter` instances. When not controlled by a human, they run AI behavior.

### Default Behavior

- **Match Start**: All 22 characters are AI-controlled
- **Human Joins**: Takes control of one character → AI stops for that character
- **Human Switches (Q)**: Previous character resumes AI → New character's AI stops
- **Human Leaves**: Character immediately resumes AI

### Quick Test

1. Open `L_MiniFootball` map
2. Play In Editor (2 players)
3. Observe all characters moving (AI - 22 players total)
4. Join a team → One character now human-controlled
5. Press Q → Switch characters, previous one resumes AI

### Console Commands

```cpp
EAIS.Debug 1               // Show AI state above characters
EAIS.ListActions           // List available AI actions
EAIS.SpawnBot 1 Striker    // Spawn additional AI bot
```

### Visual Indicators (Player Labels)

- **Text**: Displays the player's role (e.g., Striker, Defender).
- **Color**: Blue (Team A), Red (Team B), White (None/Neutral).
- **Replication**: Labels are fully replicated; all clients see the same roles and colors.
- **Customization**: Appearance is driven by `UTextRenderComponent` attachment in `MF_PlayerCharacter.cpp`.

### Customizing AI

Edit profiles in `Content/AIProfiles/`:

- `Striker.json` - Offensive behavior
- `Midfielder.json` - Midfield control
- `Defender.json` - Defensive behavior
- `Goalkeeper.json` - Goal protection

**Standard Blackboard Keys:**
- `Ball` (Vector): Current ball position
- `Goal_Opponent` (Vector): Location of opponent's goal
- `Home` (Vector): Home formation position for this agent
- `Role` (String): Agent role (Striker, Defender, etc.)
- `HasBall` (Bool): Does this agent have the ball?
- `IsBallLoose` (Bool): Is the ball currently unpossessed?

See [P_EAIS GUIDE.md](../P_EAIS/GUIDE.md) for authoring custom behaviors.

---

## 🚧 Planned Enhancements (TODO)

- **Standard Football Rules**: Implementation of Throw-ins, Corner Kicks, and Free Throws. The system currently resets the ball to the center circle for all out-of-bounds events.

---

## Authority Architecture

### The Three Pillars

#### 1. Input Authority (P_MEIS)

```
Physical Key
→ EnhancedInputSystem
→ P_MEIS Integration (broadcasts by ActionName)
→ MF_InputHandler (subscribes by ActionName)
→ Gameplay (receives signal)
```

**Rules:**

- ✓ Bind to Integration delegate (re-get from Manager after rebind)
- ✓ Route by ActionName (FName, stable)
- ✗ Never cache `UInputAction*`
- ✗ Never bind to `EnhancedInputComponent` directly

#### 2. Gameplay Authority (Single Possession)

```
Possession State
→ One PlayerController
→ One Possessed Pawn
→ Never arrays, never cycles
```

**Rules:**

- ✓ Possess nearest-to-ball (algorithmic via `SwitchToNearestToBall()`)
- ✓ Unpossess old before possessing new
- ✗ Never assume array ordering
- ✗ Never register AI as controllable

#### 3. UI Authority (HUD Routing)

```
Widget Intent (delegate broadcast)
→ HUD Listener
→ HUD Action (open widget, etc.)
```

**Rules:**

- ✓ Widget emits intent (e.g. `OnRequestTeamChange`)
- ✓ HUD executes action (e.g. `ShowTeamSelectionPopup()`)
- ✗ Widget never opens widgets directly
- ✗ Widget never calls gameplay directly

### Authoritative APIs (GameState)

```cpp
// Get the match ball (NEVER spawn - always resolve)
AMF_Ball* Ball = GameState->GetMatchBall();

// Get team for controller (authoritative)
EMF_TeamID Team = GameState->GetTeamForController(PlayerController);
```

### Verification

All architectural constraints are verifiable headlessly:

```powershell
# Verify code patterns
PowerShell -ExecutionPolicy Bypass -File DevTools/scripts/Verify_CodePatterns.ps1

# Verify ActionName parity
PowerShell -ExecutionPolicy Bypass -File DevTools/scripts/Verify_ActionNameParity.ps1

# Verify Player Indicators (Headless Build)
PowerShell -ExecutionPolicy Bypass -File DevTools/scripts/Verify_Build_Indicators.ps1
```

---

## A_WCG Integration (HTML to Widget)

A_WCG (Atomic Web Component Generator) converts HTML/CSS to Unreal Engine widget specifications.

### Automated Widget Testing

Run the automated test script for end-to-end widget generation and validation:

```powershell
# Navigate to P_MiniFootball DevTools scripts folder
cd D:\Projects\UE\A_MiniFootball\Plugins\P_MiniFootball\DevTools\scripts

# Full pipeline (builds A_WCG, generates, copies, validates)
.\TestWidgetGeneration.ps1

# Skip build if already built
.\TestWidgetGeneration.ps1 -SkipBuild

# Custom source HTML
.\TestWidgetGeneration.ps1 -SourceHtml "path\to\source.html" -ClassName "MyWidget"

# Validation only (no generation)
.\TestWidgetGeneration.ps1 -ValidateOnly

# Recreate Widget Blueprint (force update)
.\TestWidgetGeneration.ps1 -Recreate -SkipBuild
```

Note: `TestWidgetGeneration.ps1` uses a shared build helper for the UE build step:

- `DevTools/scripts/MF_BuildCommon.ps1`

### What the Script Does

1. **Builds A_WCG** (Release configuration)
2. **Generates files** from source HTML (JSON, .h, .cpp, \_preview.html)
3. **Copies to Source** (`Base\UI\` folder)
4. **Builds Project** (`A_MiniFootballEditor` target)
5. **Runs MWCS validation** (headless, displays errors)

### Output Locations

- **Generated files**: `Plugins\P_MiniFootball\Source\P_MiniFootball\Base\UI\`
- **Validation output**: `Plugins\P_MWCS\A_WCG\generated\validation_output.txt`
- **MWCS reports**: `Saved\MWCS\Reports\`

### Generated Widget Structure

A_WCG generates widgets with this hierarchy:

```
RootCanvas (CanvasPanel)
└── ContentScroll (ScrollBox) ← Fills canvas, enables scrolling
    └── ContentRoot (VerticalBox) ← Flow layout container
        └── [HTML Content]
```
