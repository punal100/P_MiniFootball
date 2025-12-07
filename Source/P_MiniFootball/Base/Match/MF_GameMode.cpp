/*
 * @Author: Punal Manalan
 * @Description: MF_GameMode - Implementation
 *               Server-only game mode for match management
 * @Date: 07/12/2025
 */

#include "Match/MF_GameMode.h"
#include "Match/MF_GameState.h"
#include "Player/MF_PlayerCharacter.h"
#include "Player/MF_PlayerController.h"
#include "Ball/MF_Ball.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AMF_GameMode::AMF_GameMode()
{
    // Set default classes
    GameStateClass = AMF_GameState::StaticClass();
    PlayerControllerClass = AMF_PlayerController::StaticClass();
    DefaultPawnClass = nullptr; // We handle pawn spawning ourselves

    // Set default character and ball classes for spawning
    PlayerCharacterClass = AMF_PlayerCharacter::StaticClass();
    BallClass = AMF_Ball::StaticClass();

    // Config defaults
    PlayersPerTeam = 5;
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

    // NOTE: Auto-assignment disabled - call AssignPlayerToTeam() manually from Blueprint
    // Available functions:
    // - AssignPlayerToTeam(PC, Team)
    // - RegisterTeamCharactersToController(PC)
    // - PossessCharacterWithController(PC, Character)
    // - PossessFirstAvailableTeamCharacter(PC)
    // - PossessTeamCharacterByIndex(PC, Index)

    UE_LOG(LogTemp, Log, TEXT("MF_GameMode::PostLogin - %s logged in (no auto-assignment)"),
           *NewPlayer->GetName());
}

void AMF_GameMode::Logout(AController *Exiting)
{
    AMF_PlayerController *MFPC = Cast<AMF_PlayerController>(Exiting);
    if (MFPC)
    {
        // Update team count
        if (MFPC->AssignedTeam == EMF_TeamID::TeamA)
        {
            TeamAPlayerCount = FMath::Max(0, TeamAPlayerCount - 1);
        }
        else if (MFPC->AssignedTeam == EMF_TeamID::TeamB)
        {
            TeamBPlayerCount = FMath::Max(0, TeamBPlayerCount - 1);
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
    if (!PC->TeamCharacters.Contains(Character))
    {
        PC->RegisterTeamCharacter(Character);
    }

    // Find index and switch
    int32 Index = PC->TeamCharacters.IndexOfByKey(Character);
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

    if (PC->TeamCharacters.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_GameMode::PossessFirstAvailableTeamCharacter - No team characters for %s"),
               *PC->GetName());
        return false;
    }

    // Find first valid character
    for (int32 i = 0; i < PC->TeamCharacters.Num(); ++i)
    {
        AMF_PlayerCharacter *Character = PC->TeamCharacters[i];
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

    AMF_PlayerCharacter *Character = FindCharacterByTeamAndIndex(PC->AssignedTeam, PlayerIndex);
    if (Character)
    {
        return PossessCharacterWithController(PC, Character);
    }

    UE_LOG(LogTemp, Warning, TEXT("MF_GameMode::PossessTeamCharacterByIndex - No character found for Team %d, Index %d"),
           static_cast<int32>(PC->AssignedTeam), PlayerIndex);
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
    if (!PC || PC->AssignedTeam == EMF_TeamID::None)
    {
        return;
    }

    TArray<AMF_PlayerCharacter *> TeamCharacters = GetSpawnedTeamCharacters(PC->AssignedTeam);

    UE_LOG(LogTemp, Log, TEXT("MF_GameMode::RegisterTeamCharactersToController - Registering %d characters to %s (Team %d)"),
           TeamCharacters.Num(), *PC->GetName(), static_cast<int32>(PC->AssignedTeam));

    for (AMF_PlayerCharacter *Character : TeamCharacters)
    {
        PC->RegisterTeamCharacter(Character);
    }
}
