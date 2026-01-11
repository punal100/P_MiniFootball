// Copyright Punal Manalan. All Rights Reserved.

#include "AI/MF_EAISActionExecutorComponent.h"
#include "Player/MF_PlayerCharacter.h"

UMF_EAISActionExecutorComponent::UMF_EAISActionExecutorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UMF_EAISActionExecutorComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerCharacter = Cast<AMF_PlayerCharacter>(GetOwner());
}

FEAIS_ActionResult UMF_EAISActionExecutorComponent::EAIS_ExecuteAction_Implementation(const FName ActionId, const FAIActionParams& Params)
{
    if (!OwnerCharacter) 
    {
        FEAIS_ActionResult Result;
        Result.bSuccess = false;
        Result.Message = TEXT("No owner character");
        return Result;
    }

    // [ANTI-RUBBERBAND] Critical Guard: Do NOT execute AI actions if controlled by a Human Player
    // This prevents the AI system from fighting with client-side prediction inputs.
    if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
    {
        FEAIS_ActionResult Result;
        Result.bSuccess = false;
        Result.Message = TEXT("Action blocked: Character is controlled by Human Player");
        return Result;
    }

    if (ActionId == "MF.Shoot") return HandleShoot(Params);
    if (ActionId == "MF.Pass") return HandlePass(Params);
    if (ActionId == "MF.Tackle") return HandleTackle(Params);
    if (ActionId == "MF.Sprint") return HandleSprint(Params);
    if (ActionId == "MF.Face") return HandleFace(Params);
    if (ActionId == "MF.Mark") return HandleMark(Params);

    FEAIS_ActionResult Result;
    Result.bSuccess = false;
    Result.Message = FString::Printf(TEXT("Unknown action: %s"), *ActionId.ToString());
    return Result;
}

FEAIS_ActionResult UMF_EAISActionExecutorComponent::HandleShoot(const FAIActionParams& Params)
{
    FEAIS_ActionResult Result;

    // Validation: Must have ball to shoot
    if (!OwnerCharacter || !OwnerCharacter->HasBall())
    {
        Result.bSuccess = false;
        Result.Message = TEXT("Cannot shoot: Do not possess ball");
        return Result;
    }
    
    // Parse direction and power
    FVector Direction = OwnerCharacter->GetActorForwardVector();
    float Power = 1.0f; // Multiplier of shoot speed

    if (Params.Power > 0.0f)
    {
        Power = Params.Power;
    }

    // [New] Target Support
    if (!Params.Target.IsEmpty())
    {
        FVector TargetLocation;
        if (IEAIS_TargetProvider::Execute_EAIS_GetTargetLocation(OwnerCharacter, FName(*Params.Target), TargetLocation))
        {
            Direction = (TargetLocation - OwnerCharacter->GetActorLocation()).GetSafeNormal();
            Result.Message = FString::Printf(TEXT("Shooting at %s"), *Params.Target);
        }
    }

    // Call server-side execution directly
    OwnerCharacter->Server_RequestShoot(Direction, Power * 2000.0f); // Proxy call to handle state/physics

    Result.bSuccess = true;
    return Result;
}

FEAIS_ActionResult UMF_EAISActionExecutorComponent::HandlePass(const FAIActionParams& Params)
{
    FEAIS_ActionResult Result;

    // Validation: Must have ball to pass
    if (!OwnerCharacter || !OwnerCharacter->HasBall())
    {
        Result.bSuccess = false;
        Result.Message = TEXT("Cannot pass: Do not possess ball");
        return Result;
    }

    FVector Direction = OwnerCharacter->GetActorForwardVector();
    float Power = 0.5f;

    if (Params.Power > 0.0f)
    {
        Power = Params.Power;
    }

    // [New] Target Support
    if (!Params.Target.IsEmpty())
    {
        FVector TargetLocation;
        if (IEAIS_TargetProvider::Execute_EAIS_GetTargetLocation(OwnerCharacter, FName(*Params.Target), TargetLocation))
        {
            Direction = (TargetLocation - OwnerCharacter->GetActorLocation()).GetSafeNormal();
            Result.Message = FString::Printf(TEXT("Passing to %s"), *Params.Target);
        }
    }

    OwnerCharacter->Server_RequestPass(Direction, Power * 1500.0f);

    Result.bSuccess = true;
    return Result;
}

FEAIS_ActionResult UMF_EAISActionExecutorComponent::HandleTackle(const FAIActionParams& Params)
{
    FEAIS_ActionResult Result;
    OwnerCharacter->Server_RequestTackle();
    Result.bSuccess = true;
    return Result;
}

FEAIS_ActionResult UMF_EAISActionExecutorComponent::HandleSprint(const FAIActionParams& Params)
{
    FEAIS_ActionResult Result;
    
    bool bActive = true; // Default to sprint on

    // Check ExtraParams for "active" flag if available
    if (Params.ExtraParams.Contains(TEXT("active")))
    {
        FString ActiveStr = Params.ExtraParams[TEXT("active")];
        bActive = ActiveStr.Equals(TEXT("true"), ESearchCase::IgnoreCase);
    }
    
    // Also support checking the boolean param if we add it to struct later
    
    OwnerCharacter->SetSprinting(bActive);
    Result.bSuccess = true;
    Result.Message = bActive ? TEXT("Sprint ON") : TEXT("Sprint OFF");
    return Result;
}

FEAIS_ActionResult UMF_EAISActionExecutorComponent::HandleFace(const FAIActionParams& Params)
{
    FEAIS_ActionResult Result;
    
    FString TargetName = TEXT("Ball"); // Default target
    
    if (!Params.Target.IsEmpty())
    {
        TargetName = Params.Target;
    }
    
    // Resolve target location via TargetProvider interface
    FVector TargetLocation;
    if (IEAIS_TargetProvider::Execute_EAIS_GetTargetLocation(OwnerCharacter, FName(*TargetName), TargetLocation))
    {
        const FVector Direction = (TargetLocation - OwnerCharacter->GetActorLocation()).GetSafeNormal2D();
        if (!Direction.IsNearlyZero())
        {
            const FRotator NewRotation = Direction.Rotation();
            OwnerCharacter->SetActorRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));
            Result.bSuccess = true;
            Result.Message = FString::Printf(TEXT("Facing %s"), *TargetName);
        }
        else
        {
            Result.bSuccess = false;
            Result.Message = TEXT("Already at target location");
        }
    }
    else
    {
        Result.bSuccess = false;
        Result.Message = FString::Printf(TEXT("Target not found: %s"), *TargetName);
    }
    
    return Result;
}

FEAIS_ActionResult UMF_EAISActionExecutorComponent::HandleMark(const FAIActionParams& Params)
{
    FEAIS_ActionResult Result;
    
    // Mark means follow the nearest opponent tightly
    // We'll get the nearest opponent's location and move towards them
    FVector TargetLocation;
    if (IEAIS_TargetProvider::Execute_EAIS_GetTargetLocation(OwnerCharacter, FName("NearestOpponent"), TargetLocation))
    {
        // Calculate offset position (don't stand on top of them, stay 100 units away)
        const FVector ToOpponent = TargetLocation - OwnerCharacter->GetActorLocation();
        const float Distance = ToOpponent.Size();
        
        if (Distance > 100.0f)
        {
            // Use AI movement component to move there
            if (AController* Controller = OwnerCharacter->GetController())
            {
                OwnerCharacter->AddMovementInput(ToOpponent.GetSafeNormal(), 1.0f);
            }
            
            Result.bSuccess = true;
            Result.Message = TEXT("Marking opponent");
        }
        else
        {
            Result.bSuccess = true;
            Result.Message = TEXT("Already marking");
        }
    }
    else
    {
        Result.bSuccess = false;
        Result.Message = TEXT("No opponent to mark");
    }
    
    return Result;
}
