# P_MiniFootball - Mini Football System

A gameplay plugin for Unreal Engine 5.5 implementing a fast-paced arcade-style mini football game with full multiplayer support.

For a quick project-oriented checklist, see [GUIDE.md](./GUIDE.md).

---

## 🎮 Key Features

- **Math-Based Ball Physics**: Custom velocity-based movement (NO UE Physics Engine) with gravity, friction, and bounce mechanics
- **Server Authoritative Networking**: Full multiplayer support for Listen Server and Dedicated Server configurations
- **P_MEIS Integration**: All input handling via P_MEIS plugin - no hardcoded key bindings
- **Possession System**: Ball attaches to possessing player with automatic pickup radius
- **Kick Mechanics**: Shoot and Pass actions with directional control and power
- **Tackle System**: Server-validated tackles with cooldown and stun mechanics
- **Match Flow**: Complete match lifecycle (Kickoff, Playing, GoalScored, HalfTime, MatchEnd)
- **Team Management**: 11v11 format with team rosters and character switching
- **Formation System**: Position-based roles (GK, DEF, MID, STR) automatically assigned on spawn
- **Smart AI System**: Diverse AI behaviors (Striker, Midfielder, Defender, Goalkeeper) with role-specific positioning and obstacle avoidance
- **Intelligent Support**: Teammates support attacks or defenses based on ball position and role
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
| Phase 7  | AI System (P_EAIS Integration)      | ✅ COMPLETE - OPTIMIZED |
| Phase 8  | Polish & Mobile Optimization        | ❌ NOT STARTED |
| Phase 9  | Spectator & Team Assignment (+Net)  | ✅ COMPLETE    |
| Phase 10 | UI Widget System (C++)              | ✅ COMPLETE    |

> [!NOTE] > **Phase 7 - AI System**: The P_EAIS integration is fully functional. Recent updates (Jan 2026) include:
> - **Pressing Logic**: Defenders and Midfielders actively press the ball carrier (`PressBall` state).
> - **Zonal Defense**: Defenders maintain formation using calculated support positions rather than chasing random targets.
> - **Tackle Implementation**: Improved tackle ranges (300cm) and dedicated states for Midfielders.
> - **Aggressive Defense**: AI engages the ball if it enters a 10m proximity zone, regardless of formation.
> - **Striker Fixes**: Resolved freezing issues and improved positioning.

---

## 🎯 Core Control Model

### Single Possession Principle

- Player controls **exactly ONE character** at any time
- AI teammates are NOT controllable
- Switching = unpossess current pawn, possess new pawn

### Character Selection (Q Key)

- Selects the **teammate closest to the ball**
- Not array cycling
- No fallback behavior
- Deterministic and provable

---

## ⌨️ Input System Contract (P_MEIS Integration)

### Fundamental Rules

- `UInputAction` objects are **ephemeral** (created/destroyed on profile reapplication)
- `FName` (ActionName) is **eternal** and stable
- Gameplay binds to **P_MEIS Integration-level delegates** via ActionName dispatch
- Gameplay NEVER caches `UInputAction*` pointers
- Gameplay NEVER binds to `EnhancedInputComponent` directly

### Why This Matters

- Rebinding a key destroys all old InputActions
- If you cached a pointer, it now dangles (invalid)
- Binding by ActionName (FName) always works

---

## 🖼️ UI Authority (P_MWCS Integration)

### Intent Flow

```
Widget → emits delegate
HUD → listens to delegate, routes to gameplay
Gameplay → executes action
```

- Widgets **never open other widgets** directly
- Widgets **never call gameplay code** directly
- HUD is **the only UI routing authority**
- Explicit contracts prevent silent failures

---

## 🧪 Verification (No Editor Required)

This plugin includes command-line verifiable checks under `DevTools/scripts/`.

For full project build + Unreal Automation tests, use the project’s build tasks / Unreal build tooling (and see the top-level project docs).

### Run Code Pattern Verification

```powershell
PowerShell -ExecutionPolicy Bypass -File ./DevTools/scripts/Verify_CodePatterns.ps1
```

### Run ActionName Parity Verification

```powershell
PowerShell -ExecutionPolicy Bypass -File ./DevTools/scripts/Verify_ActionNameParity.ps1
```

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

### Field Dimensions (FIFA Standard)

| Property     | Value             |
| ------------ | ----------------- |
| Field Length | 10500 cm (105m)   |
| Field Width  | 6800 cm (68m)     |
| Goal Width   | 732 cm (7.32m)    |
| Goal Height  | 244 cm (2.44m)    |

---

## 👤 Author

**Punal Manalan**

---

## 📦 Requirements

- Unreal Engine 5.5+
- P_MEIS Plugin (Modular Enhanced Input System)
- P_MWCS Plugin (Modular Widget Creation System) for editor-time Widget Blueprint generation/validation (editor-only)
- A_WCG Tool (optional, for HTML-to-Widget conversion) - located at `Plugins/P_MWCS/A_WCG/`

## 🧩 MWCS-generated UI (this project)

This plugin’s UI widgets are implemented as C++ `UUserWidget` classes with a `static FString GetWidgetSpec()` JSON spec.

MWCS generates/repairs Blueprint assets under the project output root (commonly `/Game/UI/Widgets`).

Notable generated widgets include:

- `/Game/UI/Widgets/WBP_MF_MainMenu` (main menu)
- `/Game/UI/Widgets/WBP_MF_MainSettings` (settings hub; opened from Main Menu + Pause Menu)
- `/Game/UI/Widgets/WBP_MF_InputSettings` (dynamic rebinding UI)
- `UMF_InputActionRow` (runtime-created rows inside `WBP_MF_InputSettings`; no MWCS-generated `WBP_*` asset)
- `/Game/UI/Widgets/WBP_MF_ToggleActionButton` (generic hold/toggle action button)
- `/Game/UI/Widgets/WBP_MF_HUD` (gameplay HUD)

## ⚙️ Widget class configuration (no hard-coded asset paths)

Widget Blueprint classes are resolved at runtime via:

- Project Settings → **P_MiniFootball** → **MF Widget Class Settings** (`UMF_WidgetClassSettings`)
- Optional JSON overrides (enabled by `bAutoLoadJsonConfig` + `JsonConfigPath`)

You can also reload JSON at runtime via the console command:

- `MF.WidgetConfig.Reload`

## 🗺️ Maps & UI setup (recommended)

Separate menu vs gameplay by using different GameModes / PlayerControllers.

### L_MainMenu

- Map: `/P_MiniFootball/Maps/L_MainMenu`
- World Settings overrides:
  - GameMode Override: `BP_MF_MenuGameMode`
  - PlayerController Class: `BP_MF_MenuPlayerController`
- Root widget: `WBP_MF_MainMenu`

Root widget is resolved from the widget configuration (default points at `WBP_MF_MainMenu`).

### L_MiniFootball

- Map: `/P_MiniFootball/Maps/L_MiniFootball`
- World Settings overrides:
  - GameMode Override: `BP_MF_GameMode`
  - PlayerController Class: `BP_MF_PlayerController`
- Root widget: `WBP_MF_HUD`

Menu UI is created by the menu PlayerController. Gameplay UI should be created by the gameplay PlayerController (Blueprint or C++ subclass).

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
├── Content/
│   └── AIProfiles/            # AI runtime profiles (*.runtime.json)
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
            │   ├── MF_ToggleActionButton.h/.cpp
            │   ├── MF_SprintButton.h/.cpp         # Legacy/compat widget (not used by default HUD)
            │   ├── MF_MainMenu.h/.cpp
            │   ├── MF_MainSettings.h/.cpp
            │   ├── MF_InputSettings.h/.cpp
            │   ├── MF_InputActionRow.h/.cpp      # Runtime-created row (NOT MWCS; no WBP asset)
            │   ├── MF_AudioSettings.h/.cpp
            │   ├── MF_GraphicsSettings.h/.cpp
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
| `AMF_AICharacter`      | AI-controlled player using P_EAIS behavior system      |
| `AMF_PlayerController` | Network-aware controller with character switching      |
| `AMF_AIController`     | AI controller configured for Enhanced Input bindings   |
| `UMF_InputHandler`     | P_MEIS integration component                           |
| `AMF_Ball`             | Math-based ball with possession and kick mechanics     |
| `AMF_GameMode`         | Server-only match management                           |
| `AMF_GameState`        | Replicated scores, time, and match phase               |
| `AMF_Goal`             | Goal trigger volume for detecting ball entry           |
| `AMF_Spectator`        | Spectator pawn for viewing matches before joining      |

---

## 🤖 AI System (P_EAIS Integration)

The plugin integrates with **P_EAIS** (Enhanced AI System) for JSON-programmable AI opponents.

### AI Architecture

```
P_EAIS (JSON Behavior) → AMF_AICharacter → P_MEIS (Input Injection) → Gameplay
                              ↓
                    Blackboard ←→ Game State Sync
```

### Key Features

- **JSON-Defined Behaviors**: AI personalities defined in `Content/AIProfiles/` (Striker, Defender, Midfielder, Goalkeeper)
- **Input Injection**: AI uses P_MEIS to "press buttons" like human players (fair AI)
- **Blackboard Sync**: Game state (ball possession, team, position) auto-synced to AI blackboard
- **AmIClosestToBall Logic**: Prevents multiple AI players from chasing the ball simultaneously
- **Role-Based Positioning**: Support positions calculated dynamically based on player role
- **Profile Switching**: Change AI behavior at runtime via console or Blueprint

### Using AI Characters

#### 1. Spawn via Console

```cpp
EAIS.SpawnBot 1 Striker    // Spawn Striker on Team 1
EAIS.SpawnBot 2 Defender   // Spawn Defender on Team 2
```

#### 2. Place in Level

1. Drag `AMF_AICharacter` Blueprint into your level
2. Set `AIProfile` to "Striker", "Defender", or "Goalkeeper"
3. Set `bAutoStartAI` to true
4. AI will start behaving when match begins

#### 3. Programmatic Spawning

```cpp
// In your GameMode
AMF_AICharacter* Bot = GetWorld()->SpawnActor<AMF_AICharacter>(AICharacterClass, SpawnLocation, SpawnRotation);
Bot->AIProfile = TEXT("Striker");
Bot->SetTeamID(EMF_TeamID::TeamA);
Bot->StartAI();
```

### AI Profiles

| Profile      | Description                                                 |
| ------------ | ----------------------------------------------------------- |
| `Striker`    | Offensive AI - positions high, moves to goal, shoots        |
| `Midfielder` | Support AI - balances between attack and defense            |
| `Defender`   | Defensive AI - marks opponents, clears ball, covers goal    |
| `Goalkeeper` | Goal protection - stays near goal, blocks shots, coming out |

### Debugging AI

```cpp
EAIS.Debug 1                    // Enable debug visualization
EAIS.InjectEvent * BallSeen     // Inject event to all AI
EAIS.ListActions                // List registered actions
```

### Creating Custom AI Profiles

1. Create JSON file in `Content/AIProfiles/`
2. Define states, transitions, and actions
3. Reference in `AMF_AICharacter::AIProfile`

See **P_EAIS GUIDE.md** for full JSON schema, action reference, and step-by-step authoring guide.

### AI Possession Handoff (Human ↔ AI)

All characters spawned in match are `AMF_AICharacter` instances, enabling seamless transition between human and AI control.

#### How It Works

```
Match Start → All characters spawn as AI → AI controllers auto-assigned
                    ↓
Human Joins Team → Possesses character → AI stops (PossessedBy)
                    ↓
Human Switches (Q) → Unpossesses old → AI resumes (UnPossessed) → Possesses new → AI stops
                    ↓
Human Leaves → Unpossesses → AI resumes (SpawnDefaultController + StartAI)
```

#### Key Implementation Points

| Event             | What Happens                                                         |
| ----------------- | -------------------------------------------------------------------- |
| Character Spawned | `AutoPossessAI = PlacedInWorldOrSpawned` → AI controller auto-spawns |
| Human Possesses   | `PossessedBy()` detects `PlayerController` → `StopAI()`              |
| Human Unpossesses | `UnPossessed()` → `SpawnDefaultController()` → `StartAI()`           |

#### Configuration (MF_AICharacter Constructor)

```cpp
// AI Controller is auto-spawned when no human controls this character
AIControllerClass = AMF_AIController::StaticClass();
AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
```

#### Testing the Handoff

1. Launch PIE with 2 players
2. Observe: All 6 characters moving (AI controlled)
3. Player 1 joins Team A → Controls 1 character → 2 teammates remain AI
4. Player 1 presses Q → Previous character resumes AI → New character controlled
5. Player 1 leaves team → Character immediately resumes AI

## 🎮 Input System Architecture

The plugin uses a **delegate-based input architecture** with strict separation between input handling and gameplay execution.

### Flow Overview

```
Hardware Input
 └─ Enhanced Input (via P_MEIS)
     └─ UMF_InputHandler (Component on Character)
         ├─ Broadcast: OnMoveInput
         ├─ Broadcast: OnActionPressed
         ├─ Broadcast: OnActionReleased
         └─ Broadcast: OnSprintInput
             ↓
     AMF_PlayerCharacter (Subscriber)
         ├─ HandleMove() → AddMovementInput (LOCAL only)
         ├─ HandleActionPressed() → Server_RequestTackle
         └─ HandleActionReleased() → Server_RequestShoot/Pass
             ↓
     Server (Authority)
         ├─ ExecuteTackle()
         ├─ ExecuteShoot()
         └─ ExecutePass()
```

### Ownership Rules (Absolute)

| System              | Owner                       |
| ------------------- | --------------------------- |
| Enhanced Input      | P_MEIS via PlayerController |
| InputHandler        | Passive intent broadcaster  |
| Gameplay execution  | Character (Server)          |
| Ball possession     | Ball Actor (Server)         |
| Character switching | PlayerController            |

### Movement Authority (Critical)

Movement input is applied **only on the owning client**. The `CharacterMovementComponent` handles replication to the server automatically.

```cpp
// MF_PlayerCharacter.cpp
void AMF_PlayerCharacter::UpdateMovement(float DeltaTime)
{
    // Movement input MUST be applied only on the owning client
    if (!IsLocallyControlled())
        return;

    AddMovementInput(GetActorForwardVector(), Input.Y);
    AddMovementInput(GetActorRightVector(),   Input.X);
}
```

**Rules:**

- ✅ Movement input executes **only on owning client**
- ❌ Server never calls `AddMovementInput`
- ❌ Simulated proxies never inject input
- ✅ CharacterMovementComponent handles replication
- ✅ Prediction remains intact

### Action Execution (Server-Authoritative)

All gameplay actions are **server-authoritative**. Clients request actions via Server RPCs.

```cpp
// Client requests
void AMF_PlayerCharacter::OnActionPressed(bool bPressed)
{
    if (!bHasBall && IsLocallyControlled())
    {
        Server_RequestTackle();  // RPC to server
    }
}

// Server executes
void AMF_PlayerCharacter::Server_RequestTackle_Implementation()
{
    ExecuteTackle();  // Server-only execution
}
```

### Delegate Subscription

The Character subscribes to InputHandler delegates in `SetupInputBindings()`:

```cpp
void AMF_PlayerCharacter::SetupInputBindings()
{
    // Bind to P_MEIS input events
    InputHandler->OnMoveInput.AddDynamic(this, &AMF_PlayerCharacter::OnMoveInputReceived);
    InputHandler->OnSprintInput.AddDynamic(this, &AMF_PlayerCharacter::OnSprintInputReceived);
    InputHandler->OnActionPressed.AddDynamic(this, &AMF_PlayerCharacter::OnActionPressed);
    InputHandler->OnActionReleased.AddDynamic(this, &AMF_PlayerCharacter::OnActionReleased);
}
```

---

## ⚽ Ball Possession System

The ball uses a **server-authoritative possession model** with replicated state for clients.

### Single Source of Truth

```cpp
// AMF_Ball is the authority for possession
AMF_Ball::CurrentPossessor    // Who has the ball (replicated)
AMF_Ball::AssignPossession()  // Authoritative assignment
AMF_Ball::ClearPossession()   // Authoritative release
```

`bHasBall` on Character is **derived state**, not authority.

### Invariant (Must Always Hold)

```
Character->bHasBall == (Character->CurrentBall != nullptr)
```

### Ball Discovery (No World Scanning)

Characters **never scan the world** for the ball. Instead:

```cpp
// MF_PlayerCharacter.h
UPROPERTY(ReplicatedUsing = OnRep_CurrentBall)
AMF_Ball* CurrentBall;

// Ball assigns itself:
void AMF_Ball::AssignPossession(AMF_PlayerCharacter* NewOwner)
{
    NewOwner->CurrentBall = this;  // Ball sets the reference
}
```

### Client-Side Attachment

Ball attachment replicates to clients via `OnRep_Possessor`:

```cpp
void AMF_Ball::OnRep_Possessor()
{
    if (CurrentPossessor)
    {
        AttachToComponent(
            CurrentPossessor->GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            TEXT("BallSocket")
        );
    }
    else
    {
        DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    }
}
```

### Auto-Pickup Eligibility

Ball auto-pickup is only allowed when:

1. Ball has **no PossessingPlayer**
2. Ball velocity is below threshold
3. Character can receive ball (`CanReceiveBall()`)

```cpp
bool AMF_Ball::CanAutoPickup(const AMF_PlayerCharacter* Character) const
{
    return
        !CurrentPossessor &&
        Velocity.SizeSquared() < AutoPickupVelocityThreshold &&
        Character &&
        Character->CanReceiveBall();
}
```

---

## 🎯 Tackle / Shoot / Pass System

### Action Matrix

| Has Ball | Event   | Action       |
| -------- | ------- | ------------ |
| ❌       | Press   | Tackle       |
| ✅       | Release | Shoot / Pass |

### Action Consumption (Prevents Accidental Shoot After Tackle)

When a player tackles and immediately gains the ball, releasing the action button should **not** shoot. This is handled via an action consumption flag:

```cpp
// MF_PlayerCharacter.h
bool bActionConsumedByTackle = false;

// On Press (without ball)
bActionConsumedByTackle = true;  // Mark consumed
Server_RequestTackle();

// On Release
if (bActionConsumedByTackle) {
    bActionConsumedByTackle = false;  // Reset
    return;  // Skip shoot
}
// Shoot only if action was not consumed
```

### Tackle (Server-Authoritative, Distance-Based)

Tackle uses distance-based detection (not overlap) with team filtering:

```cpp
void AMF_PlayerCharacter::ExecuteTackle()
{
    const float TackleRange = MF_Constants::TackleRange;  // 200cm

    for (TActorIterator<AMF_PlayerCharacter> It(GetWorld()); It; ++It)
    {
        AMF_PlayerCharacter* Other = *It;
        if (!Other || Other == this)
            continue;

        // Skip teammates (None team can tackle anyone)
        if (TeamID != EMF_TeamID::None && Other->GetTeamID() == GetTeamID())
            continue;

        float Distance = FVector::Dist(GetActorLocation(), Other->GetActorLocation());
        if (Distance <= TackleRange && Other->HasBall() && Other->CurrentBall)
        {
            Other->CurrentBall->SetPossessor(this);
            return;
        }
    }
}
```

### Shoot/Pass

```cpp
void AMF_PlayerCharacter::ExecuteShoot()
{
    check(HasAuthority());
    check(bHasBall && CurrentBall);

    CurrentBall->Kick(GetActorForwardVector(), ShootPower, bAddHeight);
}
```

### Input Bindings (Default)

| Action        | Keyboard | Gamepad       | Trigger Event       |
| ------------- | -------- | ------------- | ------------------- |
| Move          | WASD     | Left Stick    | Triggered           |
| Action        | Space    | A/X           | Started/Completed   |
| Sprint        | Shift    | Right Trigger | Triggered           |
| Switch Player | Q        | Left Shoulder | Completed (Release) |
| Pause         | P        | Start         | Completed (Release) |

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

UPROPERTY(BlueprintAssignable)
FOnPlayerRoleChanged OnPlayerRoleChanged;
// Params: AMF_PlayerController* Controller, bool bIsPlaying

UPROPERTY(BlueprintAssignable)
FOnPossessedPawnChangedDelegate OnMFPossessedPawnChanged;
// Params: AMF_PlayerController* Controller, APawn* NewPawn
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

The plugin includes a complete C++ UI widget system for HUD, spectator controls, team selection, and gameplay controls.

All widgets are designed as C++ `UUserWidget`-derived base classes that can be extended in Blueprint.

**Each widget class is self-describing** — it contains embedded JSON returned by `static FString GetWidgetSpec()`.
That JSON is consumed by **P_MWCS** to deterministically generate/repair/validate the matching Widget Blueprint assets (`WBP_MF_*`).

Read the full workflow + binding reference in **UI_WIDGETS.md**.

### Recommended (CI-safe) generation workflow

Run these commandlets headlessly (example paths):

```bat
Engine\Binaries\Win64\UnrealEditor-Cmd.exe "D:\Projects\UE\A_MiniFootball\A_MiniFootball.uproject" -run=MWCS_CreateWidgets -Mode=ForceRecreate -FailOnErrors -FailOnWarnings -unattended -nop4 -NullRHI -stdout -FullStdOutLogOutput

Engine\Binaries\Win64\UnrealEditor-Cmd.exe "D:\Projects\UE\A_MiniFootball\A_MiniFootball.uproject" -run=MWCS_ValidateWidgets -FailOnErrors -FailOnWarnings -unattended -nop4 -NullRHI -stdout -FullStdOutLogOutput
```

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
│           └── UMF_ToggleActionButton
├── UMF_TeamSelectionPopup (Modal)
│   ├── UMF_TeamPanel (Team A)
│   └── UMF_TeamPanel (Team B)
└── UMF_PauseMenu (Modal)
```

### Widget Classes Reference

| Widget Class             | File                           | Description                                     |
| ------------------------ | ------------------------------ | ----------------------------------------------- |
| `UMF_HUD`                | `MF_HUD.h/.cpp`                | Main HUD container with widget switcher         |
| `UMF_MatchInfo`          | `MF_MatchInfo.h/.cpp`          | Score and time display                          |
| `UMF_TeamIndicator`      | `MF_TeamIndicator.h/.cpp`      | Current team indicator                          |
| `UMF_TransitionOverlay`  | `MF_TransitionOverlay.h/.cpp`  | Loading/transition screen                       |
| `UMF_SpectatorControls`  | `MF_SpectatorControls.h/.cpp`  | Spectator mode UI                               |
| `UMF_GameplayControls`   | `MF_GameplayControls.h/.cpp`   | Mobile touch controls container                 |
| `UMF_VirtualJoystick`    | `MF_VirtualJoystick.h/.cpp`    | Touch joystick for movement                     |
| `UMF_ActionButton`       | `MF_ActionButton.h/.cpp`       | Context-sensitive action button                 |
| `UMF_ToggleActionButton` | `MF_ToggleActionButton.h/.cpp` | Generic hold/toggle action button (e.g. Sprint) |
| `UMF_TeamSelectionPopup` | `MF_TeamSelectionPopup.h/.cpp` | Modal team selection dialog                     |
| `UMF_TeamPanel`          | `MF_TeamPanel.h/.cpp`          | Reusable team info panel                        |
| `UMF_QuickTeamPanel`     | `MF_QuickTeamPanel.h/.cpp`     | Compact team preview with quick join            |
| `UMF_PauseMenu`          | `MF_PauseMenu.h/.cpp`          | In-game pause menu                              |

> **📖 Full UI Documentation:** See [UI_WIDGETS.md](./UI_WIDGETS.md) for complete widget binding reference, visual design specs, and step-by-step creation guide.

---

## 🧬 Self-Describing Widget System (JSON Specifications)

Each `UMF_*` widget class contains an embedded **JSON specification** accessible via `static FString GetWidgetSpec()`.

That JSON is the single source of truth used by **P_MWCS** to deterministically generate/repair/validate the corresponding Widget Blueprint assets (`WBP_MF_*`).

> Note: `GetWidgetSpec()` is exposed via `UFUNCTION` and should return **by value** (`FString`), not `const FString&`, for reflection compatibility.

### Key benefits

| Feature                    | Description                                                            |
| -------------------------- | ---------------------------------------------------------------------- |
| **Single source of truth** | Structure + bindings live with the owning C++ widget class             |
| **Deterministic builds**   | MWCS rebuilds the `WidgetTree` in a stable way (ideal for CI)          |
| **No Python**              | Automation and validation are implemented in C++ (editor-only tooling) |
| **Strict CI support**      | Commandlets can fail on warnings (`-FailOnWarnings`)                   |
| **Nested widgets**         | Specs support `Type: "UserWidget"` with `WidgetClass` for composition  |

### JSON specification contents (high level)

Each widget spec typically includes:

- **BlueprintName** — target WBP name (example: `WBP_MF_ActionButton`)
- **ParentClass** — full path to the C++ parent class
- **DesignerPreview** — preview sizing metadata (replaces legacy `DesignerToolbar`)
- **Hierarchy** — widget tree definition
- **Bindings** — required vs optional BindWidget names (name + expected type)

### Example (trimmed)

```json
{
  "BlueprintName": "WBP_MF_ActionButton",
  "ParentClass": "/Script/P_MiniFootball.MF_ActionButton",
  "DesignerPreview": { "SizeMode": "Desired" },
  "Bindings": {
    "Required": { "ActionButton": "Button" },
    "Optional": { "ActionIcon": "Image", "ActionText": "TextBlock" }
  },
  "Hierarchy": {
    "Type": "CanvasPanel",
    "Name": "Root",
    "Children": [
      { "Type": "Button", "Name": "ActionButton" },
      { "Type": "Image", "Name": "ActionIcon" },
      { "Type": "TextBlock", "Name": "ActionText" }
    ]
  }
}
```

### Generating / validating the Widget Blueprints

Use MWCS commandlets (recommended for CI):

```bat
Engine\Binaries\Win64\UnrealEditor-Cmd.exe "D:\Projects\UE\A_MiniFootball\A_MiniFootball.uproject" -run=MWCS_CreateWidgets -Mode=ForceRecreate -FailOnErrors -FailOnWarnings -unattended -nop4 -NullRHI -stdout -FullStdOutLogOutput

Engine\Binaries\Win64\UnrealEditor-Cmd.exe "D:\Projects\UE\A_MiniFootball\A_MiniFootball.uproject" -run=MWCS_ValidateWidgets -FailOnErrors -FailOnWarnings -unattended -nop4 -NullRHI -stdout -FullStdOutLogOutput
```

---

## 📝 Version History

| Version | Date       | Changes                                                                                           |
| ------- | ---------- | ------------------------------------------------------------------------------------------------- |
| 1.0     | 07/12/2025 | Initial plugin implementation (Phases 1-6)                                                        |
| 1.1     | 11/12/2025 | Added self-describing JSON widget specifications (Phase 10)                                       |
| 1.2     | 11/12/2025 | Added Editor Utility Widget guide for WBP Creator tool                                            |
| 1.3     | 12/12/2025 | Added diagnostic functions and only_euw mode for WBP Creator                                      |
| 1.4     | 12/12/2025 | EUW is shell-only (skip EUW validation/tree/diagnostics)                                          |
| 1.5     | 25/12/2025 | PLAN-VERIFIED.md implementation: Core Control Model, Input Contract, UI Authority, GameState APIs |

---

## 📄 License

This plugin is part of the A_MiniFootball project.
