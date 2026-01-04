/*
 * @Author: Punal Manalan
 * @Description: MF_Field - Football Field Actor
 *               Defines the playable area and automatically configures NavMesh bounds
 * @Date: 04/01/2026
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MF_Field.generated.h"

class UBoxComponent;

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

    /** Forces a runtime rebuild of the navigation mesh. Useful for resolving runtime issues. */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
    void ForceRebuildNavigation();

    /** Margin to add around the field bounds for NavMesh generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    float NavMeshMargin = 500.0f;

protected:
    virtual void BeginPlay() override;

};
