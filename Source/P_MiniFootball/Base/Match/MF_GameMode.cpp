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

    // Auto-assign to team
    EMF_TeamID Team = GetNextAvailableTeam();
    AssignPlayerToTeam(MFPC, Team);

    UE_LOG(LogTemp, Log, TEXT("MF_GameMode::PostLogin - %s assigned to Team %d"),
           *NewPlayer->GetName(), static_cast<int32>(Team));
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

    // Assign team
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

    // Find spawned characters for this team and register them to the controller
    AMF_GameState *GS = GetMFGameState();
    if (GS)
    {
        TArray<AMF_PlayerCharacter *> TeamPlayers = GS->GetTeamPlayers(Team);
        for (AMF_PlayerCharacter *Character : TeamPlayers)
        {
            PC->RegisterTeamCharacter(Character);
        }
    }

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
            TeamASpawnLocations.Add(FVector(X, YOffset, MF_Constants::GroundZ));
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
            TeamBSpawnLocations.Add(FVector(X, YOffset, MF_Constants::GroundZ));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("MF_GameMode::SetupDefaultSpawnLocations - TeamA: %d, TeamB: %d"),
           TeamASpawnLocations.Num(), TeamBSpawnLocations.Num());
}
