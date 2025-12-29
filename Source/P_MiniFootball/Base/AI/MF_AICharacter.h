/*
 * @Author: Punal Manalan
 * @Description: MF_AICharacter - AI-controlled football player that uses P_EAIS
 *               Wrapper around MF_PlayerCharacter with AI behavior integration
 * @Date: 29/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "../Player/MF_PlayerCharacter.h"
#include "MF_AICharacter.generated.h"

class UAIComponent;
class UAIBehaviour;

/**
 * AI-controlled football player character.
 * Uses P_EAIS for behavior and P_MEIS for input injection.
 * 
 * This class extends MF_PlayerCharacter with AI capabilities:
 * - Automatic P_EAIS component attachment
 * - Blackboard synchronization with game state
 * - Profile-based behavior loading
 */
UCLASS()
class P_MINIFOOTBALL_API AMF_AICharacter : public AMF_PlayerCharacter
{
    GENERATED_BODY()

public:
    AMF_AICharacter();

    // ==================== Lifecycle ====================
    
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void PossessedBy(AController* NewController) override;

    // ==================== AI Configuration ====================

    /** The AI Behavior profile to use (e.g., "Striker", "Defender", "Goalkeeper") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    FString AIProfile = TEXT("Striker");

    /** Optional: Pre-assigned behavior asset (takes priority over ProfileName) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    UAIBehaviour* AIBehaviour;

    /** Should AI start automatically? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    bool bAutoStartAI = true;

    /** AI tick interval (0 = every frame) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config", meta = (ClampMin = "0.0"))
    float AITickInterval = 0.1f;

    /** Enable AI debug visualization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Debug")
    bool bDebugAI = false;

    // ==================== AI Control ====================

    /** Start the AI behavior */
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartAI();

    /** Stop the AI behavior */
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopAI();

    /** Reset the AI to initial state */
    UFUNCTION(BlueprintCallable, Category = "AI")
    void ResetAI();

    /** Change the AI profile at runtime */
    UFUNCTION(BlueprintCallable, Category = "AI")
    bool SetAIProfile(const FString& ProfileName);

    /** Inject an event into the AI */
    UFUNCTION(BlueprintCallable, Category = "AI")
    void InjectAIEvent(const FString& EventName);

    // ==================== AI Component Access ====================

    /** Get the AI component */
    UFUNCTION(BlueprintPure, Category = "AI")
    UAIComponent* GetAIComponent() const { return AIComponent; }

    /** Is the AI currently running? */
    UFUNCTION(BlueprintPure, Category = "AI")
    bool IsAIRunning() const;

    /** Get the current AI state name */
    UFUNCTION(BlueprintPure, Category = "AI")
    FString GetCurrentAIState() const;

protected:
    /** The AI component (created at construction) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIComponent* AIComponent;

    /** Synchronize game state to AI blackboard */
    void SyncBlackboard();

    /** Called when possession changes - update AI blackboard */
    void OnBallPossessionChanged();
};
