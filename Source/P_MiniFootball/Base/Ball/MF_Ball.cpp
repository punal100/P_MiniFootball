/*
 * @Author: Punal Manalan
 * @Description: MF_Ball - Implementation
 *               Math-based ball physics (NO UE Physics)
 *               Full network replication for Listen Server and Dedicated Server
 * @Date: 07/12/2025
 */

#include "Ball/MF_Ball.h"
#include "Player/MF_PlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

AMF_Ball::AMF_Ball()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create collision sphere (not for physics, just for overlap detection)
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->InitSphereRadius(MF_Constants::BallRadius);
    CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    CollisionSphere->SetGenerateOverlapEvents(true);
    // NO PHYSICS - we do our own math
    CollisionSphere->SetSimulatePhysics(false);
    RootComponent = CollisionSphere;

    // Create mesh
    BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
    BallMesh->SetupAttachment(RootComponent);
    BallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Ball properties
    BallRadius = MF_Constants::BallRadius;
    PossessionOffset = FVector(60.0f, 0.0f, 20.0f); // In front of player, slightly elevated

    // Initialize state
    CurrentBallState = EMF_BallState::Loose;
    CurrentPossessor = nullptr;
    Velocity = FVector::ZeroVector;
    AngularVelocity = FVector::ZeroVector;
    bIsGrounded = true;
    PossessionCooldown = 0.0f;

    // Network settings
    bReplicates = true;
    bAlwaysRelevant = true;
    SetNetUpdateFrequency(MF_Constants::NetUpdateFrequency);
    SetMinNetUpdateFrequency(MF_Constants::MinNetUpdateFrequency);
}

void AMF_Ball::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMF_Ball, CurrentBallState);
    DOREPLIFETIME(AMF_Ball, CurrentPossessor);
    DOREPLIFETIME(AMF_Ball, ReplicatedPhysics);
}

void AMF_Ball::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("MF_Ball::BeginPlay - HasAuthority: %d"), HasAuthority());

    // Initialize interpolation
    LastReplicatedPosition = GetActorLocation();
    InterpolationTarget = LastReplicatedPosition;
}

void AMF_Ball::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update possession cooldown
    if (PossessionCooldown > 0.0f)
    {
        PossessionCooldown -= DeltaTime;
    }

    if (HasAuthority())
    {
        // Server: Run physics simulation
        switch (CurrentBallState)
        {
        case EMF_BallState::Loose:
        case EMF_BallState::InFlight:
            UpdatePhysics(DeltaTime);
            break;
        case EMF_BallState::Possessed:
            UpdatePossessedPosition();
            break;
        case EMF_BallState::OutOfBounds:
            // Ball is stationary, waiting for reset
            break;
        }

        // Update replicated physics data
        ReplicatedPhysics.Location = GetActorLocation();
        ReplicatedPhysics.Velocity = Velocity;
        ReplicatedPhysics.ServerTimestamp = GetWorld()->GetTimeSeconds();
    }
    else
    {
        // Client: Interpolate towards replicated position
        ClientInterpolate(DeltaTime);
    }
}

// ==================== Ball Actions ====================

void AMF_Ball::Kick(FVector Direction, float Power, bool bAddHeight)
{
    if (!HasAuthority())
    {
        return;
    }

    // Normalize direction
    Direction.Normalize();

    // Calculate kick velocity
    Velocity = Direction * Power;

    // Add some height for shots
    if (bAddHeight)
    {
        Velocity.Z += Power * 0.3f; // Add ~30% power as upward velocity
    }

    // Add spin (simplified)
    AngularVelocity = FVector::CrossProduct(FVector::UpVector, Direction) * (Power / 100.0f);

    // Release possession
    if (CurrentPossessor)
    {
        LastKicker = CurrentPossessor;
        ReleasePossession();
    }

    // Set state to in flight
    SetBallState(EMF_BallState::InFlight);
    bIsGrounded = false;

    UE_LOG(LogTemp, Log, TEXT("MF_Ball::Kick - Direction: %s, Power: %f, Velocity: %s"),
           *Direction.ToString(), Power, *Velocity.ToString());
}

void AMF_Ball::SetPossessor(AMF_PlayerCharacter *NewPossessor)
{
    if (!HasAuthority())
    {
        return;
    }

    if (CurrentPossessor == NewPossessor)
    {
        return;
    }

    AMF_PlayerCharacter *OldPossessor = CurrentPossessor;
    CurrentPossessor = NewPossessor;

    if (NewPossessor)
    {
        SetBallState(EMF_BallState::Possessed);
        Velocity = FVector::ZeroVector;
        AngularVelocity = FVector::ZeroVector;

        // Update player state
        NewPossessor->SetHasBall(true);
        NewPossessor->SetPossessedBall(this);

        UE_LOG(LogTemp, Log, TEXT("MF_Ball::SetPossessor - New possessor: %s"), *NewPossessor->GetName());
    }
    else
    {
        SetBallState(EMF_BallState::Loose);
    }

    // Update old possessor
    if (OldPossessor && OldPossessor != NewPossessor)
    {
        OldPossessor->SetHasBall(false);
        OldPossessor->SetPossessedBall(nullptr);
    }

    // Broadcast event
    OnPossessionChanged.Broadcast(this, OldPossessor, NewPossessor);
}

void AMF_Ball::ReleasePossession()
{
    if (!HasAuthority())
    {
        return;
    }

    if (CurrentPossessor)
    {
        AMF_PlayerCharacter *OldPossessor = CurrentPossessor;
        OldPossessor->SetHasBall(false);
        OldPossessor->SetPossessedBall(nullptr);
        CurrentPossessor = nullptr;

        // Small cooldown before can be picked up again
        PossessionCooldown = 0.2f;

        OnPossessionChanged.Broadcast(this, OldPossessor, nullptr);
    }
}

void AMF_Ball::ResetToPosition(FVector NewPosition)
{
    if (!HasAuthority())
    {
        return;
    }

    // Release any possession
    ReleasePossession();

    // Reset physics
    Velocity = FVector::ZeroVector;
    AngularVelocity = FVector::ZeroVector;
    bIsGrounded = true;

    // Move to position
    SetActorLocation(NewPosition);
    SetBallState(EMF_BallState::Loose);

    // Update replicated data
    ReplicatedPhysics.Location = NewPosition;
    ReplicatedPhysics.Velocity = FVector::ZeroVector;

    UE_LOG(LogTemp, Log, TEXT("MF_Ball::ResetToPosition - %s"), *NewPosition.ToString());
}

bool AMF_Ball::CanBePickedUpBy(AMF_PlayerCharacter *Player) const
{
    if (!Player)
    {
        return false;
    }

    // Can't pick up if someone else has it
    if (CurrentPossessor != nullptr)
    {
        return false;
    }

    // Cooldown active
    if (PossessionCooldown > 0.0f)
    {
        return false;
    }

    // Must be loose or InFlight (not out of bounds)
    if (CurrentBallState == EMF_BallState::OutOfBounds)
    {
        return false;
    }

    // Check distance
    float DistSq = FVector::DistSquared(GetActorLocation(), Player->GetActorLocation());
    float PickupRadiusSq = FMath::Square(MF_Constants::BallPickupRadius);

    return DistSq <= PickupRadiusSq;
}

// ==================== Rep Notifies ====================

void AMF_Ball::OnRep_BallState()
{
    OnBallStateChanged.Broadcast(this, CurrentBallState);
    UE_LOG(LogTemp, Log, TEXT("MF_Ball::OnRep_BallState - State: %d"), static_cast<int32>(CurrentBallState));
}

void AMF_Ball::OnRep_Possessor()
{
    UE_LOG(LogTemp, Log, TEXT("MF_Ball::OnRep_Possessor - Possessor: %s"),
           CurrentPossessor ? *CurrentPossessor->GetName() : TEXT("null"));
}

void AMF_Ball::OnRep_BallPhysics()
{
    // Set interpolation target
    InterpolationTarget = ReplicatedPhysics.Location;
    InterpolationVelocity = ReplicatedPhysics.Velocity;
    LastReplicatedPosition = ReplicatedPhysics.Location;
}

// ==================== Internal Physics ====================

void AMF_Ball::UpdatePhysics(float DeltaTime)
{
    // Apply forces (gravity, friction)
    ApplyForces(DeltaTime);

    // Update position
    FVector NewLocation = GetActorLocation() + Velocity * DeltaTime;
    SetActorLocation(NewLocation);

    // Update rotation from angular velocity (visual only)
    if (!AngularVelocity.IsNearlyZero())
    {
        FRotator DeltaRotation = FRotator(
            FMath::RadiansToDegrees(AngularVelocity.Y * DeltaTime),
            FMath::RadiansToDegrees(AngularVelocity.Z * DeltaTime),
            FMath::RadiansToDegrees(AngularVelocity.X * DeltaTime));
        AddActorLocalRotation(DeltaRotation);
    }

    // Check collisions
    CheckGroundCollision();
    CheckBoundaryCollisions();
    CheckGoalCollisions();

    // Check if ball has stopped
    if (bIsGrounded && Velocity.SizeSquared() < 100.0f) // Less than 10 cm/s
    {
        Velocity = FVector::ZeroVector;
        AngularVelocity = FVector::ZeroVector;

        if (CurrentBallState == EMF_BallState::InFlight)
        {
            SetBallState(EMF_BallState::Loose);
        }
    }
}

void AMF_Ball::ApplyForces(float DeltaTime)
{
    // Gravity (only if not grounded)
    if (!bIsGrounded)
    {
        Velocity.Z -= MF_Constants::Gravity * DeltaTime;
    }

    // Friction (ground friction when grounded, air resistance when flying)
    float FrictionCoeff = bIsGrounded ? MF_Constants::BallFriction : MF_Constants::BallAirResistance;

    // Apply friction to XY velocity
    FVector XYVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
    if (!XYVelocity.IsNearlyZero())
    {
        float Deceleration = FrictionCoeff * DeltaTime;
        float CurrentSpeed = XYVelocity.Size();
        float NewSpeed = FMath::Max(0.0f, CurrentSpeed - Deceleration);

        XYVelocity = XYVelocity.GetSafeNormal() * NewSpeed;
        Velocity.X = XYVelocity.X;
        Velocity.Y = XYVelocity.Y;
    }

    // Angular velocity decay
    AngularVelocity *= (1.0f - 2.0f * DeltaTime);
}

void AMF_Ball::CheckGroundCollision()
{
    float GroundZ = MF_Constants::GroundZ + BallRadius;
    FVector Location = GetActorLocation();

    if (Location.Z <= GroundZ)
    {
        // Hit ground
        if (!bIsGrounded && Velocity.Z < 0.0f)
        {
            // Bounce
            float BounceVelocity = -Velocity.Z * MF_Constants::BallBounciness;

            if (BounceVelocity > 50.0f) // Minimum bounce threshold
            {
                Velocity.Z = BounceVelocity;
                bIsGrounded = false;
            }
            else
            {
                Velocity.Z = 0.0f;
                bIsGrounded = true;
            }
        }

        // Clamp to ground
        Location.Z = GroundZ;
        SetActorLocation(Location);
    }
    else
    {
        bIsGrounded = false;
    }
}

void AMF_Ball::CheckBoundaryCollisions()
{
    FVector Location = GetActorLocation();
    bool bOutOfBounds = false;

    // Check X boundaries (side lines)
    float HalfWidth = MF_Constants::FieldWidth / 2.0f;
    if (FMath::Abs(Location.X) > HalfWidth + MF_Constants::OutOfBoundsBuffer)
    {
        bOutOfBounds = true;
    }

    // Check Y boundaries (goal lines - but not in goal area)
    float HalfLength = MF_Constants::FieldLength / 2.0f;
    if (FMath::Abs(Location.Y) > HalfLength + MF_Constants::OutOfBoundsBuffer)
    {
        // Check if it's in the goal width range
        float HalfGoalWidth = MF_Constants::GoalWidth / 2.0f;
        if (FMath::Abs(Location.X) > HalfGoalWidth)
        {
            bOutOfBounds = true;
        }
    }

    // Wall bounces (before going fully out of bounds)
    if (FMath::Abs(Location.X) > HalfWidth - BallRadius)
    {
        // Bounce off side wall
        Velocity.X = -Velocity.X * MF_Constants::BallBounciness;
        Location.X = FMath::Sign(Location.X) * (HalfWidth - BallRadius);
        SetActorLocation(Location);
    }

    if (bOutOfBounds)
    {
        SetBallState(EMF_BallState::OutOfBounds);
        Velocity = FVector::ZeroVector;
        AngularVelocity = FVector::ZeroVector;
        OnBallOutOfBounds.Broadcast(this);
    }
}

void AMF_Ball::CheckGoalCollisions()
{
    FVector Location = GetActorLocation();
    float HalfLength = MF_Constants::FieldLength / 2.0f;
    float HalfGoalWidth = MF_Constants::GoalWidth / 2.0f;

    // Check Team A goal (positive Y end)
    if (Location.Y > HalfLength && FMath::Abs(Location.X) < HalfGoalWidth && Location.Z < MF_Constants::GoalHeight)
    {
        // Team B scored!
        OnGoalScored.Broadcast(this, EMF_TeamID::TeamB);
        SetBallState(EMF_BallState::OutOfBounds);
        Velocity = FVector::ZeroVector;
        UE_LOG(LogTemp, Log, TEXT("GOAL! Team B scores!"));
    }
    // Check Team B goal (negative Y end)
    else if (Location.Y < -HalfLength && FMath::Abs(Location.X) < HalfGoalWidth && Location.Z < MF_Constants::GoalHeight)
    {
        // Team A scored!
        OnGoalScored.Broadcast(this, EMF_TeamID::TeamA);
        SetBallState(EMF_BallState::OutOfBounds);
        Velocity = FVector::ZeroVector;
        UE_LOG(LogTemp, Log, TEXT("GOAL! Team A scores!"));
    }
}

void AMF_Ball::UpdatePossessedPosition()
{
    if (!CurrentPossessor)
    {
        return;
    }

    // Position ball in front of player
    FVector PlayerLocation = CurrentPossessor->GetActorLocation();
    FRotator PlayerRotation = CurrentPossessor->GetActorRotation();

    FVector Offset = PlayerRotation.RotateVector(PossessionOffset);
    FVector NewLocation = PlayerLocation + Offset;

    SetActorLocation(NewLocation);
}

void AMF_Ball::ClientInterpolate(float DeltaTime)
{
    if (CurrentBallState == EMF_BallState::Possessed)
    {
        // When possessed, just follow possessor (they handle their own interpolation)
        UpdatePossessedPosition();
        return;
    }

    // Interpolate towards replicated position
    FVector CurrentLocation = GetActorLocation();
    float InterpSpeed = 15.0f; // Adjust for smoothness vs responsiveness

    FVector NewLocation = FMath::VInterpTo(CurrentLocation, InterpolationTarget, DeltaTime, InterpSpeed);
    SetActorLocation(NewLocation);
}

void AMF_Ball::SetBallState(EMF_BallState NewState)
{
    if (CurrentBallState != NewState)
    {
        CurrentBallState = NewState;

        // Call rep notify on server too for local effects
        if (HasAuthority())
        {
            OnRep_BallState();
        }
    }
}
