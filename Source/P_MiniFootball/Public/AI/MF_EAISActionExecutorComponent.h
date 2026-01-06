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
    virtual FEAIS_ActionResult EAIS_ExecuteAction_Implementation(const FName ActionId, const FString& ParamsJson) override;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    AMF_PlayerCharacter* OwnerCharacter;

    // --- Action Handlers ---
    FEAIS_ActionResult HandleShoot(const FString& ParamsJson);
    FEAIS_ActionResult HandlePass(const FString& ParamsJson);
    FEAIS_ActionResult HandleTackle(const FString& ParamsJson);
    FEAIS_ActionResult HandleSprint(const FString& ParamsJson);
    FEAIS_ActionResult HandleFace(const FString& ParamsJson);
    FEAIS_ActionResult HandleMark(const FString& ParamsJson);
};
