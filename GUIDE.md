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
PowerShell -ExecutionPolicy Bypass -File Scripts/Verify_CodePatterns.ps1

# Verify ActionName parity
PowerShell -ExecutionPolicy Bypass -File Scripts/Verify_ActionNameParity.ps1
```

