/*
 * @Author: Punal Manalan
 * @Description: MF_PlayerController - Implementation
 *               Full network replication for Listen Server and Dedicated Server
 *               Includes spectator system and team request RPCs
 * @Date: 07/12/2025
 * @Updated: 09/12/2025 - Added spectator state and team request RPCs
 */

#include "Player/MF_PlayerController.h"
#include "Player/MF_PlayerCharacter.h"
#include "Player/MF_Spectator.h"
#include "Ball/MF_Ball.h"
#include "Match/MF_GameMode.h"
#include "UI/MF_HUD.h"
#include "MF_Utilities.h"
#include "Interfaces/MF_TeamInterface.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"

#include "Engine/Engine.h"

#include "Manager/CPP_InputBindingManager.h"
#include "Integration/CPP_EnhancedInputIntegration.h"

#include "InputCoreTypes.h"

#include "Input/MF_DefaultInputTemplates.h"

#include "MF_HUD.h"

#include "UI/Configuration/MF_WidgetConfigurationSubsystem.h"
#include "UI/Configuration/MF_WidgetTypes.h"

#include "Blueprint/WidgetBlueprintLibrary.h"

#include "EnhancedInputComponent.h"

using namespace MF_Utilities;

namespace
{
    static const FName MF_DefaultInputTemplateName(TEXT("Default"));
}

AMF_PlayerController::AMF_PlayerController()
{
    bReplicates = true;
    AssignedTeam = EMF_TeamID::None;
    ActiveCharacterIndex = -1;
    bIsSpectator = true;                                    // Start as spectator by default
    CurrentSpectatorState = EMF_SpectatorState::Spectating; // Start spectating
}

void AMF_PlayerController::CreateInputComponent(TSubclassOf<UInputComponent> InputComponentToCreate)
{
    // Always prefer Enhanced Input on controllers (stable across pawn switching)
    Super::CreateInputComponent(UEnhancedInputComponent::StaticClass());
}

void AMF_PlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
   
    // === ENHANCED LOGGING FOR CLIENT INPUT DEBUG ===
    UE_LOG(LogTemp, Warning, TEXT("=== MF_PlayerController::SetupInputComponent ==="));
    UE_LOG(LogTemp, Warning, TEXT("  Controller: %s"), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("  IsLocalController: %d"), IsLocalController());
    UE_LOG(LogTemp, Warning, TEXT("  InputComponent: %s"), *GetNameSafe(InputComponent));
    UE_LOG(LogTemp, Warning, TEXT("  bInputSystemInitialized: %d"), bInputSystemInitialized);
    
    if (!IsLocalController())
    {
        UE_LOG(LogTemp, Warning, TEXT("  → Not local controller, skipping deferred binding"));
        return;
    }
    
    if (!InputComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("  → InputComponent is NULL after SetupInputComponent!"));
        return;
    }
    
    // Attempt deferred binding if P_MEIS integration exists
    UCPP_InputBindingManager* Manager = GetMEISManager();
    if (Manager)
    {
        if (UCPP_EnhancedInputIntegration* Integration = Manager->GetIntegrationForPlayer(this))
        {
            UE_LOG(LogTemp, Warning, TEXT("  → Attempting deferred P_MEIS bindings..."));
            int32 BoundCount = Integration->TryBindPendingActions();
            UE_LOG(LogTemp, Warning, TEXT("  → Bound %d pending actions"), BoundCount);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("  → No Integration found (will be created on first input profile load)"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("  → No P_MEIS Manager found!"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== MF_PlayerController::SetupInputComponent END ==="));
}


void AMF_PlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMF_PlayerController, AssignedTeam);
    DOREPLIFETIME(AMF_PlayerController, TeamCharacters);
    DOREPLIFETIME(AMF_PlayerController, ActiveCharacterIndex);
    DOREPLIFETIME(AMF_PlayerController, bIsSpectator);
    DOREPLIFETIME(AMF_PlayerController, CurrentSpectatorState);
}

void AMF_PlayerController::BeginPlay()
{
    Super::BeginPlay();

    // === ENHANCED LOGGING FOR CLIENT INPUT DEBUG ===
    UE_LOG(LogTemp, Warning, TEXT("=== MF_PlayerController::BeginPlay ==="));
    UE_LOG(LogTemp, Warning, TEXT("  Controller: %s"), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("  LocalRole: %d"), static_cast<int32>(GetLocalRole()));
    UE_LOG(LogTemp, Warning, TEXT("  RemoteRole: %d"), static_cast<int32>(GetRemoteRole()));
    UE_LOG(LogTemp, Warning, TEXT("  IsLocalController: %d"), IsLocalController());
    UE_LOG(LogTemp, Warning, TEXT("  NetMode: %d"), static_cast<int32>(GetNetMode()));
    UE_LOG(LogTemp, Warning, TEXT("  HasAuthority: %d"), HasAuthority());
    UE_LOG(LogTemp, Warning, TEXT("  InputComponent: %s"), InputComponent ? TEXT("EXISTS") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("  SpectatorState: %d"), static_cast<int32>(CurrentSpectatorState));

    // CRITICAL: Only initialize input on LOCAL controllers
    // On each client machine, THEIR PlayerController IS local (IsLocalController() = true)
    if (IsLocalController())
    {
        UE_LOG(LogTemp, Warning, TEXT("  → LOCAL CONTROLLER - Initializing P_MEIS input system"));
        
        // One-call path: ensures a usable template exists and gets applied.
        bool bInputReady = EnsureInputProfileReady(MF_DefaultInputTemplateName, /*bCreateTemplateIfMissing*/ true, /*bApplyEvenIfNotEmpty*/ false);
        UE_LOG(LogTemp, Warning, TEXT("  → EnsureInputProfileReady result: %d"), bInputReady);

        // Ensure a HUD exists early (safe: reuses existing HUD if already created in BP).
        CreateSpectatorUI();
    }
    else
    {
        // This should NOT appear on the client machine for the client's own PlayerController
        // If you see this log on a client, there's a network/replication bug
        UE_LOG(LogTemp, Error, TEXT("  → NOT LOCAL CONTROLLER - Skipping input init (this is expected ONLY on server for remote players)"));
    }

    UpdatePlayerRole();
    UE_LOG(LogTemp, Warning, TEXT("=== MF_PlayerController::BeginPlay END ==="));
}

bool AMF_PlayerController::EnsureInputProfileReady(const FName TemplateName, bool bCreateTemplateIfMissing, bool bApplyEvenIfNotEmpty)
{
    UE_LOG(LogTemp, Warning, TEXT("=== MF_PlayerController::EnsureInputProfileReady ==="));
    UE_LOG(LogTemp, Warning, TEXT("  Controller: %s"), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("  TemplateName: %s"), *TemplateName.ToString());
    UE_LOG(LogTemp, Warning, TEXT("  bCreateTemplateIfMissing: %d"), bCreateTemplateIfMissing);
    UE_LOG(LogTemp, Warning, TEXT("  bApplyEvenIfNotEmpty: %d"), bApplyEvenIfNotEmpty);
    
    if (!IsLocalController())
    {
        UE_LOG(LogTemp, Error, TEXT("  → NOT LOCAL CONTROLLER - Aborting (this is a bug if called from client's own PC!)"));
        return false;
    }

    UCPP_InputBindingManager *Manager = GetMEISManager();
    if (!Manager)
    {
        UE_LOG(LogTemp, Error, TEXT("  → No MEIS Manager found!"));
        return false;
    }
    UE_LOG(LogTemp, Log, TEXT("  → MEIS Manager: %s"), *GetNameSafe(Manager));

    // Ensure registered.
    if (!Manager->HasPlayerRegistered(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("  → Player NOT registered, registering now..."));
        UCPP_EnhancedInputIntegration* Integration = Manager->RegisterPlayer(this);
        UE_LOG(LogTemp, Warning, TEXT("  → Registered, Integration: %s"), *GetNameSafe(Integration));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("  → Player already registered"));
    }

    // Decide which template name to use.
    const FName EffectiveTemplateName = TemplateName.IsNone() ? MF_DefaultInputTemplateName : TemplateName;
    UE_LOG(LogTemp, Log, TEXT("  → Effective template name: %s"), *EffectiveTemplateName.ToString());

    // If missing, optionally create the built-in Default template.
    if (bCreateTemplateIfMissing && !Manager->DoesTemplateExist(EffectiveTemplateName))
    {
        UE_LOG(LogTemp, Warning, TEXT("  → Template doesn't exist, creating..."));
        if (EffectiveTemplateName == MF_DefaultInputTemplateName)
        {
            const FS_InputProfile DefaultTemplate = MF_DefaultInputTemplates::BuildDefaultInputTemplate(MF_DefaultInputTemplateName);
            Manager->SaveProfileTemplate(MF_DefaultInputTemplateName, DefaultTemplate);
            UE_LOG(LogTemp, Warning, TEXT("  → Created default template with %d actions"), DefaultTemplate.ActionBindings.Num());
        }
    }

    // Apply only if requested, or if profile is empty.
    bool bShouldApply = bApplyEvenIfNotEmpty;
    if (!bShouldApply)
    {
        const FS_InputProfile CurrentProfile = Manager->GetProfileForPlayer(this);
        bShouldApply = (CurrentProfile.ActionBindings.Num() == 0 && CurrentProfile.AxisBindings.Num() == 0);
        UE_LOG(LogTemp, Log, TEXT("  → Current profile has %d actions, %d axes, bShouldApply: %d"), 
            CurrentProfile.ActionBindings.Num(), CurrentProfile.AxisBindings.Num(), bShouldApply);
    }

    if (bShouldApply)
    {
        UE_LOG(LogTemp, Warning, TEXT("  → Applying template to player..."));
        if (!Manager->ApplyTemplateToPlayer(this, EffectiveTemplateName))
        {
            UE_LOG(LogTemp, Error, TEXT("  → FAILED to apply template!"));
            return false;
        }
        UE_LOG(LogTemp, Warning, TEXT("  → Template applied successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("  → Skipping template apply (profile not empty)"));
        // Even if we didn't re-apply, ensure Enhanced Input reflects current profile.
        Manager->ApplyPlayerProfileToEnhancedInput(this);
    }

    bInputSystemInitialized = true;
    bInputProfileLoaded = true;
    UE_LOG(LogTemp, Warning, TEXT("=== MF_PlayerController::EnsureInputProfileReady SUCCESS ==="));
    return true;
}

void AMF_PlayerController::OnPossess(APawn *InPawn)
{
    Super::OnPossess(InPawn);

    UE_LOG(LogTemp, Warning, TEXT("MF_PlayerController::OnPossess - Pawn: %s, IsLocalController: %d, HasAuthority: %d, Role: %s"),
           InPawn ? *InPawn->GetName() : TEXT("null"),
           IsLocalController(),
           HasAuthority(),
           *UEnum::GetValueAsString(GetLocalRole()));

    OnMFPossessedPawnChanged.Broadcast(this, InPawn);
    Execute_OnPossessedPawnChanged(this, InPawn);
    UpdatePlayerRole();
}

void AMF_PlayerController::OnUnPossess()
{
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::OnUnPossess"));
    Super::OnUnPossess();

    OnMFPossessedPawnChanged.Broadcast(this, nullptr);
    Execute_OnPossessedPawnChanged(this, nullptr);
    UpdatePlayerRole();
}

// ==================== Team Management ====================

void AMF_PlayerController::AssignToTeam(EMF_TeamID NewTeam)
{
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerController::AssignToTeam - Called on client, ignoring"));
        return;
    }

    if (AssignedTeam != NewTeam)
    {
        AssignedTeam = NewTeam;
        OnRep_AssignedTeam();

        UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::AssignToTeam - Assigned to team: %d"),
               static_cast<int32>(NewTeam));
    }
}

void AMF_PlayerController::OnRep_AssignedTeam()
{
    OnTeamAssigned.Broadcast(this, AssignedTeam);
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::OnRep_AssignedTeam - Team: %d"),
           static_cast<int32>(AssignedTeam));

    UpdatePlayerRole();
}

void AMF_PlayerController::OnRep_SpectatorState()
{
    // Update bIsSpectator for backwards compatibility
    bIsSpectator = (CurrentSpectatorState == EMF_SpectatorState::Spectating);

    OnSpectatorStateChanged.Broadcast(this, CurrentSpectatorState);

    // Call interface method
    Execute_OnTeamStateChanged(this, AssignedTeam, CurrentSpectatorState);

    UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::OnRep_SpectatorState - State: %d"),
           static_cast<int32>(CurrentSpectatorState));

    UpdatePlayerRole();
}

void AMF_PlayerController::InitializeInputSystem()
{
    if (bInputSystemInitialized)
    {
        return;
    }

    UCPP_InputBindingManager *Manager = GetMEISManager();
    if (!Manager)
    {
        return;
    }

    if (!Manager->HasPlayerRegistered(this))
    {
        Manager->RegisterPlayer(this);
    }

    bInputSystemInitialized = true;
}

void AMF_PlayerController::LoadInputProfile(const FName &TemplateName)
{
    if (bInputProfileLoaded)
    {
        return;
    }

    UCPP_InputBindingManager *Manager = GetMEISManager();
    if (!Manager)
    {
        return;
    }

    // Ensure registered first.
    if (!Manager->HasPlayerRegistered(this))
    {
        Manager->RegisterPlayer(this);
        bInputSystemInitialized = true;
    }

    // If profile is empty, try to load a template.
    if (FS_InputProfile *Profile = Manager->GetProfileRefForPlayer(this))
    {
        if (Profile->ActionBindings.Num() == 0 && Profile->AxisBindings.Num() == 0)
        {
            if (!TemplateName.IsNone())
            {
                Manager->ApplyTemplateToPlayer(this, TemplateName);
            }
        }
    }

    bInputProfileLoaded = true;
}

void AMF_PlayerController::FinalizeInputSetup()
{
    UCPP_InputBindingManager *Manager = GetMEISManager();
    if (!Manager)
    {
        return;
    }

    // Apply active profile to Enhanced Input.
    Manager->ApplyPlayerProfileToEnhancedInput(this);
}

void AMF_PlayerController::UpdatePlayerRole()
{
    const bool bIsPlayingNow = (AssignedTeam != EMF_TeamID::None) &&
                               (CurrentSpectatorState == EMF_SpectatorState::Playing) &&
                               (GetPawn() != nullptr);

    if (bIsPlayingNow != bLastKnownIsPlaying)
    {
        bLastKnownIsPlaying = bIsPlayingNow;
        OnPlayerRoleChanged.Broadcast(this, bIsPlayingNow);
    }
}

void AMF_PlayerController::HandlePlayerRoleChanged(AMF_PlayerController *Controller, bool bIsPlaying)
{
    if (Controller != this)
    {
        return;
    }

    if (CurrentHUD)
    {
        CurrentHUD->SetHUDMode(bIsPlaying ? EMF_HUDMode::Gameplay : EMF_HUDMode::Spectator);
        CurrentHUD->RefreshFromPlayerState();
    }
}

static TSubclassOf<UMF_HUD> ResolveHUDClass()
{
    if (GEngine)
    {
        if (UMF_WidgetConfigurationSubsystem *WidgetConfig = GEngine->GetEngineSubsystem<UMF_WidgetConfigurationSubsystem>())
        {
            const TSubclassOf<UUserWidget> Resolved = WidgetConfig->GetWidgetClass(EMF_WidgetType::MainHUD);
            if (Resolved)
            {
                return Resolved.Get();
            }
        }
    }

    return UMF_HUD::StaticClass();
}

void AMF_PlayerController::CreateGameplayUI_Implementation()
{
    if (!IsLocalController())
    {
        return;
    }

    CreateSpectatorUI();
    HandlePlayerRoleChanged(this, true);
}

void AMF_PlayerController::CreateSpectatorUI_Implementation()
{
    if (!IsLocalController())
    {
        return;
    }

    // Reuse an already-created HUD if Blueprint or another system spawned it.
    if (!CurrentHUD)
    {
        TArray<UUserWidget *> Found;
        UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, Found, UMF_HUD::StaticClass(), false);
        if (Found.Num() > 0)
        {
            CurrentHUD = Cast<UMF_HUD>(Found[0]);
        }
    }

    if (!CurrentHUD)
    {
        const TSubclassOf<UMF_HUD> HUDClass = ResolveHUDClass();
        CurrentHUD = CreateWidget<UMF_HUD>(this, HUDClass);
        if (CurrentHUD)
        {
            CurrentHUD->AddToViewport(HUDZOrder);
        }
    }

    // Bind role changes once.
    if (!OnPlayerRoleChanged.IsAlreadyBound(this, &AMF_PlayerController::HandlePlayerRoleChanged))
    {
        OnPlayerRoleChanged.AddDynamic(this, &AMF_PlayerController::HandlePlayerRoleChanged);
    }

    HandlePlayerRoleChanged(this, false);
}

void AMF_PlayerController::ClearUI_Implementation()
{
    if (CurrentHUD)
    {
        CurrentHUD->RemoveFromParent();
        CurrentHUD = nullptr;
    }
}

void AMF_PlayerController::SetSpectatorState(EMF_SpectatorState NewState)
{
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerController::SetSpectatorState - Called on client, ignoring"));
        return;
    }

    if (CurrentSpectatorState != NewState)
    {
        CurrentSpectatorState = NewState;
        bIsSpectator = (NewState == EMF_SpectatorState::Spectating);
        OnRep_SpectatorState();

        UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::SetSpectatorState - Set to: %d"),
               static_cast<int32>(NewState));
    }
}

// ==================== Team Request Server RPCs ====================

void AMF_PlayerController::Server_RequestJoinTeam_Implementation(EMF_TeamID RequestedTeam)
{
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::Server_RequestJoinTeam - Player %s requesting team %d"),
           *GetName(), static_cast<int32>(RequestedTeam));

    // Set transitioning state
    SetSpectatorState(EMF_SpectatorState::Transitioning);

    // Get GameMode and check if it implements the team interface
    AGameModeBase *GM = GetWorld()->GetAuthGameMode();
    if (!GM)
    {
        UE_LOG(LogTemp, Error, TEXT("MF_PlayerController::Server_RequestJoinTeam - No GameMode found"));
        Client_OnTeamAssignmentResponse(false, EMF_TeamID::None, TEXT("Server error: No GameMode"));
        SetSpectatorState(EMF_SpectatorState::Spectating);
        return;
    }

    // Check if GameMode implements IMF_TeamInterface
    if (!GM->GetClass()->ImplementsInterface(UMF_TeamInterface::StaticClass()))
    {
        UE_LOG(LogTemp, Error, TEXT("MF_PlayerController::Server_RequestJoinTeam - GameMode does not implement IMF_TeamInterface"));
        Client_OnTeamAssignmentResponse(false, EMF_TeamID::None, TEXT("Server error: Team system not available"));
        SetSpectatorState(EMF_SpectatorState::Spectating);
        return;
    }

    // Call the interface function
    FMF_TeamAssignmentResult Result = IMF_TeamInterface::Execute_HandleJoinTeamRequest(GM, this, RequestedTeam);

    if (Result.bSuccess)
    {
        // Success - update state
        AssignToTeam(Result.AssignedTeam);
        SetSpectatorState(EMF_SpectatorState::Playing);
        Client_OnTeamAssignmentResponse(true, Result.AssignedTeam, TEXT(""));

        UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::Server_RequestJoinTeam - SUCCESS: Player %s joined team %d"),
               *GetName(), static_cast<int32>(Result.AssignedTeam));
    }
    else
    {
        // Failed - return to spectating
        SetSpectatorState(EMF_SpectatorState::Spectating);
        Client_OnTeamAssignmentResponse(false, EMF_TeamID::None, Result.ErrorMessage);

        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerController::Server_RequestJoinTeam - FAILED: %s"),
               *Result.ErrorMessage);
    }
}

void AMF_PlayerController::Server_RequestLeaveTeam_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::Server_RequestLeaveTeam - Player %s requesting to leave team"),
           *GetName());

    if (AssignedTeam == EMF_TeamID::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerController::Server_RequestLeaveTeam - Player not on a team"));
        return;
    }

    // Set transitioning state
    SetSpectatorState(EMF_SpectatorState::Transitioning);

    // Get GameMode and check if it implements the team interface
    AGameModeBase *GM = GetWorld()->GetAuthGameMode();
    if (GM && GM->GetClass()->ImplementsInterface(UMF_TeamInterface::StaticClass()))
    {
        // Call the interface function
        bool bSuccess = IMF_TeamInterface::Execute_HandleLeaveTeamRequest(GM, this);

        if (bSuccess)
        {
            // Clear team assignment
            AssignToTeam(EMF_TeamID::None);
            SetSpectatorState(EMF_SpectatorState::Spectating);
            Client_OnTeamAssignmentResponse(true, EMF_TeamID::None, TEXT("Left team successfully"));

            UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::Server_RequestLeaveTeam - SUCCESS"));
        }
        else
        {
            // Failed to leave - restore playing state
            SetSpectatorState(EMF_SpectatorState::Playing);
            Client_OnTeamAssignmentResponse(false, AssignedTeam, TEXT("Failed to leave team"));

            UE_LOG(LogTemp, Warning, TEXT("MF_PlayerController::Server_RequestLeaveTeam - FAILED"));
        }
    }
    else
    {
        // No interface, just do basic cleanup
        AssignToTeam(EMF_TeamID::None);
        SetSpectatorState(EMF_SpectatorState::Spectating);

        // Unpossess current pawn
        if (GetPawn())
        {
            UnPossess();
        }

        Client_OnTeamAssignmentResponse(true, EMF_TeamID::None, TEXT("Left team"));
    }
}

// ==================== Team Response Client RPCs ====================

void AMF_PlayerController::Client_OnTeamAssignmentResponse_Implementation(bool bSuccess, EMF_TeamID Team, const FString &ErrorMessage)
{
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::Client_OnTeamAssignmentResponse - Success: %d, Team: %d, Error: %s"),
           bSuccess, static_cast<int32>(Team), *ErrorMessage);

    // Broadcast event for widgets to listen to
    OnTeamAssignmentResponseReceived.Broadcast(bSuccess, Team, ErrorMessage);

    // Call interface method
    Execute_OnTeamAssignmentResponse(this, bSuccess, Team, ErrorMessage);
}

// ==================== Interface Implementation ====================

void AMF_PlayerController::OnTeamAssignmentResponse_Implementation(bool bSuccess, EMF_TeamID InAssignedTeam, const FString &ErrorMessage)
{
    // Default implementation - can be overridden in Blueprint
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::OnTeamAssignmentResponse_Implementation - Success: %d, Team: %d"),
           bSuccess, static_cast<int32>(InAssignedTeam));
}

void AMF_PlayerController::OnTeamStateChanged_Implementation(EMF_TeamID NewTeamID, EMF_SpectatorState NewState)
{
    // Default implementation - can be overridden in Blueprint
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::OnTeamStateChanged_Implementation - Team: %d, State: %d"),
           static_cast<int32>(NewTeamID), static_cast<int32>(NewState));
}

void AMF_PlayerController::OnPossessedPawnChanged_Implementation(APawn *NewPawn)
{
    // Default implementation - can be overridden in Blueprint
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::OnPossessedPawnChanged_Implementation - Pawn: %s"),
           NewPawn ? *NewPawn->GetName() : TEXT("None"));
}

EMF_TeamID AMF_PlayerController::GetCurrentTeamID_Implementation()
{
    return AssignedTeam;
}

EMF_SpectatorState AMF_PlayerController::GetCurrentSpectatorState_Implementation()
{
    return CurrentSpectatorState;
}

bool AMF_PlayerController::IsSpectating_Implementation()
{
    return CurrentSpectatorState == EMF_SpectatorState::Spectating;
}

// ==================== Character Management ====================

void AMF_PlayerController::RegisterTeamCharacter(AMF_PlayerCharacter *InCharacter)
{
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerController::RegisterTeamCharacter - Called on client, ignored"));
        return;
    }

    if (!InCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerController::RegisterTeamCharacter - Null character"));
        return;
    }

    if (TeamCharacters.Contains(InCharacter))
    {
        UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::RegisterTeamCharacter - Character %s already registered"),
               *InCharacter->GetName());
        return;
    }

    TeamCharacters.Add(InCharacter);

    UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::RegisterTeamCharacter - Registered %s (Total: %d, Spectator: %d, ActiveIndex: %d)"),
           *InCharacter->GetName(), TeamCharacters.Num(), bIsSpectator, ActiveCharacterIndex);

    // Auto-possess first character if not in spectator mode and not already possessing
    if (!bIsSpectator && ActiveCharacterIndex < 0 && !GetPawn())
    {
        UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::RegisterTeamCharacter - Auto-possessing first character"));
        Internal_SwitchToCharacter(TeamCharacters.Num() - 1);
    }
}

void AMF_PlayerController::UnregisterTeamCharacter(AMF_PlayerCharacter *InCharacter)
{
    if (!HasAuthority())
    {
        return;
    }

    int32 Index = TeamCharacters.IndexOfByKey(InCharacter);
    if (Index != INDEX_NONE)
    {
        TeamCharacters.RemoveAt(Index);

        // If we removed our active character, switch to another
        if (ActiveCharacterIndex == Index)
        {
            if (TeamCharacters.Num() > 0)
            {
                int32 NewIndex = FMath::Min(Index, TeamCharacters.Num() - 1);
                Internal_SwitchToCharacter(NewIndex);
            }
            else
            {
                ActiveCharacterIndex = -1;
            }
        }
        else if (ActiveCharacterIndex > Index)
        {
            // Adjust index since array shifted
            ActiveCharacterIndex--;
        }

        UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::UnregisterTeamCharacter - Unregistered %s"),
               *InCharacter->GetName());
    }
}

AMF_PlayerCharacter *AMF_PlayerController::GetCurrentCharacter() const
{
    if (ActiveCharacterIndex >= 0 && ActiveCharacterIndex < TeamCharacters.Num())
    {
        return TeamCharacters[ActiveCharacterIndex];
    }
    return nullptr;
}

void AMF_PlayerController::SwitchToCharacter(int32 CharacterIndex)
{
    if (HasAuthority())
    {
        Internal_SwitchToCharacter(CharacterIndex);
    }
    else
    {
        Server_RequestCharacterSwitch(CharacterIndex);
    }
}

// DELETED: SwitchToNextCharacter - per PLAN.md this was wrong abstraction
// Q ALWAYS switches to teammate closest to ball, no array cycling

void AMF_PlayerController::SwitchToNearestToBall()
{
    // Per PLAN.md: Q ALWAYS switches control to the teammate closest to the ball
    UE_LOG(LogTemp, Warning, TEXT("MF_PlayerController::SwitchToNearestToBall - TeamCharacters: %d, ActiveIndex: %d"),
           TeamCharacters.Num(), ActiveCharacterIndex);
    
    int32 NearestIndex = FindNearestCharacterToBall();
    UE_LOG(LogTemp, Warning, TEXT("  → NearestIndex: %d"), NearestIndex);
    
    if (NearestIndex >= 0 && NearestIndex != ActiveCharacterIndex)
    {
        SwitchToCharacter(NearestIndex);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("  → No switch: NearestIndex=%d, ActiveIndex=%d"), NearestIndex, ActiveCharacterIndex);
    }
}

int32 AMF_PlayerController::FindNearestCharacterToBall() const
{
    // Try to find ball if not cached
    if (!CachedBallActor.IsValid())
    {
        TArray<AActor *> FoundBalls;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMF_Ball::StaticClass(), FoundBalls);
        if (FoundBalls.Num() > 0)
        {
            const_cast<AMF_PlayerController *>(this)->CachedBallActor = FoundBalls[0];
        }
    }

    if (!CachedBallActor.IsValid())
    {
        // No ball found - cycle to next character instead
        if (TeamCharacters.Num() > 1)
        {
            return (ActiveCharacterIndex + 1) % TeamCharacters.Num();
        }
        return ActiveCharacterIndex;
    }

    FVector BallLocation = CachedBallActor->GetActorLocation();
    float NearestDistSq = MAX_flt;
    int32 NearestIndex = -1;

    for (int32 i = 0; i < TeamCharacters.Num(); ++i)
    {
        // SKIP the currently controlled character - Q should always switch to someone else
        if (i == ActiveCharacterIndex)
        {
            continue;
        }

        if (TeamCharacters[i])
        {
            float DistSq = FVector::DistSquared(TeamCharacters[i]->GetActorLocation(), BallLocation);
            if (DistSq < NearestDistSq)
            {
                NearestDistSq = DistSq;
                NearestIndex = i;
            }
        }
    }

    return NearestIndex;
}

// ==================== Server RPCs ====================

bool AMF_PlayerController::Server_RequestCharacterSwitch_Validate(int32 NewIndex)
{
    return NewIndex >= 0 && NewIndex < TeamCharacters.Num();
}

void AMF_PlayerController::Server_RequestCharacterSwitch_Implementation(int32 NewIndex)
{
    Internal_SwitchToCharacter(NewIndex);
}

// ==================== Client RPCs ====================

void AMF_PlayerController::Client_OnCharacterSwitched_Implementation(AMF_PlayerCharacter *NewCharacter)
{
    OnControlledCharacterChanged.Broadcast(this, NewCharacter);
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::Client_OnCharacterSwitched - Character: %s"),
           NewCharacter ? *NewCharacter->GetName() : TEXT("null"));
}

// ==================== Internal Functions ====================

void AMF_PlayerController::Internal_SwitchToCharacter(int32 CharacterIndex)
{
    if (!HasAuthority())
    {
        return;
    }

    if (CharacterIndex < 0 || CharacterIndex >= TeamCharacters.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerController::Internal_SwitchToCharacter - Invalid index: %d"),
               CharacterIndex);
        return;
    }

    AMF_PlayerCharacter *NewCharacter = TeamCharacters[CharacterIndex];
    if (!NewCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerController::Internal_SwitchToCharacter - Null character at index: %d"),
               CharacterIndex);
        return;
    }

    // Unpossess current character (don't destroy it)
    if (GetPawn() && GetPawn() != NewCharacter)
    {
        UnPossess();
    }

    // Possess new character
    Possess(NewCharacter);
    ActiveCharacterIndex = CharacterIndex;

    // Notify client
    Client_OnCharacterSwitched(NewCharacter);

    UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::Internal_SwitchToCharacter - Switched to %s (Index: %d)"),
           *NewCharacter->GetName(), CharacterIndex);
}

// ==================== Input Handling ====================

void AMF_PlayerController::RequestPlayerSwitch()
{
    // Default: switch to nearest to ball
    SwitchToNearestToBall();
}

void AMF_PlayerController::RequestPause()
{
    // Toggle pause menu via HUD
    if (CurrentHUD)
    {
        CurrentHUD->TogglePauseMenu();
        UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::RequestPause - Toggled pause menu"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerController::RequestPause - No HUD available"));
    }
}

// ==================== Possession Control ====================

void AMF_PlayerController::PossessFirstTeamCharacter()
{
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::PossessFirstTeamCharacter - TeamCharacters: %d, IsSpectator: %d"),
           TeamCharacters.Num(), bIsSpectator);

    if (bIsSpectator)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerController::PossessFirstTeamCharacter - In spectator mode, call SetSpectatorMode(false) first"));
        return;
    }

    if (TeamCharacters.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerController::PossessFirstTeamCharacter - No team characters available"));
        return;
    }

    // Find first valid character
    for (int32 i = 0; i < TeamCharacters.Num(); ++i)
    {
        if (TeamCharacters[i] && !TeamCharacters[i]->IsPendingKillPending())
        {
            SwitchToCharacter(i);
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("MF_PlayerController::PossessFirstTeamCharacter - No valid characters found"));
}

void AMF_PlayerController::PossessCharacter(AMF_PlayerCharacter *CharacterToPossess)
{
    if (!CharacterToPossess)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerController::PossessCharacter - Null character"));
        return;
    }

    if (bIsSpectator)
    {
        SetSpectatorMode(false);
    }

    // Find index in team characters
    int32 Index = TeamCharacters.IndexOfByKey(CharacterToPossess);
    if (Index != INDEX_NONE)
    {
        SwitchToCharacter(Index);
    }
    else
    {
        // Character not in our team, try to possess directly (server only)
        if (HasAuthority())
        {
            if (GetPawn())
            {
                UnPossess();
            }
            Possess(CharacterToPossess);
            UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::PossessCharacter - Directly possessed %s"),
                   *CharacterToPossess->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("MF_PlayerController::PossessCharacter - Character not in team array"));
        }
    }
}

void AMF_PlayerController::SetSpectatorMode(bool bEnabled)
{
    if (!HasAuthority())
    {
        // TODO: Add Server RPC if clients need to toggle spectator
        UE_LOG(LogTemp, Warning, TEXT("MF_PlayerController::SetSpectatorMode - Called on client"));
        return;
    }

    if (bIsSpectator == bEnabled)
    {
        return;
    }

    bIsSpectator = bEnabled;

    if (bEnabled)
    {
        // Unpossess current character
        if (GetPawn())
        {
            UnPossess();
        }
        ActiveCharacterIndex = -1;
        UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::SetSpectatorMode - Spectator mode ENABLED"));
    }
    else
    {
        // Auto-possess first team character when exiting spectator mode
        PossessFirstTeamCharacter();
        UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::SetSpectatorMode - Spectator mode DISABLED"));
    }
}

// ==================== Mobile Input Functions (UI Widget Support) ====================

void AMF_PlayerController::ApplyMobileMovementInput(FVector2D Direction)
{
    AMF_PlayerCharacter *CurrentCharacter = GetCurrentCharacter();
    if (CurrentCharacter)
    {
        // Apply movement input to the character
        // This assumes the character has a function to handle 2D movement input
        // Scale by sprint if needed
        float Scale = bMobileSprinting ? 1.5f : 1.0f;
        FVector2D ScaledDirection = Direction * Scale;

        // Add movement input (forward/right based on camera)
        CurrentCharacter->AddMovementInput(FVector(ScaledDirection.Y, ScaledDirection.X, 0.0f));
    }
}

void AMF_PlayerController::OnMobileActionPressed()
{
    AMF_PlayerCharacter *CurrentCharacter = GetCurrentCharacter();
    if (CurrentCharacter)
    {
        // Trigger action on character
        // This could be shoot, pass, tackle depending on context
        // For now, call the character's action function if it exists
        UE_LOG(LogTemp, Verbose, TEXT("MF_PlayerController::OnMobileActionPressed - Action triggered"));

        // TODO: Call character action method
        // CurrentCharacter->PerformContextAction();
    }
}

void AMF_PlayerController::OnMobileActionReleased()
{
    AMF_PlayerCharacter *CurrentCharacter = GetCurrentCharacter();
    if (CurrentCharacter)
    {
        UE_LOG(LogTemp, Verbose, TEXT("MF_PlayerController::OnMobileActionReleased - Action released"));

        // TODO: Call character action release method
        // CurrentCharacter->ReleaseContextAction();
    }
}

void AMF_PlayerController::SetMobileSprintState(bool bSprinting)
{
    bMobileSprinting = bSprinting;

    AMF_PlayerCharacter *CurrentCharacter = GetCurrentCharacter();
    if (CurrentCharacter)
    {
        // Set sprint state on the character if it has such a property
        UE_LOG(LogTemp, Verbose, TEXT("MF_PlayerController::SetMobileSprintState - Sprint: %s"),
               bSprinting ? TEXT("ON") : TEXT("OFF"));

        // TODO: Call character sprint method
        // CurrentCharacter->SetSprinting(bSprinting);
    }
}
