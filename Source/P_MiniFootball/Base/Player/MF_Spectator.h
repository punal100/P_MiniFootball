/*
 * @Author: Punal Manalan
 * @Description: MF_Spectator - Spectator Pawn for viewing matches
 *               Default pawn for players not yet on a team
 * @Date: 09/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "MF_Spectator.generated.h"

class UCameraComponent;
class USpringArmComponent;
class AMF_Ball;

/**
 * MF_Spectator - Spectator pawn for Mini Football
 *
 * Features:
 * - Free-roam camera for observing the match
 * - Option to follow the ball automatically
 * - Simple movement controls for camera positioning
 * - No gameplay input (view only)
 */
UCLASS()
class P_MINIFOOTBALL_API AMF_Spectator : public ASpectatorPawn
{
    GENERATED_BODY()

public:
    AMF_Spectator();

    // ==================== Components ====================

    /** Camera boom for the spectator camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* CameraBoom;

    /** Spectator camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* SpectatorCamera;

    // ==================== Spectator Settings ====================

    /** Whether to automatically follow the ball */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator")
    bool bFollowBall;

    /** Speed multiplier for camera movement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator")
    float CameraSpeed;

    /** Height offset for the spectator camera */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator")
    float CameraHeight;

    /** How smooth the camera follows the ball (0 = instant, higher = smoother) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator")
    float CameraFollowSmoothness;

    // ==================== Functions ====================

    /** Toggle ball following mode */
    UFUNCTION(BlueprintCallable, Category = "Spectator")
    void ToggleFollowBall();

    /** Set ball following mode */
    UFUNCTION(BlueprintCallable, Category = "Spectator")
    void SetFollowBall(bool bFollow);

    /** Move to a specific location on the field */
    UFUNCTION(BlueprintCallable, Category = "Spectator")
    void MoveToLocation(FVector Location);

    /** Get the ball reference */
    UFUNCTION(BlueprintPure, Category = "Spectator")
    AMF_Ball* GetBall() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
    /** Cached ball reference */
    UPROPERTY()
    TWeakObjectPtr<AMF_Ball> CachedBall;

    /** Find the ball in the world */
    void FindBall();

    /** Update camera position when following ball */
    void UpdateBallFollow(float DeltaTime);
};
