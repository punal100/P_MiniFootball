/*
 * @Author: Punal Manalan
 * @Description: MF_AICharacter - Deprecated. Use AMF_PlayerCharacter directly.
 *               Kept for backward compatibility with existing Blueprints.
 * @Date: 29/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "../Player/MF_PlayerCharacter.h"
#include "MF_AICharacter.generated.h"

/**
 * DEPRECATED: AI functionality has been moved to AMF_PlayerCharacter to support hybrid AI/Human control.
 * This class remains only to prevent breaking existing Blueprint references.
 */
UCLASS()
class P_MINIFOOTBALL_API AMF_AICharacter : public AMF_PlayerCharacter
{
    GENERATED_BODY()

public:
    AMF_AICharacter();
};
