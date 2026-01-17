/*
 * @Author: Punal Manalan
 * @Description: MF_Field - Football Field Actor
 *               Defines the playable area and automatically configures NavMesh bounds
 * @Date: 04/01/2026
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/MF_Types.h"
#include "MF_Field.generated.h"

class UBoxComponent;
class AMF_Goal;
class AMF_PenaltyArea;

/**
 * MF_Field represents the football field bounds.
 * It provides utilities to automatically generate/resize the NavMeshBoundsVolume to match the field.
 */
UCLASS()
class P_MINIFOOTBALL_API AMF_Field : public AActor
{
    GENERATED_BODY()
    
public:    
    AMF_Field();

    // ==================== Components ====================

    /** Box component defining the field dimensions */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* FieldBounds;

    // ==================== Utilities ====================

    /** 
     * Finds or creates a NavMeshBoundsVolume and updates its size to match this field 
     * plus a configured margin. Call this from the Editor to setup navigation.
     */
    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Navigation")
    void UpdateNavMesh();

    UFUNCTION(BlueprintCallable, Category = "Navigation")
    void EnsureNavMesh();

    /** Forces a runtime rebuild of the navigation mesh. Useful for resolving runtime issues. */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
    void ForceRebuildNavigation();

    /** Margin to add around the field bounds for NavMesh generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    float NavMeshMargin = 500.0f;

    // ==================== Goal Configuration ====================

    /** Width of the goal (default: FIFA standard 7.32m) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goal", meta = (ClampMin = "100.0"))
    float GoalWidth = MF_Constants::GoalWidth;

    /** Height of the goal (default: FIFA standard 2.44m) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goal", meta = (ClampMin = "100.0"))
    float GoalHeight = MF_Constants::GoalHeight;

    /** Depth of the goal trigger box */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goal", meta = (ClampMin = "10.0"))
    float GoalDepth = 50.0f;

    /** Enable automatic goal spawning on construction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goal")
    bool bAutoSpawnGoals = true;

    // ==================== Penalty Area Configuration ====================

    /** Length of the penalty area (default: FIFA standard 16.5m) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PenaltyArea", meta = (ClampMin = "100.0"))
    float PenaltyAreaLength = MF_Constants::PenaltyAreaLength;

    /** Width of the penalty area (default: FIFA standard 40.3m) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PenaltyArea", meta = (ClampMin = "100.0"))
    float PenaltyAreaWidth = MF_Constants::PenaltyAreaWidth;

    /** Enable automatic penalty area spawning on construction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PenaltyArea")
    bool bAutoSpawnPenaltyAreas = true;

    // ==================== Debug Visualization ====================

#if WITH_EDITORONLY_DATA
    /** Enable debug visualization for field bounds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowFieldDebug = false;

    /** Enable debug visualization for goals */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowGoalDebug = false;

    /** Enable debug visualization for penalty areas */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowPenaltyAreaDebug = false;
#endif

    // ==================== Spawned Actors (Runtime) ====================

    /** Reference to spawned Goal A (TeamA defends) */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Spawned")
    AMF_Goal *GoalA = nullptr;

    /** Reference to spawned Goal B (TeamB defends) */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Spawned")
    AMF_Goal *GoalB = nullptr;

    /** Reference to spawned Penalty Area A (TeamA defends) */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Spawned")
    AMF_PenaltyArea *PenaltyAreaA = nullptr;

    /** Reference to spawned Penalty Area B (TeamB defends) */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Spawned")
    AMF_PenaltyArea *PenaltyAreaB = nullptr;

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

#if !UE_BUILD_SHIPPING
    virtual void Tick(float DeltaTime) override;
#endif

    /** Spawn or update goals based on field configuration */
    void SpawnOrUpdateGoals();

    /** Spawn or update penalty areas based on field configuration */
    void SpawnOrUpdatePenaltyAreas();

    /** Destroy any previously spawned field components */
    void DestroySpawnedComponents();

    /** Draw debug visualization */
    void DrawFieldDebug();

};
