/*
 * @Author: Punal Manalan
 * @Description: MF_MenuGameMode - UI-only GameMode for Entry/MainMenu maps
 * @Date: 14/12/2025
 */

#include "MF_MenuGameMode.h"

#include "Player/MF_MenuPlayerController.h"

AMF_MenuGameMode::AMF_MenuGameMode()
{
    DefaultPawnClass = nullptr;
    PlayerControllerClass = AMF_MenuPlayerController::StaticClass();

    // Menu maps are UI-only; avoid spawning HUD/pawns by default.
    HUDClass = nullptr;
}
