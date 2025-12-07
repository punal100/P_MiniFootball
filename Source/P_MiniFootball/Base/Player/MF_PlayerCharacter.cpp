/*
 * @Author: Punal Manalan
 * @Description: MF_PlayerCharacter - Implementation
 *               Full network replication for Listen Server and Dedicated Server
 * @Date: 07/12/2025
 */

#include "Player/MF_PlayerCharacter.h"
#include "Player/MF_InputHandler.h"
#include "Ball/MF_Ball.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Integration/CPP_EnhancedInputIntegration.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

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
    // Cleanup input
    if (InputHandler)
    {
        InputHandler->CleanupInput();
    }

    Super::UnPossessed();
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
    ExecuteShoot(Direction, Power);
}

bool AMF_PlayerCharacter::Server_RequestPass_Validate(FVector Direction, float Power)
{
    return Power >= 0.0f && Power <= MF_Constants::BallPassSpeed * 2.0f;
}

void AMF_PlayerCharacter::Server_RequestPass_Implementation(FVector Direction, float Power)
{
    ExecutePass(Direction, Power);
}

bool AMF_PlayerCharacter::Server_RequestTackle_Validate()
{
    return true;
}

void AMF_PlayerCharacter::Server_RequestTackle_Implementation()
{
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

    // Initialize P_MEIS input
    if (InputHandler->InitializeInput(PC))
    {
        // Bind to input events
        InputHandler->OnMoveInput.AddDynamic(this, &AMF_PlayerCharacter::OnMoveInputReceived);
        InputHandler->OnSprintInput.AddDynamic(this, &AMF_PlayerCharacter::OnSprintInputReceived);
        InputHandler->OnActionPressed.AddDynamic(this, &AMF_PlayerCharacter::OnActionPressed);
        InputHandler->OnActionReleased.AddDynamic(this, &AMF_PlayerCharacter::OnActionReleased);
        InputHandler->OnActionHeld.AddDynamic(this, &AMF_PlayerCharacter::OnActionHeld);

        UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter: Input bindings setup complete"));
    }
}

void AMF_PlayerCharacter::UpdateMovement(float DeltaTime)
{
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

    if (!bHasBall || !PossessedBall)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerCharacter::ExecuteShoot - No ball to shoot"));
        return;
    }

    // Clamp power
    Power = FMath::Clamp(Power, 0.0f, MF_Constants::BallShootSpeed);

    // Release ball
    // TODO: Call Ball->Kick() when ball system is implemented
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::ExecuteShoot - Direction: %s, Power: %f"),
           *Direction.ToString(), Power);

    SetPlayerState(EMF_PlayerState::Shooting);
}

void AMF_PlayerCharacter::ExecutePass(FVector Direction, float Power)
{
    if (!HasAuthority())
    {
        return;
    }

    if (!bHasBall || !PossessedBall)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerCharacter::ExecutePass - No ball to pass"));
        return;
    }

    // Clamp power
    Power = FMath::Clamp(Power, 0.0f, MF_Constants::BallPassSpeed);

    // TODO: Call Ball->Kick() when ball system is implemented
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::ExecutePass - Direction: %s, Power: %f"),
           *Direction.ToString(), Power);

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

    // TODO: Find nearby opponent and attempt to take ball
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::ExecuteTackle - Tackling"));
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
            Server_RequestTackle();
        }
    }
    // If has ball, wait for release to determine shoot vs pass
}

void AMF_PlayerCharacter::OnActionReleased()
{
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
