// Copyright Punal Manalan. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EAIS_ActionExecutor.h"
#include "MF_EAISActionExecutorComponent.generated.h"

class AMF_PlayerCharacter;

/**
 * Executes AI actions for Mini Football characters.
 * Bridges EAIS generic actions to MF specific logic.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class P_MINIFOOTBALL_API UMF_EAISActionExecutorComponent : public UActorComponent, public IEAIS_ActionExecutor
{
    GENERATED_BODY()

public:
    UMF_EAISActionExecutorComponent();

    /** IEAIS_ActionExecutor Implementation */
    /** IEAIS_ActionExecutor Implementation */
    virtual FEAIS_ActionResult EAIS_ExecuteAction_Implementation(const FName ActionId, const FAIActionParams& Params) override;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    AMF_PlayerCharacter* OwnerCharacter;

    // --- Action Handlers ---
    FEAIS_ActionResult HandleShoot(const FAIActionParams& Params);
    FEAIS_ActionResult HandlePass(const FAIActionParams& Params);
    FEAIS_ActionResult HandleTackle(const FAIActionParams& Params);
    FEAIS_ActionResult HandleSprint(const FAIActionParams& Params);
    FEAIS_ActionResult HandleFace(const FAIActionParams& Params);
    FEAIS_ActionResult HandleMark(const FAIActionParams& Params);

    // --- Movement / targeting handlers ---
    FEAIS_ActionResult HandleMoveTo(const FAIActionParams& Params);
    FEAIS_ActionResult HandleSelectPassTarget(const FAIActionParams& Params);

    // --- Goalkeeper / utility handlers (AIProfile-driven; safe no-ops if not GK) ---
    FEAIS_ActionResult HandleEvaluateShot(const FAIActionParams& Params);
    FEAIS_ActionResult HandlePerformDive(const FAIActionParams& Params);
    FEAIS_ActionResult HandleSetCooldown(const FAIActionParams& Params);
    FEAIS_ActionResult HandleClearTarget(const FAIActionParams& Params);
};
