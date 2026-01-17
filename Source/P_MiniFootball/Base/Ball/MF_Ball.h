/*
 * @Author: Punal Manalan
 * @Description: MF_Ball - Replicated Ball Actor Header
 *               Math-based ball physics (NO UE Physics)
 *               Full network replication for Listen Server and Dedicated Server
 * @Date: 07/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/MF_Types.h"
#include "MF_Ball.generated.h"

class AMF_PlayerCharacter;
class USphereComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBallPossessionChanged, AMF_Ball *, Ball, AMF_PlayerCharacter *, OldPossessor, AMF_PlayerCharacter *, NewPossessor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBallStateChanged, AMF_Ball *, Ball, EMF_BallState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBallOutOfBounds, AMF_Ball *, Ball);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGoalScored, AMF_Ball *, Ball, EMF_TeamID, ScoringTeam);

/**
 * MF_Ball - The football actor
 *
 * Features:
 * - Custom math-based physics (NO UE Physics simulation)
 * - Server authoritative with client interpolation
 * - Possession system: ball attaches to possessing player
 * - Kick mechanics for shooting and passing
 */
UCLASS()
class P_MINIFOOTBALL_API AMF_Ball : public AActor
{
    GENERATED_BODY()

public:
    AMF_Ball();

    // ==================== Replication ====================
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    // ==================== Components ====================
    /** Root collision sphere */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent *CollisionSphere;

    /** Ball mesh */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent *BallMesh;

    // ==================== Replicated State ====================
    /** Current ball state */
    UPROPERTY(ReplicatedUsing = OnRep_BallState, BlueprintReadOnly, Category = "Ball")
    EMF_BallState CurrentBallState;

    /** Which player has the ball (null if loose/flying) */
    UPROPERTY(ReplicatedUsing = OnRep_Possessor, BlueprintReadOnly, Category = "Ball")
    AMF_PlayerCharacter *CurrentPossessor;

    /**
     * GC-safe cached handle for possessor dereferencing.
     * We keep the replicated raw pointer for networking/Blueprints, but use this for runtime access.
     */
    UPROPERTY(Transient)
    TWeakObjectPtr<AMF_PlayerCharacter> CurrentPossessorWeak;

    /** Replicated ball physics data */
    UPROPERTY(ReplicatedUsing = OnRep_BallPhysics, BlueprintReadOnly, Category = "Ball")
    FMF_BallReplicationData ReplicatedPhysics;

    // ==================== Ball Physics (Math-Based) ====================
    /** Current velocity (cm/s) */
    UPROPERTY(BlueprintReadOnly, Category = "Physics")
    FVector Velocity;

    /** Current angular velocity (rad/s) */
    UPROPERTY(BlueprintReadOnly, Category = "Physics")
    FVector AngularVelocity;

    /** Is ball on ground */
    UPROPERTY(BlueprintReadOnly, Category = "Physics")
    bool bIsGrounded;

    /** Ball radius */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Physics")
    float BallRadius;

    /** Velocity threshold (squared) for auto-pickup eligibility */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Possession")
    float AutoPickupVelocityThreshold = 40000.0f;  // cm^2/s^2

    // ==================== Ball Actions ====================
    /** Kick the ball in a direction with power */
    UFUNCTION(BlueprintCallable, Category = "Ball")
    void Kick(FVector Direction, float Power, bool bAddHeight = false);

    /** Give possession to a player */
    UFUNCTION(BlueprintCallable, Category = "Ball")
    void SetPossessor(AMF_PlayerCharacter *NewPossessor);

    /** Release ball from current possessor */
    UFUNCTION(BlueprintCallable, Category = "Ball")
    void ReleasePossession();

    /** Force ball to a position (for game reset) */
    UFUNCTION(BlueprintCallable, Category = "Ball")
    void ResetToPosition(FVector NewPosition);

    /** Check if a player can pick up the ball */
    UFUNCTION(BlueprintPure, Category = "Ball")
    bool CanBePickedUpBy(AMF_PlayerCharacter *Player) const;

    /**
     * Assign possession to a new owner (Server only).
     * This is the authoritative possession assignment function.
     * Updates both Ball and Character state atomically.
     */
    UFUNCTION(BlueprintCallable, Category = "Ball")
    void AssignPossession(AMF_PlayerCharacter* NewOwner);

    /**
     * Clear possession completely (Server only).
     * Called when shooting/passing to release the ball.
     */
    UFUNCTION(BlueprintCallable, Category = "Ball")
    void ClearPossession();

    /**
     * Check if ball can be auto-picked up by a character.
     * Used for overlap-based pickup eligibility.
     */
    UFUNCTION(BlueprintPure, Category = "Ball")
    bool CanAutoPickup(const AMF_PlayerCharacter* Character) const;

    // ==================== State Getters ====================
    UFUNCTION(BlueprintPure, Category = "Ball")
    bool IsLoose() const { return CurrentBallState == EMF_BallState::Loose; }

    UFUNCTION(BlueprintPure, Category = "Ball")
    bool IsPossessed() const { return CurrentBallState == EMF_BallState::Possessed; }

    UFUNCTION(BlueprintPure, Category = "Ball")
    bool IsInFlight() const { return CurrentBallState == EMF_BallState::InFlight; }

    UFUNCTION(BlueprintPure, Category = "Ball")
    bool IsOutOfBounds() const { return CurrentBallState == EMF_BallState::OutOfBounds; }

    UFUNCTION(BlueprintPure, Category = "Ball")
    AMF_PlayerCharacter *GetPossessor() const { return CurrentPossessorWeak.Get(); }

    // ==================== Events ====================
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnBallPossessionChanged OnPossessionChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnBallStateChanged OnBallStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnBallOutOfBounds OnBallOutOfBounds;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnGoalScored OnGoalScored;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void HandlePossessorDestroyed(AActor* DestroyedActor);

    // ==================== Rep Notifies ====================
    UFUNCTION()
    void OnRep_BallState();

    UFUNCTION()
    void OnRep_Possessor();

    UFUNCTION()
    void OnRep_BallPhysics();

    // ==================== Internal Physics ====================
    /** Update ball physics simulation (Server only) */
    void UpdatePhysics(float DeltaTime);

    /** Apply gravity and friction */
    void ApplyForces(float DeltaTime);

    /** Check and handle ground collision */
    void CheckGroundCollision();

    /** Check and handle wall/boundary collisions */
    void CheckBoundaryCollisions();

    /** Check for goal scoring */
    void CheckGoalCollisions();

    /** Update ball position when possessed */
    void UpdatePossessedPosition();

    /** Interpolate client-side position */
    void ClientInterpolate(float DeltaTime);

    /** Set ball state with replication */
    void SetBallState(EMF_BallState NewState);

    /** Handle overlap with player for automatic pickup */
    UFUNCTION()
    void OnBallOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
                       UPrimitiveComponent *OtherComp, int32 OtherBodyIndex,
                       bool bFromSweep, const FHitResult &SweepResult);

    /** Check for nearby players who can pick up the ball (backup for overlap events) */
    void CheckForNearbyPlayers();

private:
    /** Offset from player when possessed */
    FVector PossessionOffset;

    /** Client-side interpolation target */
    FVector InterpolationTarget;
    FVector InterpolationVelocity;

    /** Last replicated position for interpolation */
    FVector LastReplicatedPosition;

    /** Cooldown for possession changes */
    float PossessionCooldown;

    /** Time of the last kick (server time seconds) */
    float LastKickTime = 0.0f;

    /** How long the last kicker cannot pick up the ball (seconds) */
    static constexpr float LastKickerCooldown = 1.0f;

    /** Last kicker (for assists/own-goals) */
    UPROPERTY()
    TWeakObjectPtr<AMF_PlayerCharacter> LastKicker;
};
