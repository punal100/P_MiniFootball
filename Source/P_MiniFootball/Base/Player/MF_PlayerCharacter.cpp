/*
 * @Author: Punal Manalan
 * @Description: MF_PlayerCharacter - Implementation
 *               Full network replication for Listen Server and Dedicated Server
 * @Date: 07/12/2025
 */

#include "Player/MF_PlayerCharacter.h"
#include "Player/MF_PlayerController.h"
#include "Player/MF_InputHandler.h"
#include "Ball/MF_Ball.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Integration/CPP_EnhancedInputIntegration.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EngineUtils.h"

AMF_PlayerCharacter::AMF_PlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // NOTE: ACharacter's CapsuleComponent is the root - don't change it!
    // The CharacterMovementComponent requires CapsuleComponent to be root.

    // Enable overlap events on the capsule for ball pickup detection
    if (UCapsuleComponent *Capsule = GetCapsuleComponent())
    {
        Capsule->SetGenerateOverlapEvents(true);
    }

    // Create Input Handler component
    InputHandler = CreateDefaultSubobject<UMF_InputHandler>(TEXT("InputHandler"));

    // Create Camera Boom (Spring Arm) for top-down view
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f)); // Look down at 60 degree angle
    CameraBoom->TargetArmLength = 1500.0f;                         // Distance from player
    CameraBoom->bDoCollisionTest = false;                          // Don't pull camera in when obstructed
    CameraBoom->bUsePawnControlRotation = false;                   // Don't rotate with controller
    CameraBoom->bInheritPitch = false;
    CameraBoom->bInheritYaw = false;
    CameraBoom->bInheritRoll = false;
    CameraBoom->bEnableCameraLag = true; // Smooth camera follow
    CameraBoom->CameraLagSpeed = 5.0f;

    // Create Top-Down Camera
    TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
    TopDownCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    TopDownCamera->bUsePawnControlRotation = false; // Don't rotate with controller

    // Don't use controller rotation - let movement component handle it
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Setup movement
    if (UCharacterMovementComponent *Movement = GetCharacterMovement())
    {
        Movement->MaxWalkSpeed = MF_Constants::WalkSpeed;
        Movement->MaxAcceleration = MF_Constants::Acceleration;
        Movement->bOrientRotationToMovement = true;
        Movement->RotationRate = FRotator(0.0f, MF_Constants::TurnRate, 0.0f);

        // Ground movement mode settings
        Movement->GravityScale = 1.0f;
        Movement->BrakingDecelerationWalking = 2048.0f;
        Movement->GroundFriction = 8.0f;

        // Allow the character to walk off ledges
        Movement->bCanWalkOffLedges = true;
        Movement->bCanWalkOffLedgesWhenCrouching = true;

        // Don't constrain to plane - let gravity handle Z
        Movement->bConstrainToPlane = false;

        // Set initial movement mode (will be overridden when grounded)
        Movement->SetMovementMode(MOVE_Falling);
    }

    // Network settings for smooth replication
    bReplicates = true;
    SetReplicateMovement(true);
    SetNetUpdateFrequency(MF_Constants::NetUpdateFrequency);
    SetMinNetUpdateFrequency(MF_Constants::MinNetUpdateFrequency);
}

void AMF_PlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicate to all clients
    DOREPLIFETIME(AMF_PlayerCharacter, TeamID);
    DOREPLIFETIME(AMF_PlayerCharacter, PlayerID);
    DOREPLIFETIME(AMF_PlayerCharacter, bHasBall);
    DOREPLIFETIME(AMF_PlayerCharacter, CurrentPlayerState);
    DOREPLIFETIME(AMF_PlayerCharacter, bIsSprinting);
    DOREPLIFETIME(AMF_PlayerCharacter, CurrentBall);
}

void AMF_PlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::BeginPlay - HasAuthority: %d, IsLocallyControlled: %d"),
           HasAuthority(), IsLocallyControlled());

    // Log spawn position
    FVector SpawnLoc = GetActorLocation();
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::BeginPlay - Spawned at Location: %s"), *SpawnLoc.ToString());

    // Debug: Log movement component state
    if (UCharacterMovementComponent *MovementComp = GetCharacterMovement())
    {
        UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::BeginPlay - MovementMode: %d, MaxWalkSpeed: %.1f, MaxAccel: %.1f, IsOnGround: %d"),
               (int32)MovementComp->MovementMode,
               MovementComp->MaxWalkSpeed,
               MovementComp->MaxAcceleration,
               MovementComp->IsMovingOnGround());

        // Check if we're stuck in geometry
        if (UCapsuleComponent *Capsule = GetCapsuleComponent())
        {
            float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
            UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::BeginPlay - CapsuleHalfHeight: %.1f, BottomZ: %.1f"),
                   CapsuleHalfHeight, SpawnLoc.Z - CapsuleHalfHeight);
        }
    }
}

void AMF_PlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update movement
    UpdateMovement(DeltaTime);

    // Update timers (Server only)
    if (HasAuthority())
    {
        // Tackle cooldown
        if (TackleCooldownRemaining > 0.0f)
        {
            TackleCooldownRemaining -= DeltaTime;
        }

        // Stun timer
        if (StunTimeRemaining > 0.0f)
        {
            StunTimeRemaining -= DeltaTime;
            if (StunTimeRemaining <= 0.0f)
            {
                SetPlayerState(EMF_PlayerState::Idle);
            }
        }
    }
}

void AMF_PlayerCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Input is handled via MF_InputHandler + P_MEIS, not traditional input component
    // This is intentionally minimal - P_MEIS handles all input binding

    // Now that InputComponent exists, try to bind any pending P_MEIS actions
    if (InputHandler && InputHandler->GetIntegration())
    {
        int32 BoundCount = InputHandler->GetIntegration()->TryBindPendingActions();
        UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::SetupPlayerInputComponent - Bound %d pending actions"), BoundCount);
    }
}

void AMF_PlayerCharacter::PossessedBy(AController *NewController)
{
    Super::PossessedBy(NewController);

    UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::PossessedBy - Controller: %s"),
           NewController ? *NewController->GetName() : TEXT("null"));

    // Initialize input on the owning client
    if (APlayerController *PC = Cast<APlayerController>(NewController))
    {
        // Only setup on locally controlled player
        if (PC->IsLocalController())
        {
            SetupInputBindings();
        }
    }
}

void AMF_PlayerCharacter::UnPossessed()
{
    // Reset movement input to prevent ghost movement after switching
    CurrentMoveInput = FVector2D::ZeroVector;
    
    // Stop any ongoing movement
    if (UCharacterMovementComponent *Movement = GetCharacterMovement())
    {
        Movement->StopMovementImmediately();
    }

    // Cleanup input
    if (InputHandler)
    {
        InputHandler->CleanupInput();
    }

    Super::UnPossessed();
}

void AMF_PlayerCharacter::OnRep_Owner()
{
    Super::OnRep_Owner();

    // This is called on clients when ownership/possession replicates
    // Check if we're now locally controlled and set up input
    AController* NewController = GetController();
    
    UE_LOG(LogTemp, Warning, TEXT("MF_PlayerCharacter::OnRep_Owner - %s, Controller: %s, IsLocallyControlled: %d"),
           *GetName(),
           NewController ? *NewController->GetName() : TEXT("null"),
           IsLocallyControlled());

    if (IsLocallyControlled())
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerCharacter::OnRep_Owner - Setting up input bindings for LOCAL player"));
        SetupInputBindings();
    }
}

void AMF_PlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    // Additional logic if needed when player state replicates
}

void AMF_PlayerCharacter::SetTeamID(EMF_TeamID NewTeam)
{
    if (HasAuthority())
    {
        TeamID = NewTeam;
        OnRep_TeamID();
    }
}

void AMF_PlayerCharacter::SetHasBall(bool bNewHasBall)
{
    if (HasAuthority())
    {
        if (bHasBall != bNewHasBall)
        {
            bHasBall = bNewHasBall;
            OnRep_HasBall();
        }
    }
}

void AMF_PlayerCharacter::SetPossessedBall(AMF_Ball *Ball)
{
    PossessedBall = Ball;
}

void AMF_PlayerCharacter::SetPlayerState(EMF_PlayerState NewState)
{
    if (HasAuthority())
    {
        if (CurrentPlayerState != NewState)
        {
            CurrentPlayerState = NewState;
            OnRep_CurrentPlayerState();
        }
    }
}

void AMF_PlayerCharacter::ApplyMoveInput(FVector2D MoveInput)
{
    CurrentMoveInput = MoveInput;

    // Send to server if we're a client
    if (!HasAuthority() && IsLocallyControlled())
    {
        Server_SendMoveInput(MoveInput, bIsSprinting);
    }
}

void AMF_PlayerCharacter::SetSprinting(bool bNewSprinting)
{
    if (bIsSprinting != bNewSprinting)
    {
        bIsSprinting = bNewSprinting;

        // Update movement speed
        if (UCharacterMovementComponent *Movement = GetCharacterMovement())
        {
            Movement->MaxWalkSpeed = bIsSprinting ? MF_Constants::SprintSpeed : MF_Constants::WalkSpeed;
        }

        // Notify server
        if (!HasAuthority() && IsLocallyControlled())
        {
            Server_SendMoveInput(CurrentMoveInput, bIsSprinting);
        }
    }
}

// ==================== Server RPCs ====================

bool AMF_PlayerCharacter::Server_RequestShoot_Validate(FVector Direction, float Power)
{
    // Basic validation
    return Power >= 0.0f && Power <= MF_Constants::BallShootSpeed * 2.0f;
}

void AMF_PlayerCharacter::Server_RequestShoot_Implementation(FVector Direction, float Power)
{
    UE_LOG(LogTemp, Warning, TEXT("=== Server_RequestShoot ==="));
    UE_LOG(LogTemp, Warning, TEXT("  Character: %s"), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("  Controller: %s"), *GetNameSafe(GetController()));
    UE_LOG(LogTemp, Warning, TEXT("  NetMode: %d"), static_cast<int32>(GetNetMode()));
    UE_LOG(LogTemp, Warning, TEXT("  HasAuthority: %d"), HasAuthority());
    UE_LOG(LogTemp, Warning, TEXT("  Direction: %s, Power: %.1f"), *Direction.ToString(), Power);
    
    ExecuteShoot(Direction, Power);
}

bool AMF_PlayerCharacter::Server_RequestPass_Validate(FVector Direction, float Power)
{
    return Power >= 0.0f && Power <= MF_Constants::BallPassSpeed * 2.0f;
}

void AMF_PlayerCharacter::Server_RequestPass_Implementation(FVector Direction, float Power)
{
    UE_LOG(LogTemp, Warning, TEXT("=== Server_RequestPass ==="));
    UE_LOG(LogTemp, Warning, TEXT("  Character: %s"), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("  Controller: %s"), *GetNameSafe(GetController()));
    UE_LOG(LogTemp, Warning, TEXT("  NetMode: %d"), static_cast<int32>(GetNetMode()));
    UE_LOG(LogTemp, Warning, TEXT("  Direction: %s, Power: %.1f"), *Direction.ToString(), Power);
    
    ExecutePass(Direction, Power);
}

bool AMF_PlayerCharacter::Server_RequestTackle_Validate()
{
    return true;
}

void AMF_PlayerCharacter::Server_RequestTackle_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Server_RequestTackle ==="));
    UE_LOG(LogTemp, Warning, TEXT("  Character: %s"), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("  Controller: %s"), *GetNameSafe(GetController()));
    UE_LOG(LogTemp, Warning, TEXT("  NetMode: %d"), static_cast<int32>(GetNetMode()));
    
    ExecuteTackle();
}

bool AMF_PlayerCharacter::Server_SendMoveInput_Validate(FVector2D MoveInput, bool bSprinting)
{
    // Validate input is normalized-ish
    return MoveInput.Size() <= 1.5f;
}

void AMF_PlayerCharacter::Server_SendMoveInput_Implementation(FVector2D MoveInput, bool bSprinting)
{
    CurrentMoveInput = MoveInput;

    if (bIsSprinting != bSprinting)
    {
        bIsSprinting = bSprinting;
        if (UCharacterMovementComponent *Movement = GetCharacterMovement())
        {
            Movement->MaxWalkSpeed = bIsSprinting ? MF_Constants::SprintSpeed : MF_Constants::WalkSpeed;
        }
    }
}

// ==================== Rep Notifies ====================

void AMF_PlayerCharacter::OnRep_TeamID()
{
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::OnRep_TeamID - Team: %d"), static_cast<int32>(TeamID));
    // TODO: Update visuals based on team (jersey color, etc.)
}

void AMF_PlayerCharacter::OnRep_HasBall()
{
    OnPossessionChanged.Broadcast(this, bHasBall);

    UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::OnRep_HasBall - HasBall: %d"), bHasBall);
}

void AMF_PlayerCharacter::OnRep_CurrentPlayerState()
{
    OnPlayerStateChanged.Broadcast(CurrentPlayerState);

    UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::OnRep_CurrentPlayerState - State: %d"),
           static_cast<int32>(CurrentPlayerState));
}

void AMF_PlayerCharacter::OnRep_CurrentBall()
{
    // Derived invariant enforcement: bHasBall == (CurrentBall != nullptr)
    bHasBall = (CurrentBall != nullptr);

    // Notify HUD / Ability system
    OnBallStateChanged.Broadcast(this, bHasBall);

    UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::OnRep_CurrentBall - CurrentBall: %s, bHasBall: %d"),
           CurrentBall ? *CurrentBall->GetName() : TEXT("None"), bHasBall);
}

bool AMF_PlayerCharacter::CanReceiveBall() const
{
    // Can receive ball if:
    // 1. Not already possessing a ball
    // 2. Not stunned
    // 3. Player state allows possession
    return !bHasBall && !IsStunned() && CurrentPlayerState != EMF_PlayerState::Shooting;
}

// ==================== Internal Functions ====================

void AMF_PlayerCharacter::SetupInputBindings()
{
    if (!InputHandler)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerCharacter::SetupInputBindings - No InputHandler"));
        return;
    }

    APlayerController *PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerCharacter::SetupInputBindings - No PlayerController"));
        return;
    }

    // Clear any existing delegate bindings to prevent double-binding on re-possession
    InputHandler->OnMoveInput.RemoveDynamic(this, &AMF_PlayerCharacter::OnMoveInputReceived);
    InputHandler->OnSprintInput.RemoveDynamic(this, &AMF_PlayerCharacter::OnSprintInputReceived);
    InputHandler->OnActionPressed.RemoveDynamic(this, &AMF_PlayerCharacter::OnActionPressed);
    InputHandler->OnActionReleased.RemoveDynamic(this, &AMF_PlayerCharacter::OnActionReleased);
    InputHandler->OnActionHeld.RemoveDynamic(this, &AMF_PlayerCharacter::OnActionHeld);
    InputHandler->OnSwitchPlayerInput.RemoveDynamic(this, &AMF_PlayerCharacter::OnSwitchPlayerInputReceived);
    InputHandler->OnPauseInput.RemoveDynamic(this, &AMF_PlayerCharacter::OnPauseInputReceived);

    // Initialize P_MEIS input
    if (InputHandler->InitializeInput(PC))
    {
        // Bind to input events
        InputHandler->OnMoveInput.AddDynamic(this, &AMF_PlayerCharacter::OnMoveInputReceived);
        InputHandler->OnSprintInput.AddDynamic(this, &AMF_PlayerCharacter::OnSprintInputReceived);
        InputHandler->OnActionPressed.AddDynamic(this, &AMF_PlayerCharacter::OnActionPressed);
        InputHandler->OnActionReleased.AddDynamic(this, &AMF_PlayerCharacter::OnActionReleased);
        InputHandler->OnActionHeld.AddDynamic(this, &AMF_PlayerCharacter::OnActionHeld);
        InputHandler->OnSwitchPlayerInput.AddDynamic(this, &AMF_PlayerCharacter::OnSwitchPlayerInputReceived);
        InputHandler->OnPauseInput.AddDynamic(this, &AMF_PlayerCharacter::OnPauseInputReceived);

        UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter: Input bindings setup complete"));
    }
}

void AMF_PlayerCharacter::UpdateMovement(float DeltaTime)
{
    // Movement input MUST be applied only on the owning client
    // CharacterMovementComponent handles replication to server
    if (!IsLocallyControlled())
    {
        return;
    }

    // Don't move if stunned
    if (IsStunned())
    {
        return;
    }

    // Check movement component is valid
    UCharacterMovementComponent *MovementComp = GetCharacterMovement();
    if (!MovementComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerCharacter::UpdateMovement - No CharacterMovementComponent!"));
        return;
    }

    // Apply movement
    if (!CurrentMoveInput.IsNearlyZero())
    {
        // Convert 2D input to 3D world direction
        // UE coordinate system: X = forward (red), Y = right (green), Z = up (blue)
        // Input: X = left/right (A/D), Y = forward/backward (W/S)
        // World: X = forward, Y = right
        FVector WorldDirection = FVector(CurrentMoveInput.Y, CurrentMoveInput.X, 0.0f);

        if (!WorldDirection.IsNearlyZero())
        {
            WorldDirection.Normalize();
        }

        // Add movement input - this adds to the pending input vector
        AddMovementInput(WorldDirection, 1.0f, false);

        // Update state
        if (HasAuthority())
        {
            EMF_PlayerState NewState = bIsSprinting ? EMF_PlayerState::Sprinting : EMF_PlayerState::Running;
            if (!bHasBall && CurrentPlayerState != NewState)
            {
                SetPlayerState(NewState);
            }
            else if (bHasBall && CurrentPlayerState != EMF_PlayerState::HasBall)
            {
                SetPlayerState(EMF_PlayerState::HasBall);
            }
        }
    }
    else if (HasAuthority() && CurrentPlayerState != EMF_PlayerState::Idle &&
             CurrentPlayerState != EMF_PlayerState::HasBall &&
             CurrentPlayerState != EMF_PlayerState::Stunned)
    {
        SetPlayerState(bHasBall ? EMF_PlayerState::HasBall : EMF_PlayerState::Idle);
    }
}

void AMF_PlayerCharacter::ApplyStun(float Duration)
{
    if (HasAuthority())
    {
        StunTimeRemaining = Duration;
        SetPlayerState(EMF_PlayerState::Stunned);

        // Stop movement
        if (UCharacterMovementComponent *Movement = GetCharacterMovement())
        {
            Movement->StopMovementImmediately();
        }
    }
}

void AMF_PlayerCharacter::ExecuteShoot(FVector Direction, float Power)
{
    if (!HasAuthority())
    {
        return;
    }

    if (!bHasBall || !CurrentBall)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerCharacter::ExecuteShoot - No ball to shoot"));
        return;
    }

    // Clamp power
    Power = FMath::Clamp(Power, 0.0f, MF_Constants::BallShootSpeed);

    UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::ExecuteShoot - Direction: %s, Power: %f"),
           *Direction.ToString(), Power);

    // Kick the ball (this clears possession internally)
    CurrentBall->Kick(Direction, Power, true);  // bAddHeight = true for shots

    SetPlayerState(EMF_PlayerState::Shooting);
}

void AMF_PlayerCharacter::ExecutePass(FVector Direction, float Power)
{
    if (!HasAuthority())
    {
        return;
    }

    if (!bHasBall || !CurrentBall)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerCharacter::ExecutePass - No ball to pass"));
        return;
    }

    // Clamp power
    Power = FMath::Clamp(Power, 0.0f, MF_Constants::BallPassSpeed);

    UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::ExecutePass - Direction: %s, Power: %f"),
           *Direction.ToString(), Power);

    // Kick the ball (this clears possession internally)
    CurrentBall->Kick(Direction, Power, false);  // bAddHeight = false for passes

    SetPlayerState(EMF_PlayerState::Passing);
}

void AMF_PlayerCharacter::ExecuteTackle()
{
    if (!HasAuthority())
    {
        return;
    }

    // Check cooldown
    if (TackleCooldownRemaining > 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::ExecuteTackle - On cooldown"));
        return;
    }

    // Start cooldown
    TackleCooldownRemaining = MF_Constants::TackleCooldown;
    SetPlayerState(EMF_PlayerState::Tackling);

    // Tackle range - distance within which we can steal the ball
    const float TackleRange = MF_Constants::TackleRange;
    FVector MyLocation = GetActorLocation();

    UE_LOG(LogTemp, Warning, TEXT("ExecuteTackle - Attacker: %s, MyTeam=%d, Searching within %.1f units"),
           *GetName(), (int32)TeamID, TackleRange);

    // Find nearest opponent with ball within tackle range
    AMF_PlayerCharacter* BestTarget = nullptr;
    float BestDistance = TackleRange;

    for (TActorIterator<AMF_PlayerCharacter> It(GetWorld()); It; ++It)
    {
        AMF_PlayerCharacter* Other = *It;
        if (!Other || Other == this)
            continue;

        float Distance = FVector::Dist(MyLocation, Other->GetActorLocation());
        
        // Check team - skip teammates (None team can tackle anyone)
        bool bIsTeammate = (TeamID != EMF_TeamID::None && Other->GetTeamID() == GetTeamID());
        
        UE_LOG(LogTemp, Log, TEXT("  Checking %s: Distance=%.1f, HasBall=%d, Team=%d, CurrentBall=%s, IsTeammate=%d"),
               *Other->GetName(), Distance, Other->HasBall(), (int32)Other->GetTeamID(),
               Other->CurrentBall ? TEXT("Valid") : TEXT("NULL"), bIsTeammate);

        if (bIsTeammate)
            continue;

        // Check if in range and has ball
        if (Distance <= BestDistance && Other->HasBall() && Other->CurrentBall)
        {
            BestTarget = Other;
            BestDistance = Distance;
        }
    }

    if (BestTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("ExecuteTackle - Stealing ball from %s (distance: %.1f)"),
               *BestTarget->GetName(), BestDistance);
        BestTarget->CurrentBall->SetPossessor(this);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("MF_PlayerCharacter::ExecuteTackle - No valid target found within range"));
}

// ==================== Input Callbacks ====================

void AMF_PlayerCharacter::OnMoveInputReceived(FVector2D MoveValue)
{
    ApplyMoveInput(MoveValue);
}

void AMF_PlayerCharacter::OnSprintInputReceived(bool bSprint)
{
    SetSprinting(bSprint);
}

void AMF_PlayerCharacter::OnActionPressed(bool bPressed)
{
    // Action pressed - context-sensitive
    // If we have ball: prepare to shoot/pass
    // If no ball and near opponent: tackle

    if (!bHasBall)
    {
        // Tackle if near opponent with ball
        if (IsLocallyControlled())
        {
            // Mark action as consumed by tackle - prevents shoot on release
            bActionConsumedByTackle = true;
            Server_RequestTackle();
        }
    }
    else
    {
        // Player has ball - action not consumed, will shoot/pass on release
        bActionConsumedByTackle = false;
    }
}

void AMF_PlayerCharacter::OnActionReleased()
{
    // Check if action was consumed by tackle - if so, skip shoot/pass
    if (bActionConsumedByTackle)
    {
        bActionConsumedByTackle = false;  // Reset for next press
        UE_LOG(LogTemp, Log, TEXT("OnActionReleased - Skipping shoot (action consumed by tackle)"));
        return;
    }

    if (bHasBall && InputHandler)
    {
        float HoldTime = InputHandler->GetActionHoldTime();

        // Short tap = shoot, hold = pass
        FVector Direction = GetActorForwardVector();

        if (HoldTime < 0.3f)
        {
            // Quick tap = shoot
            if (IsLocallyControlled())
            {
                Server_RequestShoot(Direction, MF_Constants::BallShootSpeed);
            }
        }
        else
        {
            // Held = pass (power based on hold time)
            float Power = FMath::Clamp(HoldTime * 1000.0f, MF_Constants::BallPassSpeed * 0.5f, MF_Constants::BallPassSpeed);
            if (IsLocallyControlled())
            {
                Server_RequestPass(Direction, Power);
            }
        }
    }
}

void AMF_PlayerCharacter::OnActionHeld(float HoldTime)
{
    // Could show power meter UI here
}

void AMF_PlayerCharacter::OnSwitchPlayerInputReceived()
{
    // Forward to PlayerController for character switching
    // Per PLAN.md: Q ALWAYS switches control to the teammate closest to the ball
    UE_LOG(LogTemp, Warning, TEXT("MF_PlayerCharacter::OnSwitchPlayerInputReceived called!"));
    
    if (AMF_PlayerController* MFC = Cast<AMF_PlayerController>(GetController()))
    {
        UE_LOG(LogTemp, Warning, TEXT("  → Calling SwitchToNearestToBall, TeamCharacters.Num: %d"), 
               MFC->GetRegisteredTeamCharacters().Num());
        MFC->SwitchToNearestToBall();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("  → No MF_PlayerController found!"));
    }
}

void AMF_PlayerCharacter::OnPauseInputReceived()
{
    // Forward to PlayerController for pause handling
    if (AMF_PlayerController* MFC = Cast<AMF_PlayerController>(GetController()))
    {
        MFC->RequestPause();
    }
}
