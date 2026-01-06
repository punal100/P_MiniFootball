/*
 * @Author: Punal Manalan
 * @Description: MF_PlayerCharacter - Replicated football player character
 *               Supports both Listen Server and Dedicated Server
 *               Server authoritative movement with client prediction
 * @Date: 07/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Core/MF_Types.h"
#include "EAIS_TargetProvider.h"
#include "MF_PlayerCharacter.generated.h"

class UMF_InputHandler;
class AMF_Ball;
class USceneComponent;
class UCapsuleComponent;
class USpringArmComponent;
class UCameraComponent;
class UAIComponent;
class UAIBehaviour;

// ==================== Delegates ====================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMF_PossessionChanged, AMF_PlayerCharacter *, Player, bool, bHasBall);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMF_PlayerStateChanged, EMF_PlayerState, NewState);

/**
 * MF_PlayerCharacter
 * The football player character with full network replication
 *
 * Network Model:
 * - Server: Authoritative for all game logic (possession, actions)
 * - Client: Sends input via RPC, predicts movement locally
 * - Replication: Position, Rotation, State all replicated
 */
UCLASS()
class P_MINIFOOTBALL_API AMF_PlayerCharacter : public ACharacter, public IEAIS_TargetProvider
{
    GENERATED_BODY()

public:
    AMF_PlayerCharacter();

    // ==================== IEAIS_TargetProvider Interface ====================

    virtual bool EAIS_GetTargetLocation_Implementation(FName TargetId, FVector& OutLocation) const override;
    virtual bool EAIS_GetTargetActor_Implementation(FName TargetId, AActor*& OutActor) const override;

    virtual int32 EAIS_GetTeamId_Implementation() const override { return (int32)GetTeamID(); }
    virtual FString EAIS_GetRole_Implementation() const override { return AIProfile; }


    // ==================== AActor Interface ====================

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;
    virtual void PossessedBy(AController *NewController) override;
    virtual void UnPossessed() override;
    virtual void OnRep_Owner() override;  // Called on client when possessed
    virtual void OnRep_PlayerState() override;

    // ==================== Team & Identity ====================

    /** Get this player's team */
    UFUNCTION(BlueprintPure, Category = "MiniFootball|Player")
    EMF_TeamID GetTeamID() const { return TeamID; }

    /** Set this player's team (Server only) */
    UFUNCTION(BlueprintCallable, Category = "MiniFootball|Player", meta = (BlueprintAuthorityOnly))
    void SetTeamID(EMF_TeamID NewTeam);

    /** Get the player's unique ID within the match */
    UFUNCTION(BlueprintPure, Category = "MiniFootball|Player")
    uint8 GetPlayerID() const { return PlayerID; }

    /** Set player ID (Server only) */
    void SetPlayerID(uint8 NewID) { PlayerID = NewID; }

    // ==================== Ball Possession ====================

    /** Check if this player has the ball */
    UFUNCTION(BlueprintPure, Category = "MiniFootball|Player")
    bool HasBall() const { return bHasBall; }

    /** Called by ball/server when possession changes */
    void SetHasBall(bool bNewHasBall);

    /** Get the ball we're possessing (if any) */
    UFUNCTION(BlueprintPure, Category = "MiniFootball|Player")
    AMF_Ball *GetPossessedBall() const { return PossessedBall; }

    /** Set the ball reference (Server only) */
    void SetPossessedBall(AMF_Ball *Ball);

    /** Check if this character can receive ball possession */
    UFUNCTION(BlueprintPure, Category = "MiniFootball|Player")
    bool CanReceiveBall() const;

    /** 
     * CurrentBall - Replicated reference to the ball this character possesses.
     * This is the single source of truth for ball possession on the character side.
     * Invariant: bHasBall == (CurrentBall != nullptr)
     */
    UPROPERTY(ReplicatedUsing = OnRep_CurrentBall, BlueprintReadOnly, Category = "MiniFootball|Player")
    AMF_Ball* CurrentBall = nullptr;

    // ==================== Player State ====================

    /** Get current player state */
    UFUNCTION(BlueprintPure, Category = "MiniFootball|Player")
    EMF_PlayerState GetPlayerState() const { return CurrentPlayerState; }

    /** Set player state (Server only) */
    UFUNCTION(BlueprintCallable, Category = "MiniFootball|Player", meta = (BlueprintAuthorityOnly))
    void SetPlayerState(EMF_PlayerState NewState);

    /** Check if player is stunned (can't act) */
    UFUNCTION(BlueprintPure, Category = "MiniFootball|Player")
    bool IsStunned() const { return CurrentPlayerState == EMF_PlayerState::Stunned; }

    // ==================== Movement ====================

    /** Apply movement input (called by InputHandler) */
    UFUNCTION(BlueprintCallable, Category = "MiniFootball|Player")
    void ApplyMoveInput(FVector2D MoveInput);

    /** Set sprint state */
    UFUNCTION(BlueprintCallable, Category = "MiniFootball|Player")
    void SetSprinting(bool bNewSprinting);

    /** Check if currently sprinting */
    UFUNCTION(BlueprintPure, Category = "MiniFootball|Player")
    bool IsSprinting() const { return bIsSprinting; }

    // ==================== AI Configuration ====================
    /** The AI Behavior profile to use */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    FString AIProfile = TEXT("Striker");

    /** Optional: Pre-assigned behavior asset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    UAIBehaviour* AIBehaviour;

    /** Should AI start automatically? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    bool bAutoStartAI = true;

    /** AI tick interval */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config", meta = (ClampMin = "0.0"))
    float AITickInterval = 0.1f;

    /** Enable AI debug visualization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Debug")
    bool bDebugAI = false;

    // ==================== AI Control ====================
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartAI();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopAI();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void ResetAI();

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool SetAIProfile(const FString& ProfileName);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void InjectAIEvent(const FString& EventName);

    UFUNCTION(BlueprintPure, Category = "AI")
    UAIComponent* GetAIComponent() const { return AIComponent; }

    UFUNCTION(BlueprintPure, Category = "AI")
    bool IsAIRunning() const;

    UFUNCTION(BlueprintPure, Category = "AI")
    FString GetCurrentAIState() const;

    // ==================== Actions (Client -> Server RPC) ====================

    /** Request shoot action (Client to Server) */
    UFUNCTION(Server, Reliable, WithValidation, Category = "MiniFootball|Actions")
    void Server_RequestShoot(FVector Direction, float Power);

    /** Request pass action (Client to Server) */
    UFUNCTION(Server, Reliable, WithValidation, Category = "MiniFootball|Actions")
    void Server_RequestPass(FVector Direction, float Power);

    /** Request tackle action (Client to Server) */
    UFUNCTION(Server, Reliable, WithValidation, Category = "MiniFootball|Actions")
    void Server_RequestTackle();

    /** Request move input (Client to Server for server-authoritative movement) */
    UFUNCTION(Server, Unreliable, WithValidation, Category = "MiniFootball|Movement")
    void Server_SendMoveInput(FVector2D MoveInput, bool bSprinting);

    // ==================== Events ====================

    /** Broadcast when possession changes */
    UPROPERTY(BlueprintAssignable, Category = "MiniFootball|Events")
    FOnMF_PossessionChanged OnPossessionChanged;

    /** Broadcast when player state changes */
    UPROPERTY(BlueprintAssignable, Category = "MiniFootball|Events")
    FOnMF_PlayerStateChanged OnPlayerStateChanged;

    /** Broadcast when ball state changes (for HUD/Ability system) */
    UPROPERTY(BlueprintAssignable, Category = "MiniFootball|Events")
    FOnMF_PossessionChanged OnBallStateChanged;

    // ==================== Input Handler Access ====================

    /** Get the input handler component */
    UFUNCTION(BlueprintPure, Category = "MiniFootball|Input")
    UMF_InputHandler *GetInputHandler() const { return InputHandler; }

protected:
    // ==================== Components ====================

    /** Input handler component (handles P_MEIS integration) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MiniFootball|Components")
    UMF_InputHandler *InputHandler;

    /** AI Component for EAIS */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIComponent* AIComponent;

    /** Action Executor for EAIS */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UMF_EAISActionExecutorComponent* AIActionExecutor;


    /** Camera boom for top-down view */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MiniFootball|Components")
    USpringArmComponent *CameraBoom;

    /** Top-down camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MiniFootball|Components")
    UCameraComponent *TopDownCamera;

    // ==================== Replicated Properties ====================

    /** Team this player belongs to */
    UPROPERTY(ReplicatedUsing = OnRep_TeamID, BlueprintReadOnly, Category = "MiniFootball|Player")
    EMF_TeamID TeamID = EMF_TeamID::None;

    /** Unique player ID in match */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "MiniFootball|Player")
    uint8 PlayerID = 0;

    /** Whether this player has the ball (derived from CurrentBall but replicated for UI/prediction) */
    UPROPERTY(ReplicatedUsing = OnRep_HasBall, BlueprintReadOnly, Category = "MiniFootball|Player")
    bool bHasBall = false;

    /** Current player state */
    UPROPERTY(ReplicatedUsing = OnRep_CurrentPlayerState, BlueprintReadOnly, Category = "MiniFootball|Player")
    EMF_PlayerState CurrentPlayerState = EMF_PlayerState::Idle;

    /** Sprint state */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "MiniFootball|Player")
    bool bIsSprinting = false;

    // ==================== Non-Replicated State ====================

    /** 
     * Legacy reference to possessed ball (for backward compatibility).
     * @deprecated Use CurrentBall instead.
     */
    UPROPERTY()
    AMF_Ball *PossessedBall = nullptr;

    /** Current move input (local) */
    FVector2D CurrentMoveInput = FVector2D::ZeroVector;

    /** Tackle cooldown timer */
    float TackleCooldownRemaining = 0.0f;

    /** Stun timer */
    float StunTimeRemaining = 0.0f;

    /** 
     * Flag to track if action was consumed by tackle on press.
     * When true, skip shoot/pass on release to prevent immediate shoot after tackle.
     */
    bool bActionConsumedByTackle = false;

    // ==================== Rep Notifies ====================

    UFUNCTION()
    void OnRep_TeamID();

    UFUNCTION()
    void OnRep_HasBall();

    UFUNCTION()
    void OnRep_CurrentBall();

    UFUNCTION()
    void OnRep_CurrentPlayerState();

    // ==================== Internal Functions ====================

    /** Synchronize game state to AI blackboard */
    void SyncBlackboard();

    /** Called when possession changes - update AI blackboard */
    void OnBallPossessionChanged();

    /** Setup input bindings via InputHandler */
    void SetupInputBindings();

    /** Update movement based on current input */
    void UpdateMovement(float DeltaTime);

    /** Apply stun to this player (Server only) */
    void ApplyStun(float Duration);

    // ==================== Server-Side Action Execution ====================

    /** Execute shoot (Server only) */
    void ExecuteShoot(FVector Direction, float Power);

    /** Execute pass (Server only) */
    void ExecutePass(FVector Direction, float Power);

    /** Execute tackle (Server only) */
    void ExecuteTackle();

    // ==================== Input Callbacks ====================

    UFUNCTION()
    void OnMoveInputReceived(FVector2D MoveValue);

    UFUNCTION()
    void OnSprintInputReceived(bool bSprint);

    UFUNCTION()
    void OnActionPressed(bool bPressed);

    UFUNCTION()
    void OnActionReleased();

    UFUNCTION()
    void OnActionHeld(float HoldTime);

    UFUNCTION()
    void OnSwitchPlayerInputReceived();

    UFUNCTION()
    void OnPauseInputReceived();
};
