/*
 * @Author: Punal Manalan
 * @Description: MF_AIController - Implementation
 * @Date: 15/12/2025
 */

#include "AI/MF_AIController.h"

#include "EnhancedInputComponent.h"

AMF_AIController::AMF_AIController()
{
}

void AMF_AIController::CreateInputComponent(TSubclassOf<UInputComponent> InputComponentToCreate)
{
    Super::CreateInputComponent(UEnhancedInputComponent::StaticClass());
}
