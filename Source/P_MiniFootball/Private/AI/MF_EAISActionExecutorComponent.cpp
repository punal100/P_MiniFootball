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
        AActor* TargetActor = nullptr;
        
        // Try to get actor first for smarter aiming
        IEAIS_TargetProvider::Execute_EAIS_GetTargetActor(OwnerCharacter, FName(*Params.Target), TargetActor);
        
        // If it's a Goal, aim for the opening!
        // We know Goal_Opponent is likely an AMF_Goal
        if (TargetActor && TargetActor->GetName().Contains(TEXT("Goal")))
        {
             TargetLocation = TargetActor->GetActorLocation();
             // Goal usually faces X or Y. In our case, Goals are at Y ends.
             // We generally want to aim for the center of the goal line, maybe with slight offset?
             // Assuming Goal Actor Location IS the center of the goal line.
             
             // Add slight noise to prevent robotic precision (Horizontal spread)
             float Noise = FMath::RandRange(-200.0f, 200.0f);
             if (FMath::Abs(TargetLocation.Y) > 5000.0f) // Goal is along Y axis
             {
                 TargetLocation.X += Noise;
             }
        }
        else if (IEAIS_TargetProvider::Execute_EAIS_GetTargetLocation(OwnerCharacter, FName(*Params.Target), TargetLocation))
        {
             // Fallback to location
        }

        if (TargetLocation != FVector::ZeroVector)
        {
            // P_MEIS: Clamp target to field dimensions to prevent shooting OOB
            // Field dimensions are roughly 4000x10000 (FieldWidth x FieldLength)
            // Safety clamp: Width +/- 2500, Length +/- 6000
            TargetLocation.X = FMath::Clamp(TargetLocation.X, -2500.0f, 2500.0f);
            TargetLocation.Y = FMath::Clamp(TargetLocation.Y, -5500.0f, 5500.0f);

            Direction = (TargetLocation - OwnerCharacter->GetActorLocation()).GetSafeNormal();
            Result.Message = FString::Printf(TEXT("Shooting at %s"), *Params.Target);
        }
    }

    // Call server-side execution directly
    // Max Shoot Speed is 2500, so Power (0-1) * 2500
    float ShootPower = FMath::Clamp(Power * 2500.0f, 500.0f, 2500.0f);
    OwnerCharacter->Server_RequestShoot(Direction, ShootPower); 

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

    bool bTargetFound = false;
    float PassSpeed = MF_Constants::BallPassSpeed * Power;
    // [New] Target Support
    if (!Params.Target.IsEmpty())
    {
        FVector TargetLocation = FVector::ZeroVector;
        AActor* TargetActor = nullptr;

        // Prefer actor for better aiming/leading (and to avoid passing "to nobody")
        if (IEAIS_TargetProvider::Execute_EAIS_GetTargetActor(OwnerCharacter, FName(*Params.Target), TargetActor) && TargetActor)
        {
            TargetLocation = TargetActor->GetActorLocation();
            bTargetFound = true;
        }
        else if (IEAIS_TargetProvider::Execute_EAIS_GetTargetLocation(OwnerCharacter, FName(*Params.Target), TargetLocation))
        {
            bTargetFound = true;
        }

        if (bTargetFound && TargetLocation != FVector::ZeroVector)
        {
            const FVector MyLoc = OwnerCharacter->GetActorLocation();

            // Lead slightly if the target is moving (helps reduce "pass behind" interceptions)
            FVector TargetVel2D = FVector::ZeroVector;
            if (TargetActor)
            {
                TargetVel2D = TargetActor->GetVelocity();
                TargetVel2D.Z = 0.0f;
            }

            const float Dist2D = FVector::Dist2D(MyLoc, TargetLocation);
            const float BaseSpeed = FMath::Clamp(Dist2D / 0.9f, 600.0f, MF_Constants::BallPassSpeed);
            PassSpeed = FMath::Clamp(BaseSpeed * FMath::Clamp(Power, 0.35f, 1.0f), 600.0f, MF_Constants::BallPassSpeed);

            const float LeadTime = FMath::Clamp(Dist2D / FMath::Max(PassSpeed, 1.0f), 0.12f, 0.30f);
            FVector AimPoint = TargetLocation + (TargetVel2D * LeadTime);

            // Clamp aim point to field dimensions
            AimPoint.X = FMath::Clamp(AimPoint.X, -3200.0f, 3200.0f);
            AimPoint.Y = FMath::Clamp(AimPoint.Y, -5250.0f, 5250.0f);
            AimPoint.Z = MyLoc.Z;

            Direction = (AimPoint - MyLoc).GetSafeNormal();
            Result.Message = FString::Printf(TEXT("Passing to %s"), *Params.Target);
        }
        else
        {
            bTargetFound = false;
        }
    }

    // fallback: if no target or target not found, and we are facing out of bounds, aim towards center
    if (!bTargetFound)
    {
        FVector MyLoc = OwnerCharacter->GetActorLocation();
        // If facing towards sideline (>3000 or <-3000)
        bool bFacingSideline = (MyLoc.X > 2500.0f && Direction.X > 0.0f) || (MyLoc.X < -2500.0f && Direction.X < 0.0f);
        if (bFacingSideline)
        {
            // Aim towards opponent goal center or at least field center
            float AttackDir = (OwnerCharacter->GetTeamID() == EMF_TeamID::TeamA) ? -1.0f : 1.0f;
            FVector SafeTarget(0.0f, 5000.0f * AttackDir, 0.0f);
            Direction = (SafeTarget - MyLoc).GetSafeNormal();
            Result.Message = TEXT("Passing towards center (safe fallback)");
        }
    }

    OwnerCharacter->Server_RequestPass(Direction, PassSpeed);

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
