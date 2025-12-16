/*
 * @Author: Punal Manalan
 * @Description: MF_AIController - AI controller configured for Enhanced Input bindings
 * @Date: 15/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MF_AIController.generated.h"

/**
 * AI Controller that uses UEnhancedInputComponent as its InputComponent class.
 * This enables P_MEIS to bind actions to the controller input component.
 */
UCLASS()
class P_MINIFOOTBALL_API AMF_AIController : public AAIController
{
    GENERATED_BODY()

public:
    AMF_AIController();

protected:
    virtual void CreateInputComponent(TSubclassOf<UInputComponent> InputComponentToCreate) override;
};
