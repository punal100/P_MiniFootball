/*
 * @Author: Punal Manalan
 * @Description: MF_GameMode - Implementation
 *               Server-only game mode for match management
 *               Implements IMF_TeamInterface for team join/leave handling
 * @Date: 07/12/2025
 * @Updated: 09/12/2025 - Added spectator system and team interface implementation
 */

#include "Match/MF_GameMode.h"
#include "Match/MF_GameState.h"
#include "Player/MF_PlayerCharacter.h"
#include "Player/MF_PlayerController.h"
#include "Player/MF_Spectator.h"
#include "Ball/MF_Ball.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AMF_GameMode::AMF_GameMode()
{
    // Set default classes
    GameStateClass = AMF_GameState::StaticClass();
    PlayerControllerClass = AMF_PlayerController::StaticClass();
    DefaultPawnClass = nullptr; // We spawn spectator manually

    // Set default character and ball classes for spawning
    PlayerCharacterClass = AMF_PlayerCharacter::StaticClass();
    BallClass = AMF_Ball::StaticClass();
    SpectatorPawnClass = AMF_Spectator::StaticClass();

    // Config defaults
    PlayersPerTeam = 3;
    MaxHumanPlayersPerTeam = 3;
    bAllowMidMatchJoin = true;
    TeamAPlayerCount = 0;
    TeamBPlayerCount = 0;
    SpawnedBall = nullptr;
}

void AMF_GameMode::InitGame(const FString &MapName, const FString &Options, FString &ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    UE_LOG(LogTemp, Log, TEXT("MF_GameMode::InitGame - Map: %s"), *MapName);

    // Setup spawn locations if not configured
    SetupDefaultSpawnLocations();
}

void AMF_GameMode::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("MF_GameMode::BeginPlay"));

    // Spawn teams and ball
    SpawnTeams();
    SpawnBall();

    // Optional hook for global UI creation.
    CreateGlobalUI();

    // NOTE: Auto-possession disabled - call these manually from Blueprint:
    // 1. AssignPlayerToTeam(PC, Team) - Assign controller to a team
    // 2. RegisterTeamCharactersToController(PC) - Give controller access to team characters
    // 3. PossessCharacterWithController(PC, Character) - Make controller possess specific character
    //    OR PossessFirstAvailableTeamCharacter(PC) - Possess first available
    //    OR PossessTeamCharacterByIndex(PC, Index) - Possess by index
}

void AMF_GameMode::PostLogin(APlayerController *NewPlayer)
{
    Super::PostLogin(NewPlayer);

    AMF_PlayerController *MFPC = Cast<AMF_PlayerController>(NewPlayer);
    if (!MFPC)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_GameMode::PostLogin - Not an MF_PlayerController"));
        return;
    }

    // Spawn spectator pawn for the new player
    AMF_Spectator *SpectatorPawn = SpawnSpectatorForController(MFPC);
    if (SpectatorPawn)
    {
        MFPC->Possess(SpectatorPawn);
        MFPC->SetSpectatorState(EMF_SpectatorState::Spectating);

        UE_LOG(LogTemp, Log, TEXT("MF_GameMode::PostLogin - %s spawned as spectator"),
               *NewPlayer->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_GameMode::PostLogin - Failed to spawn spectator for %s"),
               *NewPlayer->GetName());
    }

    // NOTE: Player must call Server_RequestJoinTeam() from their widget to join a team
    // Available functions for widgets to call:
    // - Server_RequestJoinTeam(TeamID) - Request to join a team
    // - Server_RequestLeaveTeam() - Request to leave current team

    // Optional hook for per-player UI creation.
    CreatePlayerUI(MFPC);
}

void AMF_GameMode::CreateGlobalUI_Implementation()
{
    // Intentionally empty by default. GameMode is server-only.
}

void AMF_GameMode::CreatePlayerUI_Implementation(AMF_PlayerController *PlayerController)
{
    // Intentionally empty by default. GameMode is server-only.
}

void AMF_GameMode::Logout(AController *Exiting)
{
    AMF_PlayerController *MFPC = Cast<AMF_PlayerController>(Exiting);
    if (MFPC)
    {
        const EMF_TeamID ExitingTeam = MFPC->GetAssignedTeam();
        // Handle player leaving team
        if (ExitingTeam != EMF_TeamID::None)
        {
            // Remove from team list
            if (ExitingTeam == EMF_TeamID::TeamA)
            {
                TeamAHumanPlayers.Remove(MFPC);
            }
            else if (ExitingTeam == EMF_TeamID::TeamB)
            {
                TeamBHumanPlayers.Remove(MFPC);
            }

            // Release character back to pool
            ReleaseCharacterFromPlayer(MFPC);

            UE_LOG(LogTemp, Log, TEXT("MF_GameMode::Logout - %s removed from team %d"),
                   *MFPC->GetName(), static_cast<int32>(ExitingTeam));
        }
    }

    Super::Logout(Exiting);
}

AActor *AMF_GameMode::ChoosePlayerStart_Implementation(AController *Player)
{
    // We don't use PlayerStart actors, return nullptr
    return nullptr;
}

UClass *AMF_GameMode::GetDefaultPawnClassForController_Implementation(AController *InController)
{
    // Don't auto-spawn pawns, we do it ourselves
    return nullptr;
}

// ==================== Match Control ====================

void AMF_GameMode::StartNewMatch()
{
    AMF_GameState *GS = GetMFGameState();
    if (GS)
    {
        GS->StartMatch();
    }

    UE_LOG(LogTemp, Log, TEXT("MF_GameMode::StartNewMatch"));
}

void AMF_GameMode::RestartMatch()
{
    // Reset all characters to spawn positions
    // TODO: Implement position reset

    StartNewMatch();
}

void AMF_GameMode::SpawnTeams()
{
    if (!PlayerCharacterClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_GameMode::SpawnTeams - No PlayerCharacterClass set"));
        return;
    }

    AMF_GameState *GS = GetMFGameState();

    // Spawn Team A
    for (int32 i = 0; i < PlayersPerTeam && i < TeamASpawnLocations.Num(); ++i)
    {
        AMF_PlayerCharacter *Character = SpawnTeamCharacter(EMF_TeamID::TeamA, i);
        if (Character && GS)
        {
            GS->RegisterPlayer(Character, EMF_TeamID::TeamA);
        }
    }

    // Spawn Team B
    for (int32 i = 0; i < PlayersPerTeam && i < TeamBSpawnLocations.Num(); ++i)
    {
        AMF_PlayerCharacter *Character = SpawnTeamCharacter(EMF_TeamID::TeamB, i);
        if (Character && GS)
        {
            GS->RegisterPlayer(Character, EMF_TeamID::TeamB);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("MF_GameMode::SpawnTeams - Spawned %d characters"), SpawnedCharacters.Num());
}

void AMF_GameMode::SpawnBall()
{
    if (!BallClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_GameMode::SpawnBall - No BallClass set"));
        return;
    }

    // Spawn at center of field
    FVector SpawnLocation = FVector(0.0f, 0.0f, MF_Constants::GroundZ + MF_Constants::BallRadius);
    FRotator SpawnRotation = FRotator::ZeroRotator;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    SpawnedBall = GetWorld()->SpawnActor<AMF_Ball>(BallClass, SpawnLocation, SpawnRotation, SpawnParams);

    // Register with game state
    AMF_GameState *GS = GetMFGameState();
    if (GS && SpawnedBall)
    {
        GS->RegisterBall(SpawnedBall);
    }

    UE_LOG(LogTemp, Log, TEXT("MF_GameMode::SpawnBall - Ball spawned at %s"), *SpawnLocation.ToString());
}

// ==================== Player Management ====================

AMF_GameState *AMF_GameMode::GetMFGameState() const
{
    return Cast<AMF_GameState>(GameState);
}

void AMF_GameMode::AssignPlayerToTeam(AMF_PlayerController *PC, EMF_TeamID Team)
{
    if (!PC)
    {
        return;
    }

    // Assign team to controller
    PC->AssignToTeam(Team);

    // Update counts
    if (Team == EMF_TeamID::TeamA)
    {
        TeamAPlayerCount++;
    }
    else if (Team == EMF_TeamID::TeamB)
    {
        TeamBPlayerCount++;
    }

    // NOTE: Character registration and possession is handled separately
    // by RegisterTeamCharactersToController() and PossessFirstAvailableTeamCharacter()
    // Called from PostLogin (if spawned) or BeginPlay (for early logins)

    UE_LOG(LogTemp, Log, TEXT("MF_GameMode::AssignPlayerToTeam - %s to Team %d"),
           *PC->GetName(), static_cast<int32>(Team));
}

EMF_TeamID AMF_GameMode::GetNextAvailableTeam() const
{
    // Simple balancing: assign to team with fewer players
    if (TeamAPlayerCount <= TeamBPlayerCount)
    {
        return EMF_TeamID::TeamA;
    }
    return EMF_TeamID::TeamB;
}

// ==================== Internal Functions ====================

AMF_PlayerCharacter *AMF_GameMode::SpawnTeamCharacter(EMF_TeamID Team, int32 SpawnIndex)
{
    TArray<FVector> &SpawnLocations = (Team == EMF_TeamID::TeamA) ? TeamASpawnLocations : TeamBSpawnLocations;

    if (SpawnIndex >= SpawnLocations.Num())
    {
        return nullptr;
    }

    FVector SpawnLocation = SpawnLocations[SpawnIndex];
    FRotator SpawnRotation = (Team == EMF_TeamID::TeamA) ? FRotator(0.0f, -90.0f, 0.0f) : FRotator(0.0f, 90.0f, 0.0f);

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AMF_PlayerCharacter *Character = GetWorld()->SpawnActor<AMF_PlayerCharacter>(
        PlayerCharacterClass, SpawnLocation, SpawnRotation, SpawnParams);

    if (Character)
    {
        Character->SetTeamID(Team);
        Character->SetPlayerID(SpawnIndex);
        SpawnedCharacters.Add(Character);
    }

    return Character;
}

void AMF_GameMode::SetupDefaultSpawnLocations()
{
    // Only setup defaults if not already configured
    if (TeamASpawnLocations.Num() == 0)
    {
        // Team A spawns on positive Y side (defending positive goal)
        float YOffset = MF_Constants::FieldLength * 0.25f;
        float XSpacing = MF_Constants::FieldWidth / (PlayersPerTeam + 1);

        for (int32 i = 0; i < PlayersPerTeam; ++i)
        {
            float X = -MF_Constants::FieldWidth / 2.0f + XSpacing * (i + 1);
            // Spawn above ground to prevent character being embedded in terrain
            TeamASpawnLocations.Add(FVector(X, YOffset, MF_Constants::GroundZ + MF_Constants::CharacterSpawnZOffset));
        }
    }

    if (TeamBSpawnLocations.Num() == 0)
    {
        // Team B spawns on negative Y side (defending negative goal)
        float YOffset = -MF_Constants::FieldLength * 0.25f;
        float XSpacing = MF_Constants::FieldWidth / (PlayersPerTeam + 1);

        for (int32 i = 0; i < PlayersPerTeam; ++i)
        {
            float X = -MF_Constants::FieldWidth / 2.0f + XSpacing * (i + 1);
            // Spawn above ground to prevent character being embedded in terrain
            TeamBSpawnLocations.Add(FVector(X, YOffset, MF_Constants::GroundZ + MF_Constants::CharacterSpawnZOffset));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("MF_GameMode::SetupDefaultSpawnLocations - TeamA: %d, TeamB: %d"),
           TeamASpawnLocations.Num(), TeamBSpawnLocations.Num());
}

// ==================== Possession Control ====================

bool AMF_GameMode::PossessCharacterWithController(AMF_PlayerController *PC, AMF_PlayerCharacter *Character)
{
    if (!PC || !Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_GameMode::PossessCharacterWithController - Null PC or Character"));
        return false;
    }

    // Ensure character is registered to this controller
    if (!PC->GetRegisteredTeamCharacters().Contains(Character))
    {
        PC->RegisterTeamCharacter(Character);
    }

    // Find index and switch
    int32 Index = PC->GetRegisteredTeamCharacterIndex(Character);
    if (Index != INDEX_NONE)
    {
        PC->SwitchToCharacter(Index);
        UE_LOG(LogTemp, Log, TEXT("MF_GameMode::PossessCharacterWithController - %s now possesses %s"),
               *PC->GetName(), *Character->GetName());
        return true;
    }

    return false;
}

bool AMF_GameMode::PossessFirstAvailableTeamCharacter(AMF_PlayerController *PC)
{
    if (!PC)
    {
        return false;
    }

    const TArray<AMF_PlayerCharacter *> &RegisteredCharacters = PC->GetRegisteredTeamCharacters();
    if (RegisteredCharacters.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_GameMode::PossessFirstAvailableTeamCharacter - No team characters for %s"),
               *PC->GetName());
        return false;
    }

    // Find first valid character
    for (int32 i = 0; i < RegisteredCharacters.Num(); ++i)
    {
        AMF_PlayerCharacter *Character = RegisteredCharacters[i];
        if (Character && !Character->IsPendingKillPending())
        {
            PC->SwitchToCharacter(i);
            UE_LOG(LogTemp, Log, TEXT("MF_GameMode::PossessFirstAvailableTeamCharacter - %s possessed %s (index %d)"),
                   *PC->GetName(), *Character->GetName(), i);
            return true;
        }
    }

    return false;
}

bool AMF_GameMode::PossessTeamCharacterByIndex(AMF_PlayerController *PC, int32 PlayerIndex)
{
    if (!PC)
    {
        return false;
    }

    AMF_PlayerCharacter *Character = FindCharacterByTeamAndIndex(PC->GetAssignedTeam(), PlayerIndex);
    if (Character)
    {
        return PossessCharacterWithController(PC, Character);
    }

    UE_LOG(LogTemp, Warning, TEXT("MF_GameMode::PossessTeamCharacterByIndex - No character found for Team %d, Index %d"),
           static_cast<int32>(PC->GetAssignedTeam()), PlayerIndex);
    return false;
}

TArray<AMF_PlayerCharacter *> AMF_GameMode::GetSpawnedTeamCharacters(EMF_TeamID Team) const
{
    TArray<AMF_PlayerCharacter *> Result;

    for (AMF_PlayerCharacter *Character : SpawnedCharacters)
    {
        if (Character && Character->GetTeamID() == Team)
        {
            Result.Add(Character);
        }
    }

    return Result;
}

TArray<AMF_PlayerController *> AMF_GameMode::GetAllMFPlayerControllers() const
{
    TArray<AMF_PlayerController *> Result;

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        AMF_PlayerController *MFPC = Cast<AMF_PlayerController>(It->Get());
        if (MFPC)
        {
            Result.Add(MFPC);
        }
    }

    return Result;
}

AMF_PlayerCharacter *AMF_GameMode::FindCharacterByTeamAndIndex(EMF_TeamID Team, int32 PlayerIndex) const
{
    for (AMF_PlayerCharacter *Character : SpawnedCharacters)
    {
        if (Character && Character->GetTeamID() == Team && Character->GetPlayerID() == PlayerIndex)
        {
            return Character;
        }
    }
    return nullptr;
}

void AMF_GameMode::RegisterTeamCharactersToController(AMF_PlayerController *PC)
{
    if (!PC || PC->GetAssignedTeam() == EMF_TeamID::None)
    {
        return;
    }

    const EMF_TeamID Team = PC->GetAssignedTeam();
    TArray<AMF_PlayerCharacter *> TeamCharacters = GetSpawnedTeamCharacters(Team);

    UE_LOG(LogTemp, Log, TEXT("MF_GameMode::RegisterTeamCharactersToController - Registering %d characters to %s (Team %d)"),
           TeamCharacters.Num(), *PC->GetName(), static_cast<int32>(Team));

    for (AMF_PlayerCharacter *Character : TeamCharacters)
    {
        PC->RegisterTeamCharacter(Character);
    }
}

// ==================== IMF_TeamInterface Implementation ====================

FMF_TeamAssignmentResult AMF_GameMode::HandleJoinTeamRequest_Implementation(APlayerController *RequestingPlayer, EMF_TeamID RequestedTeam)
{
    AMF_PlayerController *MFPC = Cast<AMF_PlayerController>(RequestingPlayer);
    if (!MFPC)
    {
        return FMF_TeamAssignmentResult::Failure(TEXT("Invalid player controller"));
    }

    // Check if player is already on a team
    if (MFPC->GetAssignedTeam() != EMF_TeamID::None)
    {
        return FMF_TeamAssignmentResult::Failure(TEXT("Already on a team. Leave current team first."));
    }

    // Check if mid-match join is allowed
    AMF_GameState *GS = GetMFGameState();
    if (GS && GS->CurrentPhase == EMF_MatchPhase::Playing && !bAllowMidMatchJoin)
    {
        return FMF_TeamAssignmentResult::Failure(TEXT("Mid-match joining is not allowed"));
    }

    // AUTO-ASSIGN: If RequestedTeam is None, server picks the best team
    if (RequestedTeam == EMF_TeamID::None)
    {
        TArray<EMF_TeamID> AvailableTeams = GetAvailableTeams_Implementation(RequestingPlayer);
        if (AvailableTeams.Num() == 0)
        {
            return FMF_TeamAssignmentResult::Failure(TEXT("Both teams are full"));
        }
        RequestedTeam = AvailableTeams[0];
        UE_LOG(LogTemp, Log, TEXT("MF_GameMode::HandleJoinTeamRequest - Auto-assigning to %s"),
               (RequestedTeam == EMF_TeamID::TeamA) ? TEXT("Team A") : TEXT("Team B"));
    }

    // Validate team request based on team balance
    if (!CanPlayerJoinTeam_Implementation(RequestingPlayer, RequestedTeam))
    {
        // Provide helpful message about which team they CAN join
        TArray<EMF_TeamID> AvailableTeams = GetAvailableTeams_Implementation(RequestingPlayer);
        if (AvailableTeams.Num() > 0)
        {
            FString AvailableStr = (AvailableTeams[0] == EMF_TeamID::TeamA) ? TEXT("Team A") : TEXT("Team B");
            return FMF_TeamAssignmentResult::Failure(FString::Printf(TEXT("Cannot join that team. Try %s instead."), *AvailableStr));
        }
        return FMF_TeamAssignmentResult::Failure(TEXT("Both teams are full"));
    }

    // Find an available character for this team
    AMF_PlayerCharacter *AvailableCharacter = GetAvailableCharacterForTeam(RequestedTeam);
    if (!AvailableCharacter)
    {
        return FMF_TeamAssignmentResult::Failure(TEXT("No available character slots on this team"));
    }

    // Assign player to team
    MFPC->AssignToTeam(RequestedTeam);

    // Add to team list
    if (RequestedTeam == EMF_TeamID::TeamA)
    {
        TeamAHumanPlayers.AddUnique(MFPC);
        TeamAPlayerCount = TeamAHumanPlayers.Num();
    }
    else if (RequestedTeam == EMF_TeamID::TeamB)
    {
        TeamBHumanPlayers.AddUnique(MFPC);
        TeamBPlayerCount = TeamBHumanPlayers.Num();
    }

    // Register character and possess
    MFPC->RegisterTeamCharacter(AvailableCharacter);
    MFPC->SwitchToCharacter(0);

    // Update spectator state
    MFPC->SetSpectatorState(EMF_SpectatorState::Playing);

    UE_LOG(LogTemp, Log, TEXT("MF_GameMode::HandleJoinTeamRequest - %s joined %s, possessing %s"),
           *MFPC->GetName(),
           (RequestedTeam == EMF_TeamID::TeamA) ? TEXT("Team A") : TEXT("Team B"),
           *AvailableCharacter->GetName());

    return FMF_TeamAssignmentResult::Success(RequestedTeam);
}

bool AMF_GameMode::HandleLeaveTeamRequest_Implementation(APlayerController *RequestingPlayer)
{
    AMF_PlayerController *MFPC = Cast<AMF_PlayerController>(RequestingPlayer);
    if (!MFPC)
    {
        return false;
    }

    // Check if player is on a team
    EMF_TeamID CurrentTeam = MFPC->GetAssignedTeam();
    if (CurrentTeam == EMF_TeamID::None)
    {
        return false;
    }

    // Release character back to pool
    ReleaseCharacterFromPlayer(MFPC);

    // Remove from team list
    if (CurrentTeam == EMF_TeamID::TeamA)
    {
        TeamAHumanPlayers.Remove(MFPC);
        TeamAPlayerCount = TeamAHumanPlayers.Num();
    }
    else if (CurrentTeam == EMF_TeamID::TeamB)
    {
        TeamBHumanPlayers.Remove(MFPC);
        TeamBPlayerCount = TeamBHumanPlayers.Num();
    }

    // Clear team assignment
    MFPC->AssignToTeam(EMF_TeamID::None);

    // Spawn spectator pawn and update state
    SpawnSpectatorForController(MFPC);
    MFPC->SetSpectatorState(EMF_SpectatorState::Spectating);

    UE_LOG(LogTemp, Log, TEXT("MF_GameMode::HandleLeaveTeamRequest - %s left team and returned to spectator"),
           *MFPC->GetName());

    return true;
}

bool AMF_GameMode::CanPlayerJoinTeam_Implementation(APlayerController *Player, EMF_TeamID Team)
{
    if (Team == EMF_TeamID::None)
    {
        return false;
    }

    int32 TeamACount = TeamAHumanPlayers.Num();
    int32 TeamBCount = TeamBHumanPlayers.Num();

    // Check if requested team is full
    if (Team == EMF_TeamID::TeamA && TeamACount >= MaxHumanPlayersPerTeam)
    {
        return false;
    }
    if (Team == EMF_TeamID::TeamB && TeamBCount >= MaxHumanPlayersPerTeam)
    {
        return false;
    }

    // Team balance logic: can only join if team is equal or smaller
    if (Team == EMF_TeamID::TeamA)
    {
        // Can join Team A if Team A has equal or fewer players than Team B
        return TeamACount <= TeamBCount;
    }
    else // Team B
    {
        // Can join Team B if Team B has equal or fewer players than Team A
        return TeamBCount <= TeamACount;
    }
}

bool AMF_GameMode::IsTeamFull_Implementation(EMF_TeamID Team)
{
    if (Team == EMF_TeamID::TeamA)
    {
        return TeamAHumanPlayers.Num() >= MaxHumanPlayersPerTeam;
    }
    else if (Team == EMF_TeamID::TeamB)
    {
        return TeamBHumanPlayers.Num() >= MaxHumanPlayersPerTeam;
    }
    return true;
}

int32 AMF_GameMode::GetTeamPlayerCount_Implementation(EMF_TeamID Team)
{
    if (Team == EMF_TeamID::TeamA)
    {
        return TeamAHumanPlayers.Num();
    }
    else if (Team == EMF_TeamID::TeamB)
    {
        return TeamBHumanPlayers.Num();
    }
    return 0;
}

TArray<EMF_TeamID> AMF_GameMode::GetAvailableTeams_Implementation(APlayerController *PC)
{
    TArray<EMF_TeamID> AvailableTeams;

    int32 TeamACount = TeamAHumanPlayers.Num();
    int32 TeamBCount = TeamBHumanPlayers.Num();

    // If both teams are full, return empty
    if (TeamACount >= MaxHumanPlayersPerTeam && TeamBCount >= MaxHumanPlayersPerTeam)
    {
        return AvailableTeams;
    }

    // Team balance logic
    if (TeamACount < TeamBCount && TeamACount < MaxHumanPlayersPerTeam)
    {
        // Team A has fewer players, must join Team A
        AvailableTeams.Add(EMF_TeamID::TeamA);
    }
    else if (TeamBCount < TeamACount && TeamBCount < MaxHumanPlayersPerTeam)
    {
        // Team B has fewer players, must join Team B
        AvailableTeams.Add(EMF_TeamID::TeamB);
    }
    else
    {
        // Teams are equal, can join either (if not full)
        if (TeamACount < MaxHumanPlayersPerTeam)
        {
            AvailableTeams.Add(EMF_TeamID::TeamA);
        }
        if (TeamBCount < MaxHumanPlayersPerTeam)
        {
            AvailableTeams.Add(EMF_TeamID::TeamB);
        }
    }

    return AvailableTeams;
}

int32 AMF_GameMode::GetMaxPlayersPerTeam_Implementation()
{
    return MaxHumanPlayersPerTeam;
}

bool AMF_GameMode::IsMidMatchJoinAllowed_Implementation()
{
    return bAllowMidMatchJoin;
}

// ==================== Spectator System Helpers ====================

AMF_PlayerCharacter *AMF_GameMode::GetAvailableCharacterForTeam(EMF_TeamID Team)
{
    // Find a character of the requested team that is not currently possessed by a human player
    for (AMF_PlayerCharacter *Character : SpawnedCharacters)
    {
        if (Character && Character->GetTeamID() == Team)
        {
            // Check if character is possessed by a player controller
            AController *CurrentController = Character->GetController();
            AMF_PlayerController *MFPC = Cast<AMF_PlayerController>(CurrentController);

            // Available if not possessed or only possessed by AI
            if (!CurrentController || !MFPC)
            {
                return Character;
            }
        }
    }

    return nullptr;
}

void AMF_GameMode::ReleaseCharacterFromPlayer(AMF_PlayerController *PC)
{
    if (!PC)
    {
        return;
    }

    // Get currently possessed character
    AMF_PlayerCharacter *CurrentCharacter = Cast<AMF_PlayerCharacter>(PC->GetPawn());

    if (CurrentCharacter)
    {
        // Unpossess the character
        PC->UnPossess();

        // Clear team characters list
        PC->ResetRegisteredTeamCharacters();

        UE_LOG(LogTemp, Log, TEXT("MF_GameMode::ReleaseCharacterFromPlayer - %s released %s"),
               *PC->GetName(), *CurrentCharacter->GetName());
    }
}

AMF_Spectator *AMF_GameMode::SpawnSpectatorForController(AMF_PlayerController *PC)
{
    if (!PC)
    {
        return nullptr;
    }

    // Determine spectator class to use
    TSubclassOf<AMF_Spectator> ClassToSpawn = SpectatorPawnClass;
    if (!ClassToSpawn)
    {
        ClassToSpawn = AMF_Spectator::StaticClass();
    }

    // Determine spawn location (center of field, elevated)
    FVector SpawnLocation = FVector(0.0f, 0.0f, MF_Constants::GroundZ + 500.0f);
    FRotator SpawnRotation = FRotator(-45.0f, 0.0f, 0.0f); // Looking down at field

    // Spawn spectator pawn
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AMF_Spectator *SpectatorPawn = GetWorld()->SpawnActor<AMF_Spectator>(
        ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);

    if (SpectatorPawn)
    {
        // Possess the spectator pawn
        PC->Possess(SpectatorPawn);

        UE_LOG(LogTemp, Log, TEXT("MF_GameMode::SpawnSpectatorForController - %s now spectating"),
               *PC->GetName());
        return SpectatorPawn;
    }
    return nullptr;
}
