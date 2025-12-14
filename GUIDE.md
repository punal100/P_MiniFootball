# P_MiniFootball Guide

Quick checklist for running the Mini Football gameplay plugin inside the `A_MiniFootball` project.

For implementation details, see [README.md](./README.md).

## 1) Build + open

- Open `A_MiniFootball.uproject` in Unreal Engine 5.5
- Build **Development Editor**

## 2) Play (PIE)

- Open `/P_MiniFootball/Maps/L_MiniFootball`
- Play In Editor

## 3) Multiplayer (fastest)

- In Editor: set “Number of Players” to 2+
- Run PIE

## 4) Dedicated server correctness

Dedicated server needs to start on the gameplay map.

- Confirm `ServerDefaultMap=/P_MiniFootball/Maps/L_MiniFootball` in `Config/DefaultEngine.ini`

## 5) Input pipeline (unified)

- Gameplay input is handled via **P_MEIS**.
- Mobile UI (virtual joystick/buttons) injects into P_MEIS so keyboard/gamepad and UI share the same gameplay path.

## 6) Settings → Input

The Pause menu opens the Settings menu.

In this project, MWCS generates the Settings widget as:

- `/Game/UI/Widgets/WBP_MF_SettingsMenu`

If you change widget specs, regenerate the widget assets via MWCS (see the top-level project docs or P_MWCS docs).
