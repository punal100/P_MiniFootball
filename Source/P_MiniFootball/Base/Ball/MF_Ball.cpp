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
#include "EngineUtils.h" // For TActorIterator

AMF_Ball::AMF_Ball()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create collision sphere for overlap detection (larger than visual ball for pickup detection)
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->InitSphereRadius(MF_Constants::BallPickupRadius); // Use pickup radius for overlap detection

    // Set up collision to overlap with pawns/characters
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    CollisionSphere->SetGenerateOverlapEvents(true);

    // NO PHYSICS - we do our own math
    CollisionSphere->SetSimulatePhysics(false);
    RootComponent = CollisionSphere;

    // Create mesh (visual ball is smaller than collision sphere)
    BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
    BallMesh->SetupAttachment(RootComponent);
    BallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BallMesh->SetRelativeScale3D(FVector(MF_Constants::BallRadius / 50.0f)); // Scale to proper ball size

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

    // Bind overlap event for automatic ball pickup
    if (CollisionSphere)
    {
        CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AMF_Ball::OnBallOverlap);
    }

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

    // Debug: Log ball state periodically
    static float LastStateLogTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastStateLogTime > 2.0f) // Log every 2 seconds
    {
        UE_LOG(LogTemp, Log, TEXT("MF_Ball::Tick - State: %d, Possessor: %s, HasAuthority: %d, Location: %s"),
               (int32)CurrentBallState,
               CurrentPossessor ? *CurrentPossessor->GetName() : TEXT("None"),
               HasAuthority(),
               *GetActorLocation().ToString());
        LastStateLogTime = CurrentTime;
    }

    if (HasAuthority())
    {
        // Server: Run physics simulation
        switch (CurrentBallState)
        {
        case EMF_BallState::Loose:
        case EMF_BallState::InFlight:
            UpdatePhysics(DeltaTime);
            // Also check for nearby players who can pick up the ball (backup for overlap events)
            CheckForNearbyPlayers();
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
        UE_LOG(LogTemp, Warning, TEXT("MF_Ball::CanBePickedUpBy - FAIL: Player is null"));
        return false;
    }

    // Can't pick up if someone else has it
    if (CurrentPossessor != nullptr)
    {
        UE_LOG(LogTemp, Log, TEXT("MF_Ball::CanBePickedUpBy - FAIL: Already possessed by %s"), *CurrentPossessor->GetName());
        return false;
    }

    // Cooldown active
    if (PossessionCooldown > 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("MF_Ball::CanBePickedUpBy - FAIL: Cooldown active (%.2f remaining)"), PossessionCooldown);
        return false;
    }

    // Must be loose or InFlight (not out of bounds)
    if (CurrentBallState == EMF_BallState::OutOfBounds)
    {
        UE_LOG(LogTemp, Log, TEXT("MF_Ball::CanBePickedUpBy - FAIL: Ball is out of bounds"));
        return false;
    }

    // Don't check distance here - let the caller handle distance
    // This function should only check if the ball CAN be picked up, not if the player is close enough
    return true;
}

// ==================== Overlap Detection ====================

void AMF_Ball::OnBallOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
                             UPrimitiveComponent *OtherComp, int32 OtherBodyIndex,
                             bool bFromSweep, const FHitResult &SweepResult)
{
    // Only server handles possession changes
    if (!HasAuthority())
    {
        return;
    }

    // Check if it's a player character
    AMF_PlayerCharacter *Player = Cast<AMF_PlayerCharacter>(OtherActor);
    if (!Player)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("MF_Ball::OnBallOverlap - Player: %s, CanPickup: %d"),
           *Player->GetName(), CanBePickedUpBy(Player));

    // Try to give possession to the player
    if (CanBePickedUpBy(Player))
    {
        SetPossessor(Player);
        UE_LOG(LogTemp, Log, TEXT("MF_Ball::OnBallOverlap - Possession given to %s"), *Player->GetName());
    }
}

void AMF_Ball::CheckForNearbyPlayers()
{
    // Only run on server when ball is loose
    if (!HasAuthority() || CurrentPossessor != nullptr)
    {
        return;
    }

    // Find all player characters and check distance
    FVector BallLocation = GetActorLocation();

    // Debug: count players found
    int32 PlayerCount = 0;

    for (TActorIterator<AMF_PlayerCharacter> It(GetWorld()); It; ++It)
    {
        AMF_PlayerCharacter *Player = *It;
        if (!Player)
        {
            continue;
        }

        PlayerCount++;

        float Dist = FVector::Dist(BallLocation, Player->GetActorLocation());
        float PickupRadius = MF_Constants::BallPickupRadius;

        // Always log distance for debugging
        static float LastDistLogTime = 0.0f;
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastDistLogTime > 1.0f) // Log every second
        {
            UE_LOG(LogTemp, Warning, TEXT("MF_Ball::CheckForNearbyPlayers - Player: %s, Distance: %.1f, PickupRadius: %.1f, InRange: %d, CanPickup: %d"),
                   *Player->GetName(), Dist, PickupRadius, Dist <= PickupRadius, CanBePickedUpBy(Player));
            LastDistLogTime = CurrentTime;
        }

        if (Dist <= PickupRadius && CanBePickedUpBy(Player))
        {
            UE_LOG(LogTemp, Warning, TEXT("MF_Ball::CheckForNearbyPlayers - PICKING UP! Player: %s, Distance: %.1f"),
                   *Player->GetName(), Dist);
            SetPossessor(Player);
            break; // Only one player can pick up at a time
        }
    }

    // Debug: Log if no players found
    static float LastNoPlayerLogTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (PlayerCount == 0 && CurrentTime - LastNoPlayerLogTime > 3.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_Ball::CheckForNearbyPlayers - NO PLAYERS FOUND IN WORLD!"));
        LastNoPlayerLogTime = CurrentTime;
    }
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
        UE_LOG(LogTemp, Warning, TEXT("MF_Ball::UpdatePossessedPosition - No CurrentPossessor!"));
        return;
    }

    // Position ball in front of player
    FVector PlayerLocation = CurrentPossessor->GetActorLocation();
    FRotator PlayerRotation = CurrentPossessor->GetActorRotation();

    FVector Offset = PlayerRotation.RotateVector(PossessionOffset);
    FVector NewLocation = PlayerLocation + Offset;

    SetActorLocation(NewLocation);

    // Debug: Log ball following player
    static float LastLogTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastLogTime > 1.0f) // Log every second
    {
        UE_LOG(LogTemp, Log, TEXT("MF_Ball::UpdatePossessedPosition - Following %s at %s, Ball at %s"),
               *CurrentPossessor->GetName(), *PlayerLocation.ToString(), *NewLocation.ToString());
        LastLogTime = CurrentTime;
    }
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
