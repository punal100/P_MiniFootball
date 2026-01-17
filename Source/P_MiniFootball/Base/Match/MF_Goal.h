/*
 * @Author: Punal Manalan
 * @Description: MF_Goal - Goal Trigger Volume
 *               Detects when ball enters goal area
 * @Date: 07/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/MF_Types.h"
#include "MF_Goal.generated.h"

class UBoxComponent;
class AMF_Ball;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGoalTriggered, AMF_Goal *, Goal, AMF_Ball *, Ball);

/**
 * MF_Goal - Goal trigger volume
 *
 * Place two of these in your level:
 * - Goal A at one end (set GoalTeam to TeamA)
 * - Goal B at other end (set GoalTeam to TeamB)
 *
 * When the ball enters, it broadcasts OnGoalTriggered
 * and notifies the GameState to score.
 */
UCLASS()
class P_MINIFOOTBALL_API AMF_Goal : public AActor
{
    GENERATED_BODY()

public:
    AMF_Goal();

    // ==================== Components ====================

    /** Trigger volume for goal detection */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent *GoalTrigger;

    // ==================== Configuration ====================

    /** Which team DEFENDS this goal (The owner). The OPPOSITE team scores here. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goal")
    EMF_TeamID DefendingTeam = EMF_TeamID::None;

    // ==================== Events ====================

    /** Fires when ball enters this goal */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnGoalTriggered OnGoalTriggered;

#if WITH_EDITORONLY_DATA
    /** Enable debug visualization in editor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugInEditor = false;
#endif

protected:
    virtual void BeginPlay() override;

#if !UE_BUILD_SHIPPING
    virtual void Tick(float DeltaTime) override;
#endif

    /** Called when something overlaps the goal trigger */
    UFUNCTION()
    void OnGoalOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
                       UPrimitiveComponent *OtherComp, int32 OtherBodyIndex,
                       bool bFromSweep, const FHitResult &SweepResult);

private:
    /** Prevent multiple goal detections for same ball entry */
    bool bGoalScoredThisFrame = false;

    /** Reset the goal scored flag after a delay */
    UFUNCTION()
    void ResetGoalFlag();
};
