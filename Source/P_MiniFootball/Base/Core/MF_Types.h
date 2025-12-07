/*
 * @Author: Punal Manalan
 * @Description: MF_Types - Core types, enums, and constants for Mini Football
 * @Date: 07/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "MF_Types.generated.h"

// ==================== Team Identification ====================

UENUM(BlueprintType)
enum class EMF_TeamID : uint8
{
    None UMETA(DisplayName = "None"),
    TeamA UMETA(DisplayName = "Team A"),
    TeamB UMETA(DisplayName = "Team B")
};

// ==================== Match Phases ====================

UENUM(BlueprintType)
enum class EMF_MatchPhase : uint8
{
    WaitingForPlayers UMETA(DisplayName = "Waiting For Players"),
    Kickoff UMETA(DisplayName = "Kickoff"),
    Playing UMETA(DisplayName = "Playing"),
    GoalScored UMETA(DisplayName = "Goal Scored"),
    HalfTime UMETA(DisplayName = "Half Time"),
    MatchEnd UMETA(DisplayName = "Match End")
};

// ==================== Player States ====================

UENUM(BlueprintType)
enum class EMF_PlayerState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Running UMETA(DisplayName = "Running"),
    Sprinting UMETA(DisplayName = "Sprinting"),
    HasBall UMETA(DisplayName = "Has Ball"),
    Shooting UMETA(DisplayName = "Shooting"),
    Passing UMETA(DisplayName = "Passing"),
    Tackling UMETA(DisplayName = "Tackling"),
    Stunned UMETA(DisplayName = "Stunned")
};

// ==================== Ball States ====================

UENUM(BlueprintType)
enum class EMF_BallState : uint8
{
    Loose UMETA(DisplayName = "Loose"),              // No one has the ball
    Possessed UMETA(DisplayName = "Possessed"),      // A player has the ball
    InFlight UMETA(DisplayName = "In Flight"),       // Ball was kicked, flying
    OutOfBounds UMETA(DisplayName = "Out Of Bounds") // Ball left the field
};

// ==================== Input Action Names ====================

namespace MF_InputActions
{
    const FName Move = FName(TEXT("IA_MF_Move"));
    const FName Action = FName(TEXT("IA_MF_Action"));
    const FName Sprint = FName(TEXT("IA_MF_Sprint"));
    const FName SwitchPlayer = FName(TEXT("IA_MF_SwitchPlayer"));
    const FName Pause = FName(TEXT("IA_MF_Pause"));
}

// ==================== Game Constants ====================

namespace MF_Constants
{
    // Field Dimensions (cm)
    constexpr float FieldLength = 4000.0f;       // 40 meters
    constexpr float FieldWidth = 2500.0f;        // 25 meters
    constexpr float GoalWidth = 400.0f;          // 4 meters
    constexpr float GoalHeight = 200.0f;         // 2 meters
    constexpr float CenterCircleRadius = 300.0f; // 3 meters

    // Player Movement (cm/s)
    constexpr float WalkSpeed = 400.0f;
    constexpr float SprintSpeed = 600.0f;
    constexpr float Acceleration = 2000.0f;
    constexpr float TurnRate = 540.0f;        // degrees/s
    constexpr float PossessionRadius = 80.0f; // Auto pick up ball range

    // Ball Physics (Math-Based - NO UE Physics)
    constexpr float BallShootSpeed = 2500.0f;     // cm/s for strong shots
    constexpr float BallPassSpeed = 1200.0f;      // cm/s for passes
    constexpr float BallFriction = 500.0f;        // Deceleration cm/s per second
    constexpr float BallMinSpeed = 10.0f;         // Stop ball below this
    constexpr float BallBounceRestitution = 0.7f; // Velocity retained on bounce
    constexpr float BallRadius = 11.0f;           // cm (FIFA standard)
    constexpr float BallPickupRadius = 150.0f;    // cm - auto pickup range (increased for easier pickup)
    constexpr float BallAirResistance = 50.0f;    // cm/s^2 deceleration in air
    constexpr float BallBounciness = 0.6f;        // Velocity retained on ground bounce

    // Physics Constants
    constexpr float Gravity = 980.0f;               // cm/s^2 (9.8 m/s^2)
    constexpr float GroundZ = 0.0f;                 // Ground plane Z level
    constexpr float CharacterHalfHeight = 96.0f;    // Default capsule half-height (88) + some margin
    constexpr float CharacterSpawnZOffset = 100.0f; // Spawn offset above ground to prevent embedding

    // Field Bounds
    constexpr float OutOfBoundsBuffer = 100.0f; // cm buffer zone for out of bounds check

    // Match Settings
    constexpr float MatchDuration = 180.0f;     // 3 minutes
    constexpr float KickoffCountdown = 3.0f;    // 3 seconds countdown
    constexpr float GoalCelebrationTime = 2.0f; // 2 seconds after goal
    constexpr int32 MaxPlayersPerTeam = 3;      // 3v3

    // Tackling
    constexpr float TackleCooldown = 1.0f;     // seconds
    constexpr float TackleRange = 100.0f;      // cm
    constexpr float TackleStunDuration = 0.5f; // seconds

    // Network
    constexpr float NetUpdateFrequency = 60.0f;    // Updates per second
    constexpr float MinNetUpdateFrequency = 30.0f; // Minimum updates per second
}

// ==================== Replication Info Struct ====================

USTRUCT(BlueprintType)
struct FMF_ReplicatedMovement
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite)
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    bool bIsSprinting = false;
};

// ==================== Ball Replication Struct ====================

USTRUCT(BlueprintType)
struct FMF_BallReplicationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    EMF_BallState State = EMF_BallState::Loose;

    UPROPERTY(BlueprintReadWrite)
    uint8 PossessingPlayerID = 0; // 0 = no one

    UPROPERTY(BlueprintReadWrite)
    float ServerTimestamp = 0.0f;
};
