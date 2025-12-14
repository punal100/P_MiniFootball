/*
 * @Author: Punal Manalan
 * @Description: MF_InputHandler - P_MEIS Integration Component for Mini Football
 *               Handles all input setup and action binding via P_MEIS system
 *               Works on owning client, sends commands to server via RPC
 * @Date: 07/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "Core/MF_Types.h"
#include "MF_InputHandler.generated.h"

class APlayerController;
class UCPP_InputBindingManager;
class UCPP_EnhancedInputIntegration;

// ==================== Input Event Delegates ====================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMF_MoveInput, FVector2D, MoveValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMF_ActionInput, bool, bIsPressed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMF_ActionHeld, float, HoldTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMF_ActionReleased);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMF_SprintInput, bool, bIsSprinting);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMF_SwitchPlayerInput);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMF_PauseInput);

/**
 * MF_InputHandler
 * Integrates with P_MEIS to handle all Mini Football input
 *
 * Usage:
 * 1. Add this component to PlayerController or Character
 * 2. Call InitializeInput() after possession
 * 3. Bind to delegates (OnMoveInput, OnActionInput, etc.)
 * 4. Input flows: P_MEIS -> This Handler -> Delegates -> Character/Controller
 */
UCLASS(ClassGroup = (MiniFootball), meta = (BlueprintSpawnableComponent))
class P_MINIFOOTBALL_API UMF_InputHandler : public UActorComponent
{
    GENERATED_BODY()

public:
    UMF_InputHandler();

    // ==================== Initialization ====================

    /**
     * Initialize input system for the given player controller
     * Creates P_MEIS profile and binds all input actions
     * @param PC The player controller to setup input for
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "MiniFootball|Input")
    bool InitializeInput(APlayerController *PC);

    /**
     * Cleanup input bindings
     * Call when player is unpossessed or destroyed
     */
    UFUNCTION(BlueprintCallable, Category = "MiniFootball|Input")
    void CleanupInput();

    /**
     * Check if input is initialized and ready
     */
    UFUNCTION(BlueprintPure, Category = "MiniFootball|Input")
    bool IsInputReady() const { return bInputInitialized; }

    // ==================== Input Events (Bind to these) ====================

    /** Fires when move input changes (joystick/WASD) */
    UPROPERTY(BlueprintAssignable, Category = "MiniFootball|Input Events")
    FOnMF_MoveInput OnMoveInput;

    /** Fires when action button is pressed */
    UPROPERTY(BlueprintAssignable, Category = "MiniFootball|Input Events")
    FOnMF_ActionInput OnActionPressed;

    /** Fires while action button is held (for pass charging) */
    UPROPERTY(BlueprintAssignable, Category = "MiniFootball|Input Events")
    FOnMF_ActionHeld OnActionHeld;

    /** Fires when action button is released */
    UPROPERTY(BlueprintAssignable, Category = "MiniFootball|Input Events")
    FOnMF_ActionReleased OnActionReleased;

    /** Fires when sprint state changes */
    UPROPERTY(BlueprintAssignable, Category = "MiniFootball|Input Events")
    FOnMF_SprintInput OnSprintInput;

    /** Fires when switch player is pressed */
    UPROPERTY(BlueprintAssignable, Category = "MiniFootball|Input Events")
    FOnMF_SwitchPlayerInput OnSwitchPlayerInput;

    /** Fires when pause is pressed */
    UPROPERTY(BlueprintAssignable, Category = "MiniFootball|Input Events")
    FOnMF_PauseInput OnPauseInput;

    // ==================== Input State Getters ====================

    /** Get current move input value */
    UFUNCTION(BlueprintPure, Category = "MiniFootball|Input")
    FVector2D GetMoveInput() const { return CurrentMoveInput; }

    /** Check if sprint is currently held */
    UFUNCTION(BlueprintPure, Category = "MiniFootball|Input")
    bool IsSprinting() const { return bIsSprinting; }

    /** Check if action is currently held */
    UFUNCTION(BlueprintPure, Category = "MiniFootball|Input")
    bool IsActionHeld() const { return bIsActionHeld; }

    /** Get how long action has been held */
    UFUNCTION(BlueprintPure, Category = "MiniFootball|Input")
    float GetActionHoldTime() const { return ActionHoldTime; }

    /** Get the P_MEIS Integration object (for deferred binding) */
    UCPP_EnhancedInputIntegration *GetIntegration() const { return Integration; }

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

private:
    // ==================== P_MEIS References ====================

    UPROPERTY()
    TWeakObjectPtr<APlayerController> OwningController;

    UPROPERTY()
    UCPP_EnhancedInputIntegration *Integration;

    // ==================== Input State ====================

    bool bInputInitialized = false;

    FVector2D CurrentMoveInput = FVector2D::ZeroVector;
    bool bIsSprinting = false;
    bool bIsActionHeld = false;
    float ActionHoldTime = 0.0f;

    // ==================== Internal Handlers ====================

    /** Setup default input bindings via P_MEIS */
    void SetupDefaultBindings();

    /** Bind to P_MEIS events */
    void BindP_MEISEvents();

    // P_MEIS Event Handlers
    UFUNCTION()
    void HandleMoveAction(FName ActionName, FInputActionValue Value);

    UFUNCTION()
    void HandleActionTriggered(FName ActionName, FInputActionValue Value);

    UFUNCTION()
    void HandleActionStarted(FName ActionName, FInputActionValue Value);

    UFUNCTION()
    void HandleActionCompleted(FName ActionName, FInputActionValue Value);

    UFUNCTION()
    void HandleMoveCompleted(FName ActionName, FInputActionValue Value);

    UFUNCTION()
    void HandleSprintAction(FName ActionName, FInputActionValue Value);

    UFUNCTION()
    void HandleSprintStarted(FName ActionName, FInputActionValue Value);

    UFUNCTION()
    void HandleSprintCompleted(FName ActionName, FInputActionValue Value);

    UFUNCTION()
    void HandleSwitchPlayerAction(FName ActionName, FInputActionValue Value);

    UFUNCTION()
    void HandlePauseAction(FName ActionName, FInputActionValue Value);
};
