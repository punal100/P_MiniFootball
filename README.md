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

---

## 📋 Implementation Status

| Phase   | Description                         | Status         |
| ------- | ----------------------------------- | -------------- |
| Phase 1 | Core Plugin Setup                   | ✅ COMPLETE    |
| Phase 2 | Player Controller & Movement (+Net) | ✅ COMPLETE    |
| Phase 3 | Ball System & Possession (+Net)     | ✅ COMPLETE    |
| Phase 4 | Shooting & Passing (+Net)           | ✅ COMPLETE    |
| Phase 5 | Goal & Scoring System (+Net)        | ✅ COMPLETE    |
| Phase 6 | Match Flow & Game Modes (+Net)      | ✅ COMPLETE    |
| Phase 7 | Simple AI (Optional)                | ⏳ DEFERRED    |
| Phase 8 | Polish & Mobile Optimization        | ❌ NOT STARTED |

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
├── Resources/
│   └── Icon128.png            # Plugin icon
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
            ├── Player/
            │   ├── MF_InputHandler.h/.cpp      # P_MEIS integration
            │   ├── MF_PlayerCharacter.h/.cpp   # Replicated player (top-down camera)
            │   └── MF_PlayerController.h/.cpp  # Network controller
            ├── Ball/
            │   └── MF_Ball.h/.cpp     # Math-based ball physics
            ├── Match/
            │   ├── MF_GameMode.h/.cpp    # Server-only game mode
            │   ├── MF_GameState.h/.cpp   # Replicated match state
            │   └── MF_Goal.h/.cpp        # Goal trigger volume
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

---

## 🌐 Network Architecture

- **Server Authoritative**: All game logic validated on server
- **Client RPCs**: `Server_RequestShoot`, `Server_RequestPass`, `Server_RequestTackle`
- **Replication**: `DOREPLIFETIME` macros with `ReplicatedUsing` for rep notifies
- **Interpolation**: Client-side ball position smoothing

---

## 📝 Version History

| Version | Date       | Changes                                    |
| ------- | ---------- | ------------------------------------------ |
| 1.0     | 07/12/2025 | Initial plugin implementation (Phases 1-6) |

---

## 📄 License

This plugin is part of the A_MiniFootball project.
