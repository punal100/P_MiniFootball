/*
 * @Author: Punal Manalan
 * @Description: MF_InputHandler - Implementation
 * @Date: 07/12/2025
 */

#include "Player/MF_InputHandler.h"
#include "GameFramework/PlayerController.h"
#include "Manager/CPP_InputBindingManager.h"
#include "Integration/CPP_EnhancedInputIntegration.h"
#include "InputBinding/FS_InputActionBinding.h"
#include "InputBinding/FS_InputAxisBinding.h"

UMF_InputHandler::UMF_InputHandler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UMF_InputHandler::BeginPlay()
{
    Super::BeginPlay();
}

void UMF_InputHandler::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CleanupInput();
    Super::EndPlay(EndPlayReason);
}

void UMF_InputHandler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Track action hold time
    if (bIsActionHeld)
    {
        ActionHoldTime += DeltaTime;
        OnActionHeld.Broadcast(ActionHoldTime);
    }
}

bool UMF_InputHandler::InitializeInput(APlayerController *PC)
{
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_InputHandler: Cannot initialize - null PlayerController"));
        return false;
    }

    // Only initialize on local player (owning client or standalone)
    if (!PC->IsLocalController())
    {
        UE_LOG(LogTemp, Log, TEXT("MF_InputHandler: Skipping initialization - not local controller"));
        return false;
    }

    OwningController = PC;

    // Get P_MEIS Manager (Engine Subsystem)
    UCPP_InputBindingManager *Manager = GEngine->GetEngineSubsystem<UCPP_InputBindingManager>();
    if (!Manager)
    {
        UE_LOG(LogTemp, Error, TEXT("MF_InputHandler: Failed to get P_MEIS InputBindingManager"));
        return false;
    }

    // Register player with P_MEIS (creates profile + integration)
    Integration = Manager->RegisterPlayer(PC);
    if (!Integration)
    {
        UE_LOG(LogTemp, Error, TEXT("MF_InputHandler: Failed to register player with P_MEIS"));
        return false;
    }

    // Setup default bindings
    SetupDefaultBindings();

    // Bind to P_MEIS events
    BindP_MEISEvents();

    // Apply bindings to Enhanced Input
    Manager->ApplyPlayerProfileToEnhancedInput(PC);

    // Try to bind pending actions (in case EnhancedInputComponent wasn't ready)
    Integration->TryBindPendingActions();

    bInputInitialized = true;
    SetComponentTickEnabled(true);

    UE_LOG(LogTemp, Log, TEXT("MF_InputHandler: Input initialized successfully for %s"), *PC->GetName());
    return true;
}

void UMF_InputHandler::CleanupInput()
{
    if (!bInputInitialized)
    {
        return;
    }

    if (OwningController.IsValid())
    {
        UCPP_InputBindingManager *Manager = GEngine->GetEngineSubsystem<UCPP_InputBindingManager>();
        if (Manager)
        {
            Manager->UnregisterPlayer(OwningController.Get());
        }
    }

    Integration = nullptr;
    bInputInitialized = false;
    SetComponentTickEnabled(false);

    UE_LOG(LogTemp, Log, TEXT("MF_InputHandler: Input cleaned up"));
}

void UMF_InputHandler::SetupDefaultBindings()
{
    if (!OwningController.IsValid())
    {
        return;
    }

    UCPP_InputBindingManager *Manager = GEngine->GetEngineSubsystem<UCPP_InputBindingManager>();
    if (!Manager)
    {
        return;
    }

    APlayerController *PC = OwningController.Get();

    // ==================== Move Action (Axis2D) ====================
    {
        FS_InputAxisBinding MoveBinding;
        MoveBinding.InputAxisName = MF_InputActions::Move;
        MoveBinding.DisplayName = FText::FromString(TEXT("Move"));
        MoveBinding.DeadZone = 0.2f;
        MoveBinding.Sensitivity = 1.0f;

        // WASD for keyboard - add as axis bindings
        FS_AxisKeyBinding BindD;
        BindD.Key = EKeys::D;
        BindD.Scale = 1.0f;
        FS_AxisKeyBinding BindA;
        BindA.Key = EKeys::A;
        BindA.Scale = -1.0f;
        FS_AxisKeyBinding BindW;
        BindW.Key = EKeys::W;
        BindW.Scale = 1.0f;
        FS_AxisKeyBinding BindS;
        BindS.Key = EKeys::S;
        BindS.Scale = -1.0f;

        // Gamepad left stick
        FS_AxisKeyBinding BindStick;
        BindStick.Key = EKeys::Gamepad_LeftX;
        BindStick.Scale = 1.0f;
        FS_AxisKeyBinding BindStickY;
        BindStickY.Key = EKeys::Gamepad_LeftY;
        BindStickY.Scale = 1.0f;

        MoveBinding.AxisBindings.Add(BindD);
        MoveBinding.AxisBindings.Add(BindA);
        MoveBinding.AxisBindings.Add(BindW);
        MoveBinding.AxisBindings.Add(BindS);
        MoveBinding.AxisBindings.Add(BindStick);
        MoveBinding.AxisBindings.Add(BindStickY);

        Manager->SetPlayerAxisBinding(PC, MF_InputActions::Move, MoveBinding);
    }

    // ==================== Action Button (Shoot/Pass/Tackle) ====================
    {
        FS_InputActionBinding ActionBinding;
        ActionBinding.InputActionName = MF_InputActions::Action;
        ActionBinding.DisplayName = FText::FromString(TEXT("Action"));

        // Space for keyboard, Face Button Bottom (A/X) for gamepad
        FS_KeyBinding KeySpace;
        KeySpace.Key = EKeys::SpaceBar;
        FS_KeyBinding KeyGamepad;
        KeyGamepad.Key = EKeys::Gamepad_FaceButton_Bottom;
        ActionBinding.KeyBindings.Add(KeySpace);
        ActionBinding.KeyBindings.Add(KeyGamepad);

        Manager->SetPlayerActionBinding(PC, MF_InputActions::Action, ActionBinding);
    }

    // ==================== Sprint ====================
    {
        FS_InputActionBinding SprintBinding;
        SprintBinding.InputActionName = MF_InputActions::Sprint;
        SprintBinding.DisplayName = FText::FromString(TEXT("Sprint"));

        // Shift for keyboard, Right Trigger for gamepad
        FS_KeyBinding KeyShift;
        KeyShift.Key = EKeys::LeftShift;
        FS_KeyBinding KeyTrigger;
        KeyTrigger.Key = EKeys::Gamepad_RightTrigger;
        SprintBinding.KeyBindings.Add(KeyShift);
        SprintBinding.KeyBindings.Add(KeyTrigger);

        Manager->SetPlayerActionBinding(PC, MF_InputActions::Sprint, SprintBinding);
    }

    // ==================== Switch Player (Optional) ====================
    {
        FS_InputActionBinding SwitchBinding;
        SwitchBinding.InputActionName = MF_InputActions::SwitchPlayer;
        SwitchBinding.DisplayName = FText::FromString(TEXT("Switch Player"));

        // Q for keyboard, LB for gamepad
        FS_KeyBinding KeyQ;
        KeyQ.Key = EKeys::Q;
        FS_KeyBinding KeyLB;
        KeyLB.Key = EKeys::Gamepad_LeftShoulder;
        SwitchBinding.KeyBindings.Add(KeyQ);
        SwitchBinding.KeyBindings.Add(KeyLB);

        Manager->SetPlayerActionBinding(PC, MF_InputActions::SwitchPlayer, SwitchBinding);
    }

    // ==================== Pause ====================
    {
        FS_InputActionBinding PauseBinding;
        PauseBinding.InputActionName = MF_InputActions::Pause;
        PauseBinding.DisplayName = FText::FromString(TEXT("Pause"));

        // Escape for keyboard, Start for gamepad
        FS_KeyBinding KeyEsc;
        KeyEsc.Key = EKeys::Escape;
        FS_KeyBinding KeyStart;
        KeyStart.Key = EKeys::Gamepad_Special_Right;
        PauseBinding.KeyBindings.Add(KeyEsc);
        PauseBinding.KeyBindings.Add(KeyStart);

        Manager->SetPlayerActionBinding(PC, MF_InputActions::Pause, PauseBinding);
    }

    UE_LOG(LogTemp, Log, TEXT("MF_InputHandler: Default bindings setup complete"));
}

void UMF_InputHandler::BindP_MEISEvents()
{
    if (!Integration)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_InputHandler: Cannot bind events - no Integration"));
        return;
    }

    // Bind to global P_MEIS events and filter by action name
    Integration->OnActionTriggered.AddDynamic(this, &UMF_InputHandler::HandleMoveAction);
    Integration->OnActionTriggered.AddDynamic(this, &UMF_InputHandler::HandleActionTriggered);
    Integration->OnActionTriggered.AddDynamic(this, &UMF_InputHandler::HandleSprintAction);
    Integration->OnActionTriggered.AddDynamic(this, &UMF_InputHandler::HandleSwitchPlayerAction);
    Integration->OnActionTriggered.AddDynamic(this, &UMF_InputHandler::HandlePauseAction);

    // Started/Completed for action hold detection
    Integration->OnActionStarted.AddDynamic(this, &UMF_InputHandler::HandleActionStarted);
    Integration->OnActionCompleted.AddDynamic(this, &UMF_InputHandler::HandleActionCompleted);

    UE_LOG(LogTemp, Log, TEXT("MF_InputHandler: P_MEIS events bound"));
}

void UMF_InputHandler::HandleMoveAction(FName ActionName, FInputActionValue Value)
{
    if (ActionName != MF_InputActions::Move)
    {
        return;
    }

    CurrentMoveInput = Value.Get<FVector2D>();
    OnMoveInput.Broadcast(CurrentMoveInput);
}

void UMF_InputHandler::HandleActionTriggered(FName ActionName, FInputActionValue Value)
{
    if (ActionName != MF_InputActions::Action)
    {
        return;
    }

    // Triggered fires continuously while held for some trigger types
    // We track state via Started/Completed
}

void UMF_InputHandler::HandleActionStarted(FName ActionName, FInputActionValue Value)
{
    if (ActionName != MF_InputActions::Action)
    {
        return;
    }

    bIsActionHeld = true;
    ActionHoldTime = 0.0f;
    OnActionPressed.Broadcast(true);
}

void UMF_InputHandler::HandleActionCompleted(FName ActionName, FInputActionValue Value)
{
    if (ActionName != MF_InputActions::Action)
    {
        return;
    }

    if (bIsActionHeld)
    {
        bIsActionHeld = false;
        OnActionReleased.Broadcast();
    }
}

void UMF_InputHandler::HandleSprintAction(FName ActionName, FInputActionValue Value)
{
    if (ActionName != MF_InputActions::Sprint)
    {
        return;
    }

    bool bNewSprinting = Value.Get<bool>();
    if (bNewSprinting != bIsSprinting)
    {
        bIsSprinting = bNewSprinting;
        OnSprintInput.Broadcast(bIsSprinting);
    }
}

void UMF_InputHandler::HandleSwitchPlayerAction(FName ActionName, FInputActionValue Value)
{
    if (ActionName != MF_InputActions::SwitchPlayer)
    {
        return;
    }

    if (Value.Get<bool>())
    {
        OnSwitchPlayerInput.Broadcast();
    }
}

void UMF_InputHandler::HandlePauseAction(FName ActionName, FInputActionValue Value)
{
    if (ActionName != MF_InputActions::Pause)
    {
        return;
    }

    if (Value.Get<bool>())
    {
        OnPauseInput.Broadcast();
    }
}
