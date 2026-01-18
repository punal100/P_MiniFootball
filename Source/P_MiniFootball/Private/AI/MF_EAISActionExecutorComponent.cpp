// Copyright Punal Manalan. All Rights Reserved.

#include "AI/MF_EAISActionExecutorComponent.h"
#include "Player/MF_PlayerCharacter.h"
#include "AIComponent.h"
#include "Match/MF_Goal.h"
#include "EngineUtils.h"  // For TActorIterator

namespace
{
    static float GetExtraParamFloat(const TMap<FString, FString>& Map, const TCHAR* Key, float DefaultValue)
    {
        const FString* Found = Map.Find(Key);
        if (!Found)
        {
            return DefaultValue;
        }

        return FCString::Atof(**Found);
    }

    static FString GetExtraParamString(const TMap<FString, FString>& Map, const TCHAR* Key, const FString& DefaultValue)
    {
        const FString* Found = Map.Find(Key);
        return Found ? *Found : DefaultValue;
    }
}

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
    if (ActionId == "MF.MoveTo") return HandleMoveTo(Params);
    if (ActionId == "MF.SelectPassTarget") return HandleSelectPassTarget(Params);

    // Goalkeeper actions (safe to call for any role; they no-op if no AIComponent)
    if (ActionId == "MF.EvaluateShot") return HandleEvaluateShot(Params);
    if (ActionId == "MF.PerformDive") return HandlePerformDive(Params);
    if (ActionId == "MF.SetCooldown") return HandleSetCooldown(Params);
    if (ActionId == "MF.ClearTarget") return HandleClearTarget(Params);

    FEAIS_ActionResult Result;
    Result.bSuccess = false;
    Result.Message = FString::Printf(TEXT("Unknown action: %s"), *ActionId.ToString());
    return Result;
}

FEAIS_ActionResult UMF_EAISActionExecutorComponent::HandleEvaluateShot(const FAIActionParams& Params)
{
    FEAIS_ActionResult Result;
    Result.bSuccess = false;

    if (!OwnerCharacter)
    {
        Result.Message = TEXT("No owner character");
        return Result;
    }

    UAIComponent* AIComp = OwnerCharacter->GetAIComponent();
    if (!AIComp)
    {
        Result.Message = TEXT("No AIComponent");
        return Result;
    }

    // Defaults (JSON-tunable via blackboard overrides)
    const float ShotSpeedMin = FMath::Max(1.0f, AIComp->GetBlackboardFloat(TEXT("GK_ShotSpeedMin")));
    const float ShotAngleDotMin = FMath::Clamp(AIComp->GetBlackboardFloat(TEXT("GK_ShotAngleDotMin")), -1.0f, 1.0f);
    const float GoalHalfWidth = FMath::Max(50.0f, AIComp->GetBlackboardFloat(TEXT("GK_GoalHalfWidth")));
    const float GoalMargin = FMath::Max(0.0f, AIComp->GetBlackboardFloat(TEXT("GK_GoalMargin")));
    const float ReachRadius = FMath::Max(10.0f, AIComp->GetBlackboardFloat(TEXT("GK_ReachRadius")));
    const float MinDiveTime = FMath::Max(0.01f, AIComp->GetBlackboardFloat(TEXT("GK_MinDiveTime")));
    const float ReactionTimeBase = FMath::Max(0.0f, AIComp->GetBlackboardFloat(TEXT("GK_ReactionTimeBase")));
    const float CatchingSkill = FMath::Clamp(AIComp->GetBlackboardFloat(TEXT("GK_CatchingSkill")), 0.0f, 1.0f);

    AActor* BallActor = nullptr;
    if (!IEAIS_TargetProvider::Execute_EAIS_GetTargetActor(OwnerCharacter, FName(TEXT("Ball")), BallActor) || !BallActor)
    {
        AIComp->SetBlackboardBool(TEXT("IsShotTowardsGoal"), false);
        AIComp->SetBlackboardBool(TEXT("ShotIsWide"), false);
        AIComp->SetBlackboardBool(TEXT("IsDiveRecommended"), false);
        AIComp->SetBlackboardFloat(TEXT("TimeToImpact"), 0.0f);
        AIComp->SetBlackboardVector(TEXT("GK_ShotImpactPoint"), FVector::ZeroVector);
        Result.Message = TEXT("Ball not found");
        return Result;
    }

    AActor* GoalActor = nullptr;
    if (!IEAIS_TargetProvider::Execute_EAIS_GetTargetActor(OwnerCharacter, FName(TEXT("Goal_Self")), GoalActor) || !GoalActor)
    {
        AIComp->SetBlackboardBool(TEXT("IsShotTowardsGoal"), false);
        AIComp->SetBlackboardBool(TEXT("ShotIsWide"), false);
        AIComp->SetBlackboardBool(TEXT("IsDiveRecommended"), false);
        AIComp->SetBlackboardFloat(TEXT("TimeToImpact"), 0.0f);
        AIComp->SetBlackboardVector(TEXT("GK_ShotImpactPoint"), FVector::ZeroVector);
        Result.Message = TEXT("Goal_Self not found");
        return Result;
    }

    const FVector BallPos = BallActor->GetActorLocation();
    const FVector BallVel = BallActor->GetVelocity();
    const float BallSpeed = BallVel.Size();

    // Quick reject: not moving fast enough
    if (BallSpeed < ShotSpeedMin)
    {
        AIComp->SetBlackboardBool(TEXT("IsShotTowardsGoal"), false);
        AIComp->SetBlackboardBool(TEXT("ShotIsWide"), false);
        AIComp->SetBlackboardBool(TEXT("IsDiveRecommended"), false);
        AIComp->SetBlackboardFloat(TEXT("TimeToImpact"), 0.0f);
        AIComp->SetBlackboardVector(TEXT("GK_ShotImpactPoint"), FVector::ZeroVector);
        Result.bSuccess = true;
        Result.Message = TEXT("Ball slow; no shot");
        return Result;
    }

    const FVector GoalPos = GoalActor->GetActorLocation();
    FVector ToGoal = (GoalPos - BallPos);
    ToGoal.Z = 0.0f;
    const FVector ToGoalDir = ToGoal.IsNearlyZero() ? FVector::ZeroVector : ToGoal.GetSafeNormal();

    FVector BallVel2D = BallVel;
    BallVel2D.Z = 0.0f;
    const FVector BallDir2D = BallVel2D.IsNearlyZero() ? FVector::ZeroVector : BallVel2D.GetSafeNormal();

    const float DotToGoal = FVector::DotProduct(BallDir2D, ToGoalDir);
    const bool bHeadingToGoal = !BallDir2D.IsNearlyZero() && (DotToGoal >= ShotAngleDotMin);

    bool bShotTowardsGoal = false;
    bool bShotWide = false;
    float TimeToImpact = 0.0f;
    FVector ImpactPoint = FVector::ZeroVector;

    // Approximate goal plane as constant Y at goal location.
    // This is consistent with MiniFootball's field alignment (goals are at +/-Y).
    const float Vy = BallVel.Y;
    if (bHeadingToGoal && !FMath::IsNearlyZero(Vy, 1.0f))
    {
        const float t = (GoalPos.Y - BallPos.Y) / Vy;
        if (t > 0.0f)
        {
            TimeToImpact = t;
            ImpactPoint = BallPos + (BallVel * t);

            const float HalfWidth = GoalHalfWidth + GoalMargin;
            bShotWide = FMath::Abs(ImpactPoint.X - GoalPos.X) > HalfWidth;
            bShotTowardsGoal = !bShotWide;
        }
    }

    AIComp->SetBlackboardBool(TEXT("IsShotTowardsGoal"), bShotTowardsGoal);
    AIComp->SetBlackboardBool(TEXT("ShotIsWide"), bShotWide);
    AIComp->SetBlackboardFloat(TEXT("TimeToImpact"), TimeToImpact);
    AIComp->SetBlackboardVector(TEXT("GK_ShotImpactPoint"), ImpactPoint);

    // Dive recommendation heuristic (JSON-tunable)
    bool bDiveRecommended = false;
    const float DiveCooldownEnd = AIComp->GetBlackboardFloat(TEXT("CooldownEnd_dive"));
    const float Now = OwnerCharacter->GetWorld() ? OwnerCharacter->GetWorld()->GetTimeSeconds() : 0.0f;
    const bool bDiveCooldownActive = (DiveCooldownEnd > 0.0f) && (Now < DiveCooldownEnd);
    AIComp->SetBlackboardBool(TEXT("DiveCooldownActive"), bDiveCooldownActive);

    if (bShotTowardsGoal && !bDiveCooldownActive)
    {
        // Effective reaction penalized by low catching skill
        const float EffectiveReaction = ReactionTimeBase * (1.0f + (1.0f - CatchingSkill) * 0.35f);

        const FVector MyLoc = OwnerCharacter->GetActorLocation();
        const float LateralDist = FMath::Abs(ImpactPoint.X - MyLoc.X);
        const bool bNeedsDive = LateralDist > ReachRadius;
        const bool bEnoughTime = TimeToImpact >= FMath::Max(MinDiveTime, EffectiveReaction * 0.75f);
        bDiveRecommended = bNeedsDive && bEnoughTime;
    }

    AIComp->SetBlackboardBool(TEXT("IsDiveRecommended"), bDiveRecommended);
    AIComp->SetBlackboardBool(TEXT("ShotHandled"), !bShotTowardsGoal);
    AIComp->SetBlackboardBool(TEXT("DiveComplete"), false);

    Result.bSuccess = true;
    Result.Message = bShotTowardsGoal ? TEXT("Shot evaluated: on target") : TEXT("Shot evaluated: not a goal threat");
    return Result;
}

FEAIS_ActionResult UMF_EAISActionExecutorComponent::HandlePerformDive(const FAIActionParams& Params)
{
    FEAIS_ActionResult Result;
    Result.bSuccess = false;

    if (!OwnerCharacter)
    {
        Result.Message = TEXT("No owner character");
        return Result;
    }

    UAIComponent* AIComp = OwnerCharacter->GetAIComponent();
    if (!AIComp)
    {
        Result.Message = TEXT("No AIComponent");
        return Result;
    }

    const FVector ImpactPoint = AIComp->GetBlackboardVector(TEXT("GK_ShotImpactPoint"));
    if (ImpactPoint.IsZero())
    {
        Result.bSuccess = true;
        Result.Message = TEXT("No impact point; skipping dive");
        AIComp->SetBlackboardBool(TEXT("DiveComplete"), true);
        return Result;
    }

    const float DiveSpeed = FMath::Max(200.0f, AIComp->GetBlackboardFloat(TEXT("GK_DiveSpeed")));
    const float DiveDuration = FMath::Clamp(AIComp->GetBlackboardFloat(TEXT("GK_DiveDuration")), 0.05f, 2.0f);

    FVector ToImpact = (ImpactPoint - OwnerCharacter->GetActorLocation());
    ToImpact.Z = 0.0f;
    const FVector Dir = ToImpact.IsNearlyZero() ? OwnerCharacter->GetActorForwardVector() : ToImpact.GetSafeNormal();

    // Light-weight dive approximation: short launch towards the predicted impact point.
    // This is intentionally animation-agnostic and keeps the implementation purely in C++.
    const FVector LaunchVel = Dir * DiveSpeed;
    OwnerCharacter->LaunchCharacter(FVector(LaunchVel.X, LaunchVel.Y, 0.0f), true, true);

    // Mark dive as in-progress, then complete after a short duration.
    AIComp->SetBlackboardBool(TEXT("DiveComplete"), false);

    if (UWorld* World = OwnerCharacter->GetWorld())
    {
        FTimerHandle TimerHandle;
        World->GetTimerManager().SetTimer(
            TimerHandle,
            FTimerDelegate::CreateWeakLambda(OwnerCharacter, [AIComp]()
            {
                if (AIComp)
                {
                    AIComp->SetBlackboardBool(TEXT("DiveComplete"), true);
                }
            }),
            DiveDuration,
            false
        );
    }

    Result.bSuccess = true;
    Result.Message = TEXT("Dive initiated");
    return Result;
}

FEAIS_ActionResult UMF_EAISActionExecutorComponent::HandleSetCooldown(const FAIActionParams& Params)
{
    FEAIS_ActionResult Result;
    Result.bSuccess = false;

    if (!OwnerCharacter)
    {
        Result.Message = TEXT("No owner character");
        return Result;
    }

    UAIComponent* AIComp = OwnerCharacter->GetAIComponent();
    if (!AIComp)
    {
        Result.Message = TEXT("No AIComponent");
        return Result;
    }

    const FString Key = GetExtraParamString(Params.ExtraParams, TEXT("key"), TEXT("dive"));
    const float Seconds = FMath::Max(0.0f, GetExtraParamFloat(Params.ExtraParams, TEXT("seconds"), FMath::Max(0.0f, Params.Power)));

    const float Now = OwnerCharacter->GetWorld() ? OwnerCharacter->GetWorld()->GetTimeSeconds() : 0.0f;
    const FString EndKey = FString::Printf(TEXT("CooldownEnd_%s"), *Key);
    AIComp->SetBlackboardFloat(EndKey, Now + Seconds);

    // Convenience: also expose a bool for dive specifically (used by goalkeeper JSON)
    if (Key.Equals(TEXT("dive"), ESearchCase::IgnoreCase))
    {
        AIComp->SetBlackboardBool(TEXT("DiveCooldownActive"), Seconds > 0.0f);
    }

    Result.bSuccess = true;
    Result.Message = FString::Printf(TEXT("Cooldown set: %s = %.2fs"), *Key, Seconds);
    return Result;
}

FEAIS_ActionResult UMF_EAISActionExecutorComponent::HandleClearTarget(const FAIActionParams& Params)
{
    FEAIS_ActionResult Result;
    Result.bSuccess = false;

    if (!OwnerCharacter)
    {
        Result.Message = TEXT("No owner character");
        return Result;
    }

    UAIComponent* AIComp = OwnerCharacter->GetAIComponent();
    if (!AIComp)
    {
        Result.Message = TEXT("No AIComponent");
        return Result;
    }

    AIComp->SetBlackboardVector(TEXT("AimTarget"), FVector::ZeroVector);
    AIComp->SetBlackboardVector(TEXT("GK_ShotImpactPoint"), FVector::ZeroVector);
    AIComp->SetBlackboardBool(TEXT("IsDiveRecommended"), false);
    AIComp->SetBlackboardBool(TEXT("DiveComplete"), true);

    Result.bSuccess = true;
    Result.Message = TEXT("Targets cleared");
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

    // [GK Distribution] If no target specified, check blackboard for pre-selected pass target
    FString EffectiveTarget = Params.Target;
    if (EffectiveTarget.IsEmpty())
    {
        UAIComponent* AIComp = OwnerCharacter->GetAIComponent();
        if (AIComp && AIComp->GetBlackboardBool(TEXT("HasSelectedPassTarget")))
        {
            FVector TargetPos = AIComp->GetBlackboardVector(TEXT("SelectedPassTargetPosition"));
            if (!TargetPos.IsZero())
            {
                const FVector MyLoc = OwnerCharacter->GetActorLocation();
                Direction = (TargetPos - MyLoc).GetSafeNormal();
                
                float Dist2D = FVector::Dist2D(MyLoc, TargetPos);
                PassSpeed = FMath::Clamp(Dist2D / 0.85f * Power, 600.0f, MF_Constants::BallPassSpeed);
                
                bTargetFound = true;
                Result.Message = TEXT("Passing to pre-selected target from blackboard");
                UE_LOG(LogTemp, Log, TEXT("[MF.Pass] Using blackboard SelectedPassTargetPosition"));
            }
        }
    }

    // [Original] Target Support - only if not already found from blackboard
    if (!bTargetFound && !EffectiveTarget.IsEmpty())
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

FEAIS_ActionResult UMF_EAISActionExecutorComponent::HandleMoveTo(const FAIActionParams& Params)
{
    FEAIS_ActionResult Result;
    Result.bSuccess = false;

    if (!OwnerCharacter)
    {
        Result.Message = TEXT("No owner character");
        return Result;
    }

    FString TargetName = Params.Target.IsEmpty() ? TEXT("SupportPosition") : Params.Target;
    FVector TargetLocation = FVector::ZeroVector;

    // Resolve target via TargetProvider interface
    if (!IEAIS_TargetProvider::Execute_EAIS_GetTargetLocation(OwnerCharacter, FName(*TargetName), TargetLocation))
    {
        UE_LOG(LogTemp, Warning, TEXT("[MF.MoveTo] %s: Target '%s' not found!"), *OwnerCharacter->GetName(), *TargetName);
        Result.Message = FString::Printf(TEXT("Target not found: %s"), *TargetName);
        return Result;
    }

    FVector CurrentLocation = OwnerCharacter->GetActorLocation();
    float DistToTarget = FVector::Dist2D(CurrentLocation, TargetLocation);

    // Already at destination
    if (DistToTarget < 75.0f)
    {
        Result.bSuccess = true;
        Result.Message = TEXT("Already at destination");
        return Result;
    }

    // Calculate direction
    FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal2D();

    // Use AddMovementInput directly (works regardless of controller type - fixes GK movement issue)
    OwnerCharacter->AddMovementInput(Direction, 1.0f);

    // Debug log every second (throttle to reduce spam)
    static float LastLogTime = 0.0f;
    float Now = OwnerCharacter->GetWorld() ? OwnerCharacter->GetWorld()->GetTimeSeconds() : 0.0f;
    if (Now - LastLogTime > 1.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("[MF.MoveTo] %s: Moving to %s (Dist: %.0f) Dir: (%.2f, %.2f)"),
            *OwnerCharacter->GetName(), *TargetName, DistToTarget, Direction.X, Direction.Y);
        LastLogTime = Now;
    }

    Result.bSuccess = true;
    Result.Message = FString::Printf(TEXT("Moving to %s (%.0f units away)"), *TargetName, DistToTarget);
    return Result;
}

FEAIS_ActionResult UMF_EAISActionExecutorComponent::HandleSelectPassTarget(const FAIActionParams& Params)
{
    FEAIS_ActionResult Result;
    Result.bSuccess = false;

    if (!OwnerCharacter)
    {
        Result.Message = TEXT("No owner character");
        return Result;
    }

    UAIComponent* AIComp = OwnerCharacter->GetAIComponent();
    if (!AIComp)
    {
        Result.Message = TEXT("No AIComponent");
        return Result;
    }

    // Find best pass target among teammates
    AMF_PlayerCharacter* BestTarget = nullptr;
    float BestScore = -9999.0f;

    EMF_TeamID MyTeam = OwnerCharacter->GetTeamID();
    FVector MyLocation = OwnerCharacter->GetActorLocation();

    // GK should NOT pass to themselves
    // Prefer: Defenders > Midfielders > Strikers (safety-first distribution)
    // Score factors:
    //   - Distance to nearest opponent (higher = safer = better)
    //   - Role priority (Defender > Midfielder > Striker for GK)
    //   - Distance from GK (not too close, not too far)
    //   - Clear passing lane (no opponents blocking)

    for (TActorIterator<AMF_PlayerCharacter> It(OwnerCharacter->GetWorld()); It; ++It)
    {
        AMF_PlayerCharacter* Teammate = *It;

        // Skip self, different team, goalkeepers
        if (!Teammate || Teammate == OwnerCharacter)
            continue;
        if (Teammate->GetTeamID() != MyTeam)
            continue;
        if (Teammate->AIProfile.Contains(TEXT("Goalkeeper")))
            continue;

        FVector TeammateLocation = Teammate->GetActorLocation();
        float DistToTeammate = FVector::Dist(MyLocation, TeammateLocation);

        // Skip if too close (< 5m) or too far (> 60m)
        if (DistToTeammate < 500.0f || DistToTeammate > 6000.0f)
            continue;

        // Calculate opponent proximity (safety score)
        float MinOpponentDist = 9999.0f;
        for (TActorIterator<AMF_PlayerCharacter> OppIt(OwnerCharacter->GetWorld()); OppIt; ++OppIt)
        {
            AMF_PlayerCharacter* Opponent = *OppIt;
            if (!Opponent || Opponent->GetTeamID() == MyTeam || Opponent->GetTeamID() == EMF_TeamID::None)
                continue;

            float OppDist = FVector::Dist(TeammateLocation, Opponent->GetActorLocation());
            MinOpponentDist = FMath::Min(MinOpponentDist, OppDist);
        }

        // Score calculation
        float Score = 0.0f;

        // Safety: More distance to opponent = better (max 30 points)
        Score += FMath::Clamp(MinOpponentDist / 500.0f, 0.0f, 10.0f) * 3.0f;

        // Role priority for GK distribution (prefer safe players)
        if (Teammate->AIProfile.Contains(TEXT("Defender")))
        {
            Score += 20.0f; // Defenders are safest
        }
        else if (Teammate->AIProfile.Contains(TEXT("Midfielder")))
        {
            Score += 10.0f; // Midfielders are okay
        }
        else if (Teammate->AIProfile.Contains(TEXT("Striker")))
        {
            Score += 5.0f; // Strikers are risky (long ball)
        }

        // Distance penalty: Prefer medium range (20-35m)
        float IdealDist = 2750.0f; // ~27.5m
        float DistPenalty = FMath::Abs(DistToTeammate - IdealDist) / 500.0f;
        Score -= DistPenalty * 2.0f;

        // Passing lane check (is there an opponent in the way?)
        FVector ToTeammate = (TeammateLocation - MyLocation).GetSafeNormal();
        bool bLaneBlocked = false;
        for (TActorIterator<AMF_PlayerCharacter> OppIt(OwnerCharacter->GetWorld()); OppIt; ++OppIt)
        {
            AMF_PlayerCharacter* Opponent = *OppIt;
            if (!Opponent || Opponent->GetTeamID() == MyTeam || Opponent->GetTeamID() == EMF_TeamID::None)
                continue;

            FVector ToOpponent = Opponent->GetActorLocation() - MyLocation;
            float OpponentDist = ToOpponent.Size();

            // Only check opponents between GK and teammate
            if (OpponentDist < DistToTeammate)
            {
                float DotProduct = FVector::DotProduct(ToTeammate, ToOpponent.GetSafeNormal());
                if (DotProduct > 0.9f) // Within ~25 degree cone
                {
                    bLaneBlocked = true;
                    break;
                }
            }
        }

        if (bLaneBlocked)
        {
            Score -= 15.0f; // Heavy penalty for blocked lane
        }

        // Update best target
        if (Score > BestScore)
        {
            BestScore = Score;
            BestTarget = Teammate;
        }
    }

    if (BestTarget)
    {
        // Store target in blackboard for MF.Pass to use
        AIComp->SetBlackboardVector(TEXT("SelectedPassTargetPosition"), BestTarget->GetActorLocation());
        AIComp->SetBlackboardBool(TEXT("HasSelectedPassTarget"), true);

        Result.bSuccess = true;
        Result.Message = FString::Printf(TEXT("Selected pass target: %s (Score: %.1f)"), *BestTarget->AIProfile, BestScore);

        UE_LOG(LogTemp, Log, TEXT("[GK Distribution] Selected target: %s, Role: %s, Score: %.1f, Dist: %.0f"),
            *BestTarget->GetName(), *BestTarget->AIProfile, BestScore, FVector::Dist(MyLocation, BestTarget->GetActorLocation()));
    }
    else
    {
        AIComp->SetBlackboardBool(TEXT("HasSelectedPassTarget"), false);
        Result.bSuccess = false;
        Result.Message = TEXT("No valid pass target found");

        UE_LOG(LogTemp, Warning, TEXT("[GK Distribution] No valid pass target found!"));
    }

    return Result;
}
