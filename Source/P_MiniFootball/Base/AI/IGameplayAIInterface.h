/*
 * @Author: Punal Manalan
 * @Description: IGameplayAIInterface - Interface for AI to interact with game systems
 *               Provides game-specific functions for P_EAIS actions
 * @Date: 29/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IGameplayAIInterface.generated.h"

// Forward declarations
class AMF_Ball;
class AMF_PlayerCharacter;

/**
 * Interface for gameplay AI interactions.
 * Implement this interface on GameMode or a manager actor to provide
 * game-specific functionality to P_EAIS actions.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UGameplayAIInterface : public UInterface
{
    GENERATED_BODY()
};

class P_MINIFOOTBALL_API IGameplayAIInterface
{
    GENERATED_BODY()

public:
    // ==================== Ball Queries ====================

    /** Get the current ball location in the world */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Ball")
    FVector GetBallLocation() const;

    /** Get the ball actor */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Ball")
    AMF_Ball* GetBall() const;

    /** Check if a character is in possession of the ball */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Ball")
    bool IsInPossession(AActor* Player) const;

    // ==================== Team Queries ====================

    /** Get the closest teammate to a given actor */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Team")
    AActor* GetClosestTeammate(AActor* InActor) const;

    /** Get all teammates of a given actor */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Team")
    TArray<AActor*> GetTeammates(AActor* InActor) const;

    /** Get all opponents of a given actor */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Team")
    TArray<AActor*> GetOpponents(AActor* InActor) const;

    /** Get the goal location for a team (the goal they're attacking) */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Team")
    FVector GetOpponentGoalLocation(AActor* InActor) const;

    /** Get own goal location (the goal they're defending) */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Team")
    FVector GetOwnGoalLocation(AActor* InActor) const;

    // ==================== Actions ====================

    /** Attempt to pass the ball to a target */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Actions")
    bool AttemptPass(AActor* From, AActor* To, float Power);

    /** Attempt to shoot at goal */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Actions")
    bool AttemptShot(AActor* Shooter, FVector Target, float Power);

    /** Attempt to tackle another player */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Actions")
    bool AttemptTackle(AActor* Tackler, AActor* Target);

    // ==================== State Queries ====================

    /** Get the current match phase (e.g., "Playing", "Kickoff", "HalfTime") */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Match")
    FString GetMatchPhase() const;

    /** Get remaining match time in seconds */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Match")
    float GetRemainingTime() const;

    /** Get team score */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Match")
    int32 GetTeamScore(int32 TeamID) const;
};
