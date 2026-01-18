/*
 * @Author: Punal Manalan
 * @Description: MF_PlayerCharacter - Implementation
 *               Full network replication for Listen Server and Dedicated Server
 * @Date: 07/12/2025
 */

#include "Player/MF_PlayerCharacter.h"
#include "Player/MF_PlayerController.h"
#include "Player/MF_InputHandler.h"
#include "Player/MF_CharacterMovementComponent.h"
#include "Ball/MF_Ball.h"
#include "Match/MF_Goal.h"
#include "Match/MF_GameState.h"
#include "Net/UnrealNetwork.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Integration/CPP_EnhancedInputIntegration.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EngineUtils.h"
#include "AIComponent.h"
#include "AIBehaviour.h"
#include "EAISSubsystem.h"
#include "AI/MF_AIController.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "AI/MF_EAISActionExecutorComponent.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Interfaces/IPluginManager.h"
#include "NavigationInvokerComponent.h"

AMF_PlayerCharacter::AMF_PlayerCharacter(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UMF_CharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
    PrimaryActorTick.bCanEverTick = true;

    // Create the AI component
    AIComponent = CreateDefaultSubobject<UAIComponent>(TEXT("AIComponent"));

    // Configure default AI settings
    AIComponent->bAutoStart = false; // We control start timing in BeginPlay
    AIComponent->TickInterval = AITickInterval;

    // Create Action Executor
    AIActionExecutor = CreateDefaultSubobject<UMF_EAISActionExecutorComponent>(TEXT("AIActionExecutor"));

    // Create Navigation Invoker
    NavInvoker = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavInvoker"));
    NavInvoker->SetGenerationRadii(4000.0f, 5000.0f); // Cover significant area, with removal radius slightly larger

    // Configure AI Controller - ensures AI characters get AI controllers automatically
    AIControllerClass = AMF_AIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

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
    CameraBoom->SetRelativeRotation(FRotator(-60.0f, 180.0f, 0.0f)); // Look down at 60 degree angle
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

        // Network smoothing to reduce rubber banding on clients
        Movement->NetworkSmoothingMode = ENetworkSmoothingMode::Linear;
    }

    // Network settings for smooth replication
    bReplicates = true;
    SetReplicateMovement(true);
    SetNetUpdateFrequency(MF_Constants::NetUpdateFrequency);
    SetMinNetUpdateFrequency(MF_Constants::MinNetUpdateFrequency);

    // Create and configure Player Indicator
    PlayerIndicator = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PlayerIndicator"));
    PlayerIndicator->SetupAttachment(GetMesh());
    PlayerIndicator->SetRelativeLocation(FVector(0.0f, 0.0f, 210.0f)); // Position above head
    PlayerIndicator->SetHorizontalAlignment(EHTA_Center);
    PlayerIndicator->SetVerticalAlignment(EVRTA_TextBottom);
    PlayerIndicator->SetXScale(5.0f);
    PlayerIndicator->SetYScale(5.0f);
    PlayerIndicator->SetWorldRotation(FRotator(0.0f, 0.0f, 0.0f)); // Face forward by default
    PlayerIndicator->SetText(FText::FromString(AIProfile));
    
    // Ensure it's visible but doesn't cast shadows
    PlayerIndicator->SetCastShadow(false);
    PlayerIndicator->bIsEditorOnly = false;
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
    DOREPLIFETIME(AMF_PlayerCharacter, AIProfile);
}

void AMF_PlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Ensure Goalkeepers have an Actor Tag for reliable identification.
    // Prefer setting this tag in the Blueprint defaults; this is a safety net.
    static const FName GoalkeeperTag(TEXT("Goalkeeper"));
    if (!ActorHasTag(GoalkeeperTag) && AIProfile.Contains(TEXT("Goalkeeper")))
    {
        Tags.Add(GoalkeeperTag);
        UE_LOG(LogTemp, Warning, TEXT("[MF_PlayerCharacter] Auto-added Actor Tag '%s' to %s based on AIProfile. Prefer setting the tag in BP."),
               *GoalkeeperTag.ToString(), *GetName());
    }

    UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::BeginPlay - HasAuthority: %d, IsLocallyControlled: %d"),
           HasAuthority(), IsLocallyControlled());

    // Log spawn position and store for formation-based AI positioning
    SpawnLocation = GetActorLocation();
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::BeginPlay - Spawned at Location: %s"), *SpawnLocation.ToString());

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
                   CapsuleHalfHeight, SpawnLocation.Z - CapsuleHalfHeight);
        }
    }

    // AI Initialization
    if (HasAuthority())
    {
        // Configure AI component
        AIComponent->TickInterval = AITickInterval;
        AIComponent->bDebugMode = bDebugAI;

        // Load behavior from profile or asset
        if (AIBehaviour)
        {
            AIComponent->InitializeAI(AIBehaviour);
        }
        if (!AIProfile.IsEmpty())
        {
            // Resolve Path dynamically using IPluginManager (Correct for Packaged Builds + UFS Staging)
            // This relies on DirectoriesToAlwaysStageAsUFS pointing to the correct plugin content location
            if (TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("P_MiniFootball"))
            {
                 FString PluginDir = Plugin->GetContentDir();
                 FString AIProfileDir = FPaths::Combine(PluginDir, TEXT("AIProfiles"));

                 // Debug Logging for Packaged Build Verification
                 UE_LOG(LogTemp, Warning, TEXT("[MF_PlayerCharacter] === AI PROFILE LOADING DEBUG ==="));
                 UE_LOG(LogTemp, Warning, TEXT("[MF_PlayerCharacter] Plugin Content Dir: %s"), *PluginDir);
                 UE_LOG(LogTemp, Warning, TEXT("[MF_PlayerCharacter] Target AIProfiles Dir: %s"), *AIProfileDir);
                 
                 bool bDirExists = IFileManager::Get().DirectoryExists(*AIProfileDir);
                 UE_LOG(LogTemp, Warning, TEXT("[MF_PlayerCharacter] Directory Exists: %s"), bDirExists ? TEXT("TRUE") : TEXT("FALSE"));

                 if (bDirExists)
                 {
                     TArray<FString> Files;
                     IFileManager::Get().FindFiles(Files, *AIProfileDir, TEXT("*.json"));
                     UE_LOG(LogTemp, Warning, TEXT("[MF_PlayerCharacter] Found %d JSON files"), Files.Num());
                     for (const FString& F : Files)
                     {
                         UE_LOG(LogTemp, Warning, TEXT("[MF_PlayerCharacter]   - %s"), *F);
                     }
                 }
                 else
                 {
                     UE_LOG(LogTemp, Error, TEXT("[MF_PlayerCharacter] CRITICAL: AIProfiles directory missing in packaged build!"));
                 }
                 UE_LOG(LogTemp, Warning, TEXT("[MF_PlayerCharacter] ==================================="));

                 // Pass the profile name and the local plugin path to the AI Component
                 if (AIComponent)
                 {
                     AIComponent->StartAI(AIProfile, AIProfileDir);
                 }
            }
            else
            {
                 UE_LOG(LogTemp, Error, TEXT("[MF_PlayerCharacter] Failed to find P_MiniFootball plugin via IPluginManager!"));
            }
        }

        // Auto-start if enabled and not controlled by a human player
        AController *CurrentController = GetController();
        bool bIsHuman = CurrentController && (CurrentController->IsA<APlayerController>());

        if (bAutoStartAI && !bIsHuman)
        {
            UE_LOG(LogTemp, Warning, TEXT("[MF_PlayerCharacter] Auto-starting AI for %s with profile %s"), *GetName(), *AIProfile);
            StartAI();
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("[MF_PlayerCharacter] Auto-start AI deferred for %s (Human: %d)"), *GetName(), bIsHuman);
        }
    }

    // Initialize visual indicator
    UpdatePlayerIndicator();
}

void AMF_PlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update movement
    UpdateMovement(DeltaTime);

    // Sync game state to blackboard
    if (HasAuthority() && AIComponent && AIComponent->IsValid() && IsAIRunning())
    {
        SyncBlackboard();
        
        // AUTO SPRINT LOGIC REMOVED
        // We now control sprinting explicitly via AI Profiles (MF.Sprint action)
        // This prevents conflicts where the code forces walking when the AI wants to dribble/sprint.
    }

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

    // Dynamic Billboarding: Make indicator face the camera on non-dedicated servers
    if (PlayerIndicator && !IsNetMode(NM_DedicatedServer))
    {
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            if (APlayerCameraManager* CameraManager = PC->PlayerCameraManager)
            {
                FVector CameraLoc = CameraManager->GetCameraLocation();
                FVector IndicatorLoc = PlayerIndicator->GetComponentLocation();
                
                FRotator TargetRot = (CameraLoc - IndicatorLoc).Rotation();
                
                // For a more stable top-down feel, we can optionally zero out pitch/roll
                // but full rotation is usually safer for all camera angles.
                PlayerIndicator->SetWorldRotation(TargetRot);
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

    // Update AI blackboard with controller info
    if (AIComponent)
    {
        AIComponent->SetBlackboardObject(TEXT("Controller"), NewController);
    }

    // Stop AI when possessed by a human PlayerController
    if (Cast<APlayerController>(NewController))
    {
        if (AIComponent && AIComponent->IsRunning())
        {
            StopAI();
            UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter: AI stopped - human player took control of %s"), *GetName());
        }
    }

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

    // Only handle AI resumption on server
    if (!HasAuthority())
    {
        return;
    }

    // Spawn a new AI controller if we don't have one
    if (!GetController())
    {
        SpawnDefaultController();
    }

    // Restart AI behavior
    if (AIComponent && !AIComponent->IsRunning())
    {
        StartAI();
        UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter: AI resumed for %s after human unpossessed"), *GetName());
    }
}

void AMF_PlayerCharacter::OnRep_Owner()
{
    Super::OnRep_Owner();

    // This is called on clients when ownership/possession replicates
    // Check if we're now locally controlled and set up input
    AController *NewController = GetController();

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
            UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter[%s]::SetHasBall - New Value: %d"), *GetName(), bHasBall);
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
            UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter[%s]::SetPlayerState - New State: %d"), *GetName(), (int32)CurrentPlayerState);
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

        // Prediction-driven sprint: encode sprint intent into saved moves (owning client)
        // and apply it deterministically on both client/server via UMF_CharacterMovementComponent.
        if (UMF_CharacterMovementComponent *MFMove = Cast<UMF_CharacterMovementComponent>(GetCharacterMovement()))
        {
            MFMove->bWantsToSprint = bIsSprinting;
        }
        else if (UCharacterMovementComponent *Movement = GetCharacterMovement())
        {
            // Fallback (if a different movement component is used).
            Movement->MaxWalkSpeed = bIsSprinting ? MF_Constants::SprintSpeed : MF_Constants::WalkSpeed;
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

    // Sprint intent is handled by CharacterMovement prediction (compressed flags)
    // via UMF_CharacterMovementComponent. Keep legacy fallback only.
    if (!Cast<UMF_CharacterMovementComponent>(GetCharacterMovement()))
    {
        if (bIsSprinting != bSprinting)
        {
            bIsSprinting = bSprinting;
            if (UCharacterMovementComponent *Movement = GetCharacterMovement())
            {
                Movement->MaxWalkSpeed = bIsSprinting ? MF_Constants::SprintSpeed : MF_Constants::WalkSpeed;
            }
        }
    }
}

// ==================== Rep Notifies ====================

void AMF_PlayerCharacter::OnRep_TeamID()
{
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::OnRep_TeamID - Team: %d"), static_cast<int32>(TeamID));
    
    UpdatePlayerIndicator();
}

void AMF_PlayerCharacter::OnRep_HasBall()
{
    OnPossessionChanged.Broadcast(this, bHasBall);

    if (HasAuthority())
    {
        OnBallPossessionChanged();
    }

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

void AMF_PlayerCharacter::OnRep_AIProfile()
{
    UpdatePlayerIndicator();
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

void AMF_PlayerCharacter::UpdatePlayerIndicator()
{
    if (!PlayerIndicator)
    {
        return;
    }

    // Set Role Text
    PlayerIndicator->SetText(FText::FromString(AIProfile));

    // Set Team Color
    FColor TeamColor = FColor::White;
    if (TeamID == EMF_TeamID::TeamA)
    {
        TeamColor = FColor(51, 153, 255); // Team A Blue (approx 0.2, 0.6, 1.0)
    }
    else if (TeamID == EMF_TeamID::TeamB)
    {
        TeamColor = FColor(255, 77, 77);   // Team B Red (approx 1.0, 0.3, 0.3)
    }

    PlayerIndicator->SetTextRenderColor(TeamColor);
    
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter::UpdatePlayerIndicator - Updated for %s: Role=%s, Team=%d"), 
        *GetName(), *AIProfile, (int32)TeamID);
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
        // World: X = backward, Y = left
        FVector WorldDirection = FVector(-CurrentMoveInput.Y, -CurrentMoveInput.X, 0.0f);
        // Relative to camera
        //FRotator CameraRotation = CameraBoom ? CameraBoom->GetComponentRotation() : FRotator::ZeroRotator;
        //FRotator YawRotation(0, CameraRotation.Yaw, 0);
        //FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        //FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        //FVector WorldDirection = (ForwardVector * CurrentMoveInput.Y) + (RightVector * CurrentMoveInput.X);

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
    CurrentBall->Kick(Direction, Power, true); // bAddHeight = true for shots

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
    CurrentBall->Kick(Direction, Power, false); // bAddHeight = false for passes

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
    AMF_PlayerCharacter *BestTarget = nullptr;
    float BestDistance = TackleRange;

    static const FName GoalkeeperTag(TEXT("Goalkeeper"));

    for (TActorIterator<AMF_PlayerCharacter> It(GetWorld()); It; ++It)
    {
        AMF_PlayerCharacter *Other = *It;
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

        // Goalkeeper Immunity: Goalkeepers can NEVER be tackled.
        if (Other->ActorHasTag(GoalkeeperTag))
        {
            UE_LOG(LogTemp, Log, TEXT("  Target is Goalkeeper (Actor Tag) - Immunity applied"));
            continue;
        }

        // Check if in range and has ball
        if (Distance <= BestDistance && Other->HasBall() && Other->CurrentBall)
        {
            // ==================== FACING CHECK LOGIC ====================
            // Facing check is BYPASSED only for Goalkeepers inside their own penalty area.
            // All other players (Strikers, Midfielders, Defenders) MUST face the ball carrier to tackle.
            
            bool bBypassFacingCheck = false;
            
            // Check if THIS player (the tackler) is a GK inside their own penalty area
            if (ActorHasTag(GoalkeeperTag))
            {
                // Calculate GK's own penalty box position
                // TeamA defends at negative Y, TeamB defends at positive Y
                float MyGoalLineY = (MF_Constants::FieldLength / 2.0f) * ((TeamID == EMF_TeamID::TeamA) ? -1.0f : 1.0f);
                float MyDistY = FMath::Abs(MyLocation.Y - MyGoalLineY);
                float MyDistX = FMath::Abs(MyLocation.X);
                
                // Penalty Area: 16.5m (1650cm) deep, 40.3m (4030cm) wide → half-width = 2015cm
                if (MyDistY < MF_Constants::PenaltyAreaLength && MyDistX < (MF_Constants::PenaltyAreaWidth / 2.0f))
                {
                    bBypassFacingCheck = true;
                    UE_LOG(LogTemp, Log, TEXT("  GK in own penalty box - facing check bypassed"));
                }
            }
            
            if (bBypassFacingCheck)
            {
                // GK in own penalty area - can tackle from any direction
                BestTarget = Other;
                BestDistance = Distance;
            }
            else
            {
                // All other cases: Must be facing the ball carrier
                FVector ToTarget = (Other->GetActorLocation() - MyLocation).GetSafeNormal();
                FVector MyForward = GetActorForwardVector();
                float FacingDot = FVector::DotProduct(MyForward, ToTarget);
                
                if (FacingDot >= MF_Constants::TackleFacingMinDot)
                {
                    BestTarget = Other;
                    BestDistance = Distance;
                }
                else
                {
                    UE_LOG(LogTemp, Log, TEXT("  Tackle failed: Not facing target (Dot: %.2f, Required: %.2f)"), 
                           FacingDot, MF_Constants::TackleFacingMinDot);
                }
            }
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
        bActionConsumedByTackle = false; // Reset for next press
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

    if (AMF_PlayerController *MFC = Cast<AMF_PlayerController>(GetController()))
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
    if (AMF_PlayerController *MFC = Cast<AMF_PlayerController>(GetController()))
    {
        MFC->RequestPause();
    }
}

// ==================== AI Implementation ====================

void AMF_PlayerCharacter::StartAI()
{
    if (AIComponent)
    {
        AIComponent->StartAI();

        if (bDebugAI)
        {
            UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter: Started AI with profile '%s'"), *AIProfile);
        }

        // Initial blackboard sync
        SyncBlackboard();
    }
}

bool AMF_PlayerCharacter::EAIS_GetTargetLocation_Implementation(FName TargetId, FVector &OutLocation) const
{
    AActor *TargetActor = nullptr;
    if (EAIS_GetTargetActor_Implementation(TargetId, TargetActor))
    {
        if (TargetActor)
        {
            OutLocation = TargetActor->GetActorLocation();
            return true;
        }
    }

    // Intercept point for the current ball carrier.
    // Helps defenders/midfielders/strikers get in front instead of trailing behind.
    if (TargetId == "BallCarrierIntercept")
    {
        AActor* BallCarrierActor = nullptr;
        if (EAIS_GetTargetActor_Implementation(TEXT("BallCarrier"), BallCarrierActor) && BallCarrierActor)
        {
            const FVector CarrierLoc = BallCarrierActor->GetActorLocation();
            FVector Dir = BallCarrierActor->GetVelocity();
            Dir.Z = 0.0f;

            constexpr float MinVelForLead = 80.0f;
            if (Dir.SizeSquared() < FMath::Square(MinVelForLead))
            {
                Dir = BallCarrierActor->GetActorForwardVector();
                Dir.Z = 0.0f;
            }

            Dir = Dir.GetSafeNormal();
            if (Dir.IsNearlyZero())
            {
                OutLocation = CarrierLoc;
                return true;
            }

            constexpr float LeadDistance = 350.0f; // cm
            OutLocation = CarrierLoc + (Dir * LeadDistance);
            return true;
        }

        return false;
    }

    // Handle coordinate targets (if any)
    if (TargetId == "Home")
    {
        // Return spawn or defensive position
        OutLocation = SpawnLocation;
        return true;
    }
    
    // Explicitly handle SupportPosition to prevent resolution warnings
    if (TargetId == "SupportPosition")
    {
        // SupportPosition is always maintained in the blackboard by SyncBlackboard
        OutLocation = AIComponent->GetBlackboardVector(TEXT("SupportPosition"));
        return true;
    }

    if (TargetId == "GK_TargetPosition")
    {
        OutLocation = AIComponent->GetBlackboardVector(TEXT("GK_TargetPosition"));
        return !OutLocation.IsZero();
    }

    return false;
}

bool AMF_PlayerCharacter::EAIS_GetTargetActor_Implementation(FName TargetId, AActor *&OutActor) const

{
    if (TargetId == "Ball")
    {
        if (CurrentBall)
        {
            OutActor = CurrentBall;
            return true;
        }

        // Find ball in world
        for (TActorIterator<AMF_Ball> It(GetWorld()); It; ++It)
        {
            OutActor = *It;
            return true;
        }
    }

    if (TargetId == "Goal_Opponent" || TargetId == "Goal_Self")
    {
        bool bOpponent = (TargetId == "Goal_Opponent");
        for (TActorIterator<AMF_Goal> It(GetWorld()); It; ++It)
        {
            AMF_Goal *Goal = *It;
            // GoalTeam is the team that scores.
            // So if I'm TeamA, Goal_Opponent is the one where TeamA scores.
            if (bOpponent)
            {
                if (Goal->DefendingTeam != TeamID && Goal->DefendingTeam != EMF_TeamID::None)
                {
                    OutActor = Goal;
                    return true;
                }
            }
            else
            {
                if (Goal->DefendingTeam == TeamID)
                {
                    OutActor = Goal;
                    return true;
                }
            }
        }
    }

    if (TargetId == "BallCarrier")
    {
        for (TActorIterator<AMF_PlayerCharacter> It(GetWorld()); It; ++It)
        {
            if (It->HasBall())
            {
                OutActor = *It;
                return true;
            }
        }
    }

    if (TargetId == "NearestOpponent")
    {
        float NearestDist = 99999.0f;
        AMF_PlayerCharacter *Nearest = nullptr;

        for (TActorIterator<AMF_PlayerCharacter> It(GetWorld()); It; ++It)
        {
            AMF_PlayerCharacter *Other = *It;
            if (Other && Other != this && Other->GetTeamID() != TeamID)
            {
                const float Dist = FVector::Dist(GetActorLocation(), Other->GetActorLocation());
                if (Dist < NearestDist)
                {
                    NearestDist = Dist;
                    Nearest = Other;
                }
            }
        }

        if (Nearest)
        {
            OutActor = Nearest;
            return true;
        }
    }

    if (TargetId == "Striker")
    {
        // Find a teammate with the Striker role
        float NearestDist = 99999.0f;
        AMF_PlayerCharacter* BestStriker = nullptr;

        for (TActorIterator<AMF_PlayerCharacter> It(GetWorld()); It; ++It)
        {
            AMF_PlayerCharacter* Teammate = *It;
            if (Teammate && Teammate != this && Teammate->GetTeamID() == TeamID)
            {
                // Check if profile name contains "Striker"
                if (Teammate->AIProfile.Contains(TEXT("Striker")))
                {
                     const float Dist = FVector::Dist(GetActorLocation(), Teammate->GetActorLocation());
                     if (Dist < NearestDist)
                     {
                         NearestDist = Dist;
                         BestStriker = Teammate;
                     }
                }
            }
        }

        if (BestStriker)
        {
            OutActor = BestStriker;
            return true;
        }
    }

    if (TargetId == "Midfielder")
    {
        // Find a teammate with the Midfielder role
        float NearestDist = 99999.0f;
        AMF_PlayerCharacter* BestMid = nullptr;

        for (TActorIterator<AMF_PlayerCharacter> It(GetWorld()); It; ++It)
        {
            AMF_PlayerCharacter* Teammate = *It;
            if (Teammate && Teammate != this && Teammate->GetTeamID() == TeamID)
            {
                if (Teammate->AIProfile.Contains(TEXT("Midfielder")))
                {
                     const float Dist = FVector::Dist(GetActorLocation(), Teammate->GetActorLocation());
                     if (Dist < NearestDist)
                     {
                         NearestDist = Dist;
                         BestMid = Teammate;
                     }
                }
            }
        }

        if (BestMid)
        {
            OutActor = BestMid;
            return true;
        }
    }

    return false;
}

void AMF_PlayerCharacter::StopAI()
{
    if (AIComponent)
    {
        AIComponent->StopAI();
    }
}

void AMF_PlayerCharacter::ResetAI()
{
    if (AIComponent)
    {
        AIComponent->ResetAI();
    }
}

bool AMF_PlayerCharacter::SetAIProfile(const FString &ProfileName)
{
    if (!AIComponent)
    {
        return false;
    }

    // Stop current AI
    AIComponent->StopAI();

    // Load new profile
    FString ProfilePath = ProfileName;
    if (!ProfilePath.EndsWith(TEXT(".json")))
    {
        ProfilePath += TEXT(".json");
    }

    // Resolve Path dynamically from this plugin content dir
    // IMPORTANT: StartAI() requires the directory argument if the ProfileName is relative
    // Same logic as in BeginPlay()
    FString PluginDir = IPluginManager::Get().FindPlugin("P_MiniFootball")->GetContentDir();
    FString AIProfileDir = FPaths::Combine(PluginDir, TEXT("AIProfiles"));

    AIComponent->JsonFilePath = ProfilePath;
    AIComponent->ResetAI();

    AIProfile = ProfileName;

    // Restart if was running (using the directory!)
    if (bAutoStartAI)
    {
        AIComponent->StartAI(AIProfile, AIProfileDir);
    }

    return true;
}

void AMF_PlayerCharacter::InjectAIEvent(const FString &EventName)
{
    if (AIComponent)
    {
        AIComponent->EnqueueSimpleEvent(EventName);
    }
}

bool AMF_PlayerCharacter::IsAIRunning() const
{
    return AIComponent && AIComponent->IsRunning();
}

FString AMF_PlayerCharacter::GetCurrentAIState() const
{
    if (AIComponent)
    {
        return AIComponent->GetCurrentState();
    }
    return TEXT("");
}

void AMF_PlayerCharacter::SyncBlackboard()
{
    if (!AIComponent)
    {
        return;
    }

    const FVector MyLocation = GetActorLocation();

    // ==================== MATCH PHASE AWARENESS ====================
    bool bMatchIsPlaying = false;
    if (const AMF_GameState* GS = GetWorld()->GetGameState<AMF_GameState>())
    {
        bMatchIsPlaying = (GS->CurrentPhase == EMF_MatchPhase::Playing);
    }
    AIComponent->SetBlackboardBool(TEXT("MatchIsPlaying"), bMatchIsPlaying);

    // ==================== BASIC STATE ====================
    AIComponent->SetBlackboardBool(TEXT("HasBall"), HasBall());
    AIComponent->SetBlackboardBool(TEXT("IsStunned"), IsStunned());
    AIComponent->SetBlackboardBool(TEXT("IsSprinting"), bIsSprinting);
    AIComponent->SetBlackboardVector(TEXT("MyPosition"), MyLocation);
    AIComponent->SetBlackboardFloat(TEXT("TeamID"), static_cast<float>(GetTeamID()));

    // Log sync occasionally
    static float LastSyncLogTime = 0.0f;
    if (GetWorld()->GetTimeSeconds() - LastSyncLogTime > 2.0f)
    {
        //UE_LOG(LogTemp, Log, TEXT("MF_PlayerCharacter[%s]::Sync - HasBall: %d, Pos: %s"), *GetName(), HasBall(), *MyLocation.ToString());
        LastSyncLogTime = GetWorld()->GetTimeSeconds();
    }

    // ==================== BALL DATA ====================
    FVector BallPos = FVector::ZeroVector;
    AMF_Ball *MatchBall = nullptr;
    bool bBallFound = false;
    bool bIsBallOutOfBounds = false;

    // First check via target provider
    if (IEAIS_TargetProvider::Execute_EAIS_GetTargetLocation(this, TEXT("Ball"), BallPos))
    {
        bBallFound = true;
    }

    // Get ball actor for possession check
    for (TActorIterator<AMF_Ball> It(GetWorld()); It; ++It)
    {
        AMF_Ball* Ball = *It;
        if (Ball)
        {
            MatchBall = Ball;
            if (!bBallFound)
            {
                BallPos = Ball->GetActorLocation();
                bBallFound = true;
            }
            bIsBallOutOfBounds = Ball->IsOutOfBounds();
            break;
        }
    }

    AIComponent->SetBlackboardBool(TEXT("IsBallInPlay"), bBallFound && !bIsBallOutOfBounds && bMatchIsPlaying);

    if (bBallFound)
    {
        AIComponent->SetBlackboardVector(TEXT("Ball"), BallPos);
        const float DistToBall = FVector::Dist(MyLocation, BallPos);
        AIComponent->SetBlackboardFloat(TEXT("DistToBall"), DistToBall);
    }
    else
    {
        AIComponent->SetBlackboardFloat(TEXT("DistToBall"), 99999.0f);
    }

    // ==================== GOAL DATA ====================
    FVector GoalPos = FVector::ZeroVector;
    if (IEAIS_TargetProvider::Execute_EAIS_GetTargetLocation(this, TEXT("Goal_Opponent"), GoalPos))
    {
        AIComponent->SetBlackboardVector(TEXT("Goal_Opponent"), GoalPos);
        const float DistToGoal = FVector::Dist(MyLocation, GoalPos);
        AIComponent->SetBlackboardFloat(TEXT("DistToOpponentGoal"), DistToGoal);
    }
    else
    {
        AIComponent->SetBlackboardFloat(TEXT("DistToOpponentGoal"), 99999.0f);
    }

    // ==================== HOME/FORMATION DATA ====================
    // TODO: Get actual formation position from GameState
    const FVector HomePos = GetActorLocation(); // Placeholder
    AIComponent->SetBlackboardVector(TEXT("Home"), HomePos);
    AIComponent->SetBlackboardFloat(TEXT("DistToHome"), 0.0f); // Will be actual distance when formation implemented

    // ==================== POSSESSION STATE ====================
    bool bTeamHasBall = false;
    bool bOpponentHasBall = false;
    bool bBallLoose = true;
    AMF_PlayerCharacter *BallCarrier = nullptr;

    // Check all players for ball possession
    for (TActorIterator<AMF_PlayerCharacter> It(GetWorld()); It; ++It)
    {
        AMF_PlayerCharacter *OtherPlayer = *It;
        if (OtherPlayer && OtherPlayer->HasBall())
        {
            bBallLoose = false;
            BallCarrier = OtherPlayer;

            if (OtherPlayer->GetTeamID() == TeamID)
            {
                bTeamHasBall = true;
            }
            else
            {
                bOpponentHasBall = true;
            }
            break;
        }
    }

    AIComponent->SetBlackboardBool(TEXT("TeamHasBall"), bTeamHasBall);
    AIComponent->SetBlackboardBool(TEXT("OpponentHasBall"), bOpponentHasBall);
    AIComponent->SetBlackboardBool(TEXT("IsBallLoose"), bBallLoose);

    // ==================== NEAREST OPPONENT ====================
    float NearestOpponentDist = 99999.0f;
    AMF_PlayerCharacter *NearestOpponent = nullptr;

    for (TActorIterator<AMF_PlayerCharacter> It(GetWorld()); It; ++It)
    {
        AMF_PlayerCharacter *OtherPlayer = *It;
        if (OtherPlayer && OtherPlayer != this && OtherPlayer->GetTeamID() != TeamID)
        {
            const float Dist = FVector::Dist(MyLocation, OtherPlayer->GetActorLocation());
            if (Dist < NearestOpponentDist)
            {
                NearestOpponentDist = Dist;
                NearestOpponent = OtherPlayer;
            }
        }
    }

    AIComponent->SetBlackboardFloat(TEXT("DistToNearestOpponent"), NearestOpponentDist);
    if (NearestOpponent)
    {
        AIComponent->SetBlackboardVector(TEXT("NearestOpponentPosition"), NearestOpponent->GetActorLocation());
    }

    // ==================== DANGER DETECTION ====================
    // IsInDanger: True if opponent is within tackle range (~200 units)
    constexpr float DangerRadius = 200.0f;
    const bool bIsInDanger = NearestOpponentDist < DangerRadius;
    AIComponent->SetBlackboardBool(TEXT("IsInDanger"), bIsInDanger);

    // ==================== STRIKER TARGETING ====================
    // Used by Midfielder AI to decide when a striker pass is viable.
    bool bHasStriker = false;
    float DistToStriker = 99999.0f;
    FVector StrikerPos = FVector::ZeroVector;

    for (TActorIterator<AMF_PlayerCharacter> It(GetWorld()); It; ++It)
    {
        AMF_PlayerCharacter* Teammate = *It;
        if (!Teammate || Teammate == this || Teammate->GetTeamID() != TeamID)
        {
            continue;
        }

        if (!Teammate->AIProfile.Contains(TEXT("Striker")))
        {
            continue;
        }

        const float Dist = FVector::Dist(MyLocation, Teammate->GetActorLocation());
        if (Dist < DistToStriker)
        {
            DistToStriker = Dist;
            StrikerPos = Teammate->GetActorLocation();
            bHasStriker = true;
        }
    }

    AIComponent->SetBlackboardBool(TEXT("HasStriker"), bHasStriker);
    AIComponent->SetBlackboardFloat(TEXT("DistToStriker"), DistToStriker);
    if (bHasStriker)
    {
        AIComponent->SetBlackboardVector(TEXT("StrikerPosition"), StrikerPos);
    }

    // ==================== CLEAR SHOT CHECK ====================
    // HasClearShot: True if no enemies are directly between player and goal
    bool bHasClearShot = false;
    if (HasBall() && GoalPos != FVector::ZeroVector)
    {
        bHasClearShot = true; // Assume clear by default

        const FVector ToGoal = (GoalPos - MyLocation).GetSafeNormal();
        const float GoalDist = FVector::Dist(MyLocation, GoalPos);

        for (TActorIterator<AMF_PlayerCharacter> It(GetWorld()); It; ++It)
        {
            AMF_PlayerCharacter *OtherPlayer = *It;
            if (OtherPlayer && OtherPlayer != this && OtherPlayer->GetTeamID() != TeamID)
            {
                const FVector ToEnemy = OtherPlayer->GetActorLocation() - MyLocation;
                const float EnemyDist = ToEnemy.Size();

                // Only check enemies between us and the goal
                if (EnemyDist < GoalDist)
                {
                    // Check if enemy is in the "cone" towards the goal
                    const float DotProduct = FVector::DotProduct(ToGoal, ToEnemy.GetSafeNormal());
                    if (DotProduct > 0.85f) // Within ~30 degree cone
                    {
                        bHasClearShot = false;
                        break;
                    }
                }
            }
        }
    }
    AIComponent->SetBlackboardBool(TEXT("HasClearShot"), bHasClearShot);

    // ==================== CLOSEST TO BALL DETECTION (CRITICAL FIX) ====================
    // Only the closest teammate to the ball should chase it - prevents all AI mobbing the ball
    bool bAmIClosestToBall = true; // Assume true until proven otherwise
    float MyDistToBall = 99999.0f;

    if (bBallFound)
    {
        // Calculate My Effective Distance
        float MyEffectiveDist = FVector::Dist(MyLocation, BallPos);
        
        // PRIORITY ADJUSTMENTS:
        // Strikers are "closer" by virtue of role (more aggressive)
        if (AIProfile.Contains(TEXT("Striker")))
        {
            MyEffectiveDist *= 0.85f; // 15% bonus
        }
        else if (AIProfile.Contains(TEXT("Goalkeeper")))
        {
            // If ball is in penalty box, GK is HYPER aggressive
            // Penalty box is ~16m (1650 units) deep
            float GoalLineY = (MF_Constants::FieldLength / 2.0f) * ((TeamID == EMF_TeamID::TeamA) ? -1.0f : 1.0f);
            if (FMath::Abs(BallPos.Y - GoalLineY) < 1650.0f && FMath::Abs(BallPos.X) < 2015.0f)
            {
                 MyEffectiveDist *= 0.1f; // Massive priority
            }
            else
            {
                 MyEffectiveDist *= 2.0f; // Stay in goal otherwise
            }
        }
        
        MyDistToBall = FVector::Dist(MyLocation, BallPos); // Actual distance for blackboard
        
        // Check teammates
        for (TActorIterator<AMF_PlayerCharacter> ClosestIt(GetWorld()); ClosestIt; ++ClosestIt)
        {
            AMF_PlayerCharacter* OtherPlayer = *ClosestIt;
            if (!OtherPlayer || OtherPlayer == this || OtherPlayer->GetTeamID() != TeamID)
                continue;

            float TheirEffectiveDist = FVector::Dist(OtherPlayer->GetActorLocation(), BallPos);
            
            // Apply same weighting logic to teammates
            if (OtherPlayer->AIProfile.Contains(TEXT("Striker")))
            {
                TheirEffectiveDist *= 0.85f;
            }
            else if (OtherPlayer->AIProfile.Contains(TEXT("Goalkeeper")))
            {
                 float GoalLineY = (MF_Constants::FieldLength / 2.0f) * ((TeamID == EMF_TeamID::TeamA) ? -1.0f : 1.0f);
                 if (FMath::Abs(BallPos.Y - GoalLineY) < 1650.0f && FMath::Abs(BallPos.X) < 2015.0f)
                 {
                      TheirEffectiveDist *= 0.1f;
                 }
                 else
                 {
                      TheirEffectiveDist *= 2.0f;
                 }
            }

            if (TheirEffectiveDist < MyEffectiveDist - 50.0f) 
            {
                bAmIClosestToBall = false;
                break;
            }
        }
    }

    AIComponent->SetBlackboardBool(TEXT("AmIClosestToBall"), bAmIClosestToBall);
    AIComponent->SetBlackboardFloat(TEXT("DistToBall"), MyDistToBall);

    // ==================== ROLE-BASED DATA ====================
    AIComponent->SetBlackboardValue(TEXT("Role"), FBlackboardValue(FString(AIProfile)));

    // ==================== FORMATION POSITION ====================
    // Use stored spawn position as home position for formation-based AI
    AIComponent->SetBlackboardVector(TEXT("Home"), SpawnLocation);
    const float DistToHome = FVector::Dist(MyLocation, SpawnLocation);
    AIComponent->SetBlackboardFloat(TEXT("DistToHome"), DistToHome);

    // ==================== SUPPORT POSITION ====================
    // Calculate intelligent support position based on ball and role
    const FVector SupportPos = CalculateSupportPosition(BallPos, TeamID);
    AIComponent->SetBlackboardVector(TEXT("SupportPosition"), SupportPos);
    
    // Add distance for logic checks
    const float DistToSupport = FVector::Dist(MyLocation, SupportPos);
    AIComponent->SetBlackboardFloat(TEXT("DistToSupportPosition"), DistToSupport);

    // ==================== GOALKEEPER TARGET DAMPING ====================
    // Goalkeepers can jitter/circle if their MoveTo target changes every tick.
    // Provide a cached/thresholded target that changes less often.
    // NOTE: Reduced thresholds (150->50cm, 0.25->0.08s) for more responsive GK movement
    if (AIProfile.Contains(TEXT("Goalkeeper")))
    {
        constexpr float MinTargetMoveDist = 50.0f;    // cm (was 150.0f - too sluggish)
        constexpr float MinTargetUpdateInterval = 0.08f; // seconds (was 0.25f - too slow)

        const float Now = GetWorld()->GetTimeSeconds();
        const bool bHasCached = !CachedGKTargetPosition.IsZero();
        const bool bEnoughTime = (Now - LastGKTargetUpdateTime) >= MinTargetUpdateInterval;
        const bool bFarEnough = !bHasCached || FVector::DistSquared(CachedGKTargetPosition, SupportPos) >= FMath::Square(MinTargetMoveDist);

        if (!bHasCached || (bEnoughTime && bFarEnough))
        {
            CachedGKTargetPosition = SupportPos;
            LastGKTargetUpdateTime = Now;
        }

        AIComponent->SetBlackboardVector(TEXT("GK_TargetPosition"), CachedGKTargetPosition);
    }
}

void AMF_PlayerCharacter::OnBallPossessionChanged()
{
    if (AIComponent)
    {
        // Inject event for AI state transition
        if (HasBall())
        {
            AIComponent->EnqueueSimpleEvent(TEXT("GotBall"));
        }
        else
        {
            AIComponent->EnqueueSimpleEvent(TEXT("LostBall"));
        }
    }
}

FVector AMF_PlayerCharacter::CalculateSupportPosition(const FVector& BallPosition, EMF_TeamID MyTeam) const
{
    // Calculate a supporting position based on ball location and role
    const float OffsetFromBall = 500.0f; // 5 meters
    const float SideOffset = 300.0f;     // 3 meters to the side

    // Get vital game state info
    AMF_GameState* GS = GetWorld()->GetGameState<AMF_GameState>();
    
    // Default to strict support
    FVector SupportPos = BallPosition;
    
    // Determine attack direction based on team
    // TeamA (at +Y) attacks Negative Y (Goal is at -5250)
    // TeamB (at -Y) attacks Positive Y (Goal is at +5250)
    float AttackDirection = (MyTeam == EMF_TeamID::TeamA) ? -1.0f : 1.0f;

    // Check if my team has the ball (Attacking vs Defending)
    bool bMyTeamHasBall = true; // Default to attacking logic if unsure
    if (GS)
    {
        bMyTeamHasBall = GS->TeamHasBall(MyTeam);
    }
    
    // Get current location
    FVector MyLocation = GetActorLocation();

    // ==================== BALL CARRIER LOGIC ====================
    // If I HAVE the ball, my "SupportPosition" should be a safe dribbling target
    if (HasBall())
    {
        // Target: Center of opponent goal line
        float GoalY = (MF_Constants::FieldLength / 2.0f) * AttackDirection;
        FVector GoalPos(0.0f, GoalY, MF_Constants::GroundZ);
        
        // Point 10 meters ahead towards goal
        FVector DirToGoal = (GoalPos - MyLocation).GetSafeNormal();
        FVector DribblePos = MyLocation + (DirToGoal * 1000.0f);
        
        // Bias towards center: if we are near sidelines, push X towards 0
        if (FMath::Abs(DribblePos.X) > 2000.0f)
        {
            DribblePos.X *= 0.7f;
        }
        
        return DribblePos;
    }
    
    // Helper to calculate "Home" position effectively
    // Use SpawnLocation, but ensure it's not ZeroVector (fallback to calculated formation)
    FVector EffectiveHome = (SpawnLocation.IsNearlyZero()) ? MyLocation : SpawnLocation;

    // ==================== STRIKER LOGIC ====================
    if (AIProfile.Contains(TEXT("Striker")))
    {
        if (bMyTeamHasBall)
        {
            // ATTACKING: Get in position to score
            // Position ahead of the ball towards opponent goal (Along Y)
            SupportPos.Y += 1000.0f * AttackDirection;
            
            // Unclump logic: Use PlayerID to spread out circle-wise or line-wise
            // Max separation: 1200 units width
            // Map PlayerID (0-255) to a -1.0 to 1.0 range based on modulo to ensure distinct slots
            // Slot 0: Center, Slot 1: Right, Slot 2: Left, etc.
            int32 Slot = PlayerID % 3; 
            float SpreadFactor = 0.0f;
            if (Slot == 1) SpreadFactor = 1.0f;
            else if (Slot == 2) SpreadFactor = -1.0f;

            SupportPos.X += SpreadFactor * 600.0f; 
        }
        else
        {
            // DEFENDING: Stay high up field (cherry pick) or press if close
            // Don't drop back too far. Stay near center circle or opponents defensive third.
            // Target: Stay high up field (Cherry Pick) near opponent defenders
            float TargetY = 2500.0f * AttackDirection; // Keep pressure high in opponent half
            SupportPos = FVector(EffectiveHome.X, TargetY, MF_Constants::GroundZ);
            
            // If ball is very close, press it (this is handled by AmIClosestToBall, but position ref helps)
            if (FVector::Dist(MyLocation, BallPosition) < 1500.0f)
            {
               SupportPos = BallPosition; 
            }
        }
    }
    // ==================== MIDFIELDER LOGIC ====================
    else if (AIProfile.Contains(TEXT("Midfielder")))
    {
        if (bMyTeamHasBall)
        {
            // ATTACKING: Support the striker, stay behind ball
            // Triangle formation support
            float Spread = 900.0f;
            SupportPos.Y -= 600.0f * AttackDirection; // Behind ball
            
            // Spread based on ID
            int32 Slot = PlayerID % 2;
            float SpreadDir = (Slot == 0) ? 1.0f : -1.0f;
            SupportPos.X = BallPosition.X + (Spread * SpreadDir);
        }
        else
        {
            // DEFENDING: Defensive Screen
            // Position between Ball and Our Goal (Screening)
            // Goal Y is roughly +/- 5250
            float MyGoalY = (MF_Constants::FieldLength / 2.0f) * -AttackDirection; 
            FVector MyGoalPos(0.0f, MyGoalY, 0.0f);
            
            // Block path to goal at 20% mark (closer to ball to allow pressing)
            SupportPos = FMath::Lerp(BallPosition, MyGoalPos, 0.2f);
            
            // Add some width based on ID to cover passing lanes
            float Spread = 400.0f; // Tighter while defending
            int32 Slot = PlayerID % 2;
             float SpreadDir = (Slot == 0) ? 1.0f : -1.0f;
            SupportPos.X += Spread * SpreadDir;
        }
    }
    // ==================== DEFENDER LOGIC ====================
    else if (AIProfile.Contains(TEXT("Defender")))
    {
        // Defenders: Anchor to HOME (SpawnLocation/Defensive Third)
        // Adjust engagement based on threat level
        
        // Calculate threat zone: How close is ball to our goal?
        float MyGoalY = (MF_Constants::FieldLength / 2.0f) * -AttackDirection;
        float DistBallToGoal = FMath::Abs(BallPosition.Y - MyGoalY);
        float FieldLen = MF_Constants::FieldLength;
        
        // ThreatRatio: 0.0 (Far) to 1.0 (In Goal Mouth)
        float ThreatRatio = 1.0f - FMath::Clamp(DistBallToGoal / (FieldLen * 0.6f), 0.0f, 1.0f);
        
        if (bMyTeamHasBall)
        {
            // ATTACKING: Move up to half-way line or maintain structure
            // Stay largely at EffectiveHome but shift Y up slightly
            SupportPos = EffectiveHome;
            SupportPos.Y -= 500.0f * AttackDirection; // Shift up slightly
        }
        else
        {
            // DEFENDING: Dynamic Engagement
            // If threat is high (> 0.7), move to Ball
            // If threat is low (< 0.3), stay Home
            // Otherwise blend
            
            if (ThreatRatio > 0.7f)
            {
                // CRITICAL DEFENSE: Convert to ball chaser behavior essentially
                // We define this via position - if position == ball, DistToSupport becomes 0
                SupportPos = BallPosition;
            }
            else
            {
                // ZONAL DEFENSE: Position between Ball and Goal, biased by Home X
                FVector MyGoalPos(0.0f, MyGoalY, 0.0f);
                
                // Intercept vector
                FVector InterceptPos = FMath::Lerp(BallPosition, MyGoalPos, 0.25f);
                
                // Blend Intercept Y with Home X to maintain lane
                SupportPos.X = EffectiveHome.X; 
                SupportPos.Y = InterceptPos.Y;
                
                // If ball is on our flank, shift X to cover
                if (FMath::Abs(BallPosition.X - EffectiveHome.X) < 1000.0f)
                {
                    SupportPos.X = FMath::Lerp(EffectiveHome.X, BallPosition.X, 0.5f);
                }
            }
        }
    }
    // ==================== GOALKEEPER LOGIC ====================
    else if (AIProfile.Contains(TEXT("Goalkeeper")))
    {
        // Goalkeeper stays in goal area but shifts X to match ball (cut off angle)
        // Goal is at +/- FieldLength/2
        float GoalLineY = (MF_Constants::FieldLength / 2.0f) * -AttackDirection; 
        
        // Stay slightly off line (200 units)
        float BaseY = GoalLineY + (200.0f * AttackDirection);
        
        // Match Ball X but clamp to Goal Width (plus a bit of margin)
        // Goal Width is 7.32m (732 units). Half is 366.
        float ClampedX = FMath::Clamp(BallPosition.X, -400.0f, 400.0f);
        
        SupportPos = FVector(ClampedX, BaseY, MF_Constants::GroundZ);
        
        // ENGAGEMENT: If ball is very close (inside box approx), rush it?
        // Penalty Box is ~16m deep (1650 units).
        float DistFromLine = FMath::Abs(BallPosition.Y - GoalLineY);
        if (DistFromLine < 1200.0f && FMath::Abs(BallPosition.X) < 1500.0f && !bMyTeamHasBall)
        {
            // Rush ball if in box and opponent has it/loose
            SupportPos = BallPosition;
        }
    }
    // ==================== DEFAULT LOGIC ====================
    else
    {
        // Default: follow ball loosely
        SupportPos.Y += OffsetFromBall * 0.5f * AttackDirection;
    }
    
    // Clamp to field bounds (Safety net)
    const float HalfLength = MF_Constants::FieldLength / 2.0f - 100.0f; // Buffer
    const float HalfWidth = MF_Constants::FieldWidth / 2.0f - 100.0f;
    SupportPos.X = FMath::Clamp(SupportPos.X, -HalfWidth, HalfWidth); // Width is X
    SupportPos.Y = FMath::Clamp(SupportPos.Y, -HalfLength, HalfLength); // Length is Y
    SupportPos.Z = MF_Constants::GroundZ;
    
    return SupportPos;
}

FVector AMF_PlayerCharacter::CalculateSeparationVector() const
{
    FVector Separation(0.f);
    const float SeparationRadius = 250.0f; // 2.5 meters
    const float SeparationStrength = 1.6f; // Strong nudging to prevent clumping

    // Find nearby teammates
    for (TActorIterator<AMF_PlayerCharacter> It(GetWorld()); It; ++It)
    {
        AMF_PlayerCharacter* Other = *It;
        
        // Skip self and non-teammates
        if (!Other || Other == this || Other->GetTeamID() != TeamID)
        {
            continue;
        }

        float Dist = FVector::Dist(GetActorLocation(), Other->GetActorLocation());
        
        // If within radius, add repulsion
        if (Dist < SeparationRadius && Dist > 1.0f)
        {
            FVector ToMe = GetActorLocation() - Other->GetActorLocation();
            
            // Stronger repulsion the closer they are
            float Weight = 1.0f - (Dist / SeparationRadius);
            Separation += ToMe.GetSafeNormal() * Weight;
        }
    }

    if (!Separation.IsNearlyZero())
    {
        return Separation.GetSafeNormal() * SeparationStrength;
    }

    return FVector::ZeroVector;
}
