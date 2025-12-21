/*
 * @Author: Punal Manalan
 * @Description: MF_Spectator - Implementation
 * @Date: 09/12/2025
 */

#include "Player/MF_Spectator.h"
#include "Ball/MF_Ball.h"
#include "Core/MF_Types.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

AMF_Spectator::AMF_Spectator()
{
    // Enable default movement bindings for WASD and look controls
    bAddDefaultMovementBindings = true;

    // Enable controller rotation for mouse look
    // This makes the pawn face the direction the controller (mouse) is pointing
    bUseControllerRotationPitch = true;  // Allow looking up/down
    bUseControllerRotationYaw = true;    // Allow looking left/right
    bUseControllerRotationRoll = false;  // Keep roll disabled to prevent flipping

    // Setup camera boom
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 2000.0f;
    CameraBoom->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));  // Slightly less steep angle
    
    // Use pawn control rotation so mouse look works
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bInheritPitch = true;
    CameraBoom->bInheritYaw = true;
    CameraBoom->bInheritRoll = false;
    CameraBoom->bDoCollisionTest = false;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 5.0f;  // Slightly faster lag for responsiveness

    // Setup camera
    SpectatorCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("SpectatorCamera"));
    SpectatorCamera->SetupAttachment(CameraBoom);
    SpectatorCamera->bUsePawnControlRotation = false;  // Camera inherits from boom, not controller

    // Default settings
    bFollowBall = true;
    CameraSpeed = 1000.0f;
    CameraHeight = 1500.0f;
    CameraFollowSmoothness = 5.0f;
}

void AMF_Spectator::BeginPlay()
{
    Super::BeginPlay();

    // Find the ball
    FindBall();

    // Set initial position above field center
    SetActorLocation(FVector(0.0f, 0.0f, CameraHeight));

    UE_LOG(LogTemp, Log, TEXT("MF_Spectator::BeginPlay - Spectator pawn spawned"));
}

void AMF_Spectator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bFollowBall)
    {
        UpdateBallFollow(DeltaTime);
    }
}

void AMF_Spectator::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UE_LOG(LogTemp, Log, TEXT("MF_Spectator::SetupPlayerInputComponent - Setting up spectator inputs"));

    // Bind F key to toggle ball follow
    PlayerInputComponent->BindAction("ToggleBallFollow", IE_Pressed, this, &AMF_Spectator::ToggleFollowBall);
    
    // Simple fallback: Also bind F key directly in case action mapping doesn't exist
    PlayerInputComponent->BindKey(EKeys::F, IE_Pressed, this, &AMF_Spectator::ToggleFollowBall);
}

void AMF_Spectator::ToggleFollowBall()
{
    SetFollowBall(!bFollowBall);
}

void AMF_Spectator::SetFollowBall(bool bFollow)
{
    bFollowBall = bFollow;

    UE_LOG(LogTemp, Log, TEXT("MF_Spectator::SetFollowBall - Follow ball: %s"),
           bFollowBall ? TEXT("ON") : TEXT("OFF"));
}

void AMF_Spectator::MoveToLocation(FVector Location)
{
    // Keep the camera at the specified height
    Location.Z = CameraHeight;
    SetActorLocation(Location);
}

AMF_Ball *AMF_Spectator::GetBall() const
{
    return CachedBall.Get();
}

void AMF_Spectator::FindBall()
{
    if (CachedBall.IsValid())
    {
        return;
    }

    TArray<AActor *> FoundBalls;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMF_Ball::StaticClass(), FoundBalls);

    if (FoundBalls.Num() > 0)
    {
        CachedBall = Cast<AMF_Ball>(FoundBalls[0]);
    }
}

void AMF_Spectator::UpdateBallFollow(float DeltaTime)
{
    // Try to find ball if not cached
    if (!CachedBall.IsValid())
    {
        FindBall();
        return;
    }

    FVector BallLocation = CachedBall->GetActorLocation();
    FVector CurrentLocation = GetActorLocation();

    // Target location above the ball
    FVector TargetLocation = FVector(BallLocation.X, BallLocation.Y, CameraHeight);

    // Clamp to field bounds
    TargetLocation.X = FMath::Clamp(TargetLocation.X, -MF_Constants::FieldWidth / 2.0f, MF_Constants::FieldWidth / 2.0f);
    TargetLocation.Y = FMath::Clamp(TargetLocation.Y, -MF_Constants::FieldLength / 2.0f, MF_Constants::FieldLength / 2.0f);

    // Smoothly interpolate to target
    FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, CameraFollowSmoothness);

    SetActorLocation(NewLocation);
}
