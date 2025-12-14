/*
 * @Author: Punal Manalan
 * @Description: MF_MenuGameMode - UI-only GameMode for Entry/MainMenu maps
 * @Date: 14/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MF_MenuGameMode.generated.h"

/**
 * Menu-only GameMode.
 * No pawn spawning; uses AMF_MenuPlayerController.
 */
UCLASS()
class P_MINIFOOTBALL_API AMF_MenuGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMF_MenuGameMode();
};
