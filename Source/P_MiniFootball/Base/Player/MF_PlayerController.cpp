/*
 * @Author: Punal Manalan
 * @Description: MF_PlayerController - Implementation
 *               Full network replication for Listen Server and Dedicated Server
 * @Date: 07/12/2025
 */

#include "Player/MF_PlayerController.h"
#include "Player/MF_PlayerCharacter.h"
#include "Ball/MF_Ball.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"

AMF_PlayerController::AMF_PlayerController()
{
    bReplicates = true;
    AssignedTeam = EMF_TeamID::None;
    ActiveCharacterIndex = -1;
    bIsSpectator = false;
}

void AMF_PlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMF_PlayerController, AssignedTeam);
    DOREPLIFETIME(AMF_PlayerController, TeamCharacters);
    DOREPLIFETIME(AMF_PlayerController, ActiveCharacterIndex);
    DOREPLIFETIME(AMF_PlayerController, bIsSpectator);
}

void AMF_PlayerController::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::BeginPlay - HasAuthority: %d, IsLocalController: %d"),
           HasAuthority(), IsLocalController());
}

void AMF_PlayerController::OnPossess(APawn *InPawn)
{
    Super::OnPossess(InPawn);

    UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::OnPossess - Pawn: %s"),
           InPawn ? *InPawn->GetName() : TEXT("null"));
}

void AMF_PlayerController::OnUnPossess()
{
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::OnUnPossess"));
    Super::OnUnPossess();
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

void AMF_PlayerController::SwitchToNextCharacter()
{
    if (TeamCharacters.Num() <= 1)
    {
        return;
    }

    int32 NextIndex = (ActiveCharacterIndex + 1) % TeamCharacters.Num();
    SwitchToCharacter(NextIndex);
}

void AMF_PlayerController::SwitchToNearestToBall()
{
    int32 NearestIndex = FindNearestCharacterToBall();
    if (NearestIndex >= 0 && NearestIndex != ActiveCharacterIndex)
    {
        SwitchToCharacter(NearestIndex);
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
        return ActiveCharacterIndex; // No ball, stay with current
    }

    FVector BallLocation = CachedBallActor->GetActorLocation();
    float NearestDistSq = MAX_flt;
    int32 NearestIndex = -1;

    for (int32 i = 0; i < TeamCharacters.Num(); ++i)
    {
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
    // TODO: Implement pause menu logic
    UE_LOG(LogTemp, Log, TEXT("MF_PlayerController::RequestPause - Pause requested"));
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
