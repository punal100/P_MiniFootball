/*
 * @Author: Punal Manalan
 * @Description: MF_PenaltyArea - Penalty Area Actor
 *               Defines the penalty area bounds for each team
 * @Date: 17/01/2026
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/MF_Types.h"
#include "MF_PenaltyArea.generated.h"

class UBoxComponent;

/**
 * MF_PenaltyArea represents the penalty area bounds for a team.
 * Two instances are expected (one for each goal end).
 */
UCLASS()
class P_MINIFOOTBALL_API AMF_PenaltyArea : public AActor
{
    GENERATED_BODY()

public:
    AMF_PenaltyArea();

    // ==================== Components ====================

    /** Box component defining the penalty area dimensions */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent *PenaltyAreaBounds;

    // ==================== Configuration ====================

    /** Which team DEFENDS this penalty area (the GK's team) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PenaltyArea")
    EMF_TeamID DefendingTeam = EMF_TeamID::None;

    // ==================== Utilities ====================

    /** Check if a world location is inside this penalty area */
    UFUNCTION(BlueprintCallable, Category = "PenaltyArea")
    bool IsLocationInside(const FVector &Location) const;

    /** Get the center of the penalty area */
    UFUNCTION(BlueprintCallable, Category = "PenaltyArea")
    FVector GetPenaltyAreaCenter() const;

    /** Get the box extent (half-size) */
    UFUNCTION(BlueprintCallable, Category = "PenaltyArea")
    FVector GetPenaltyAreaExtent() const;

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
};
