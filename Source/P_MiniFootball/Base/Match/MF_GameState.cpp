/*
 * @Author: Punal Manalan
 * @Description: MF_GameState - Implementation
 *               Full network replication for Listen Server and Dedicated Server
 * @Date: 07/12/2025
 */

#include "Match/MF_GameState.h"
#include "Ball/MF_Ball.h"
#include "Player/MF_PlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "GameFramework/PlayerState.h"

AMF_GameState::AMF_GameState()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default match settings
    HalfDuration = MF_Constants::MatchDuration / 2.0f;
    ScoreToWin = 0; // Time-based by default

    // Initialize state
    CurrentPhase = EMF_MatchPhase::WaitingForPlayers;
    ScoreTeamA = 0;
    ScoreTeamB = 0;
    MatchTimeRemaining = HalfDuration;
    CurrentHalf = 1;
    KickoffTeam = EMF_TeamID::TeamA;
    MatchBall = nullptr;
    bMatchTimerActive = false;
}

void AMF_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMF_GameState, CurrentPhase);
    DOREPLIFETIME(AMF_GameState, ScoreTeamA);
    DOREPLIFETIME(AMF_GameState, ScoreTeamB);
    DOREPLIFETIME(AMF_GameState, MatchTimeRemaining);
    DOREPLIFETIME(AMF_GameState, CurrentHalf);
    DOREPLIFETIME(AMF_GameState, KickoffTeam);
    DOREPLIFETIME(AMF_GameState, TeamAPlayers);
    DOREPLIFETIME(AMF_GameState, TeamBPlayers);
    DOREPLIFETIME(AMF_GameState, MatchBall);
}

void AMF_GameState::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("MF_GameState::BeginPlay - HasAuthority: %d"), HasAuthority());
}

void AMF_GameState::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (HasAuthority())
    {
        UpdateMatchTimer(DeltaTime);
    }
}

// ==================== Match Control ====================

void AMF_GameState::StartMatch()
{
    if (!HasAuthority())
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("MF_GameState::StartMatch"));

    // Reset scores
    ScoreTeamA = 0;
    ScoreTeamB = 0;
    CurrentHalf = 1;
    MatchTimeRemaining = HalfDuration;

    // Start with kickoff
    ResetForKickoff(EMF_TeamID::TeamA);
}

void AMF_GameState::PauseMatch()
{
    if (!HasAuthority())
    {
        return;
    }

    bMatchTimerActive = false;
    UE_LOG(LogTemp, Log, TEXT("MF_GameState::PauseMatch"));
}

void AMF_GameState::ResumeMatch()
{
    if (!HasAuthority())
    {
        return;
    }

    if (CurrentPhase == EMF_MatchPhase::Playing)
    {
        bMatchTimerActive = true;
    }
    UE_LOG(LogTemp, Log, TEXT("MF_GameState::ResumeMatch"));
}

void AMF_GameState::EndMatch()
{
    if (!HasAuthority())
    {
        return;
    }

    HandleMatchEnd();
}

void AMF_GameState::AddScore(EMF_TeamID Team, int32 Points)
{
    if (!HasAuthority())
    {
        return;
    }

    if (Team == EMF_TeamID::TeamA)
    {
        ScoreTeamA += Points;
        OnRep_ScoreTeamA();
    }
    else if (Team == EMF_TeamID::TeamB)
    {
        ScoreTeamB += Points;
        OnRep_ScoreTeamB();
    }

    UE_LOG(LogTemp, Log, TEXT("MF_GameState::AddScore - Team: %d, Score: A=%d B=%d"),
           static_cast<int32>(Team), ScoreTeamA, ScoreTeamB);

    // Check win condition
    CheckWinCondition();

    // If match continues, reset for kickoff to the scored-upon team
    if (CurrentPhase != EMF_MatchPhase::MatchEnd)
    {
        EMF_TeamID KickoffTo = (Team == EMF_TeamID::TeamA) ? EMF_TeamID::TeamB : EMF_TeamID::TeamA;

        // Short delay before kickoff
        SetMatchPhase(EMF_MatchPhase::GoalScored);

        FTimerDelegate TimerDel;
        TimerDel.BindLambda([this, KickoffTo]()
                            { ResetForKickoff(KickoffTo); });
        GetWorld()->GetTimerManager().SetTimer(PhaseTimerHandle, TimerDel, 3.0f, false);
    }
}

void AMF_GameState::SetMatchPhase(EMF_MatchPhase NewPhase)
{
    if (!HasAuthority())
    {
        return;
    }

    if (CurrentPhase != NewPhase)
    {
        CurrentPhase = NewPhase;

        // Handle timer based on phase
        switch (NewPhase)
        {
        case EMF_MatchPhase::Playing:
            bMatchTimerActive = true;
            NotifyAIMatchPlaying();
            break;
        case EMF_MatchPhase::WaitingForPlayers:
        case EMF_MatchPhase::Kickoff:
        case EMF_MatchPhase::GoalScored:
        case EMF_MatchPhase::HalfTime:
        case EMF_MatchPhase::MatchEnd:
            bMatchTimerActive = false;
            break;
        }

        OnRep_MatchPhase();
        UE_LOG(LogTemp, Log, TEXT("MF_GameState::SetMatchPhase - Phase: %d"), static_cast<int32>(NewPhase));
    }
}

void AMF_GameState::NotifyAIMatchPlaying()
{
    if (!HasAuthority())
    {
        return;
    }

    auto NotifyRoster = [](const TArray<AMF_PlayerCharacter*>& Roster)
    {
        for (AMF_PlayerCharacter* Player : Roster)
        {
            if (Player && Player->IsAIRunning())
            {
                Player->InjectAIEvent(TEXT("MatchStarted"));
                Player->ResetAI();
            }
        }
    };

    NotifyRoster(TeamAPlayers);
    NotifyRoster(TeamBPlayers);

    UE_LOG(LogTemp, Log, TEXT("MF_GameState::NotifyAIMatchPlaying - Notified running AI on both teams"));
}

void AMF_GameState::ResetForKickoff(EMF_TeamID Team)
{
    if (!HasAuthority())
    {
        return;
    }

    KickoffTeam = Team;
    SetMatchPhase(EMF_MatchPhase::Kickoff);

    // Reset ball to center
    if (MatchBall)
    {
        MatchBall->ResetToPosition(FVector(0.0f, 0.0f, MF_Constants::GroundZ + MF_Constants::BallRadius));
    }

    // TODO: Reset player positions

    // Start playing after short delay
    FTimerDelegate TimerDel;
    TimerDel.BindLambda([this]()
                        { SetMatchPhase(EMF_MatchPhase::Playing); });
    GetWorld()->GetTimerManager().SetTimer(PhaseTimerHandle, TimerDel, 2.0f, false);

    UE_LOG(LogTemp, Log, TEXT("MF_GameState::ResetForKickoff - Team: %d"), static_cast<int32>(Team));
}

void AMF_GameState::RegisterBall(AMF_Ball *Ball)
{
    if (!HasAuthority())
    {
        return;
    }

    MatchBall = Ball;

    // Bind to ball events
    if (MatchBall)
    {
        MatchBall->OnGoalScored.AddDynamic(this, &AMF_GameState::HandleGoalScored);
    }

    UE_LOG(LogTemp, Log, TEXT("MF_GameState::RegisterBall - %s"),
           Ball ? *Ball->GetName() : TEXT("null"));
}

void AMF_GameState::HandleGoalScored(AMF_Ball *Ball, EMF_TeamID ScoringTeam)
{
    // Delegate to AddScore which handles the actual scoring logic
    AddScore(ScoringTeam, 1);
}

// ==================== Team Management ====================

void AMF_GameState::RegisterPlayer(AMF_PlayerCharacter *Player, EMF_TeamID Team)
{
    if (!HasAuthority() || !Player)
    {
        return;
    }

    // Remove from any existing team first
    UnregisterPlayer(Player);

    // Add to team
    if (Team == EMF_TeamID::TeamA)
    {
        TeamAPlayers.Add(Player);
    }
    else if (Team == EMF_TeamID::TeamB)
    {
        TeamBPlayers.Add(Player);
    }

    // Set player's team
    Player->SetTeamID(Team);

    UE_LOG(LogTemp, Log, TEXT("MF_GameState::RegisterPlayer - %s to Team %d"),
           *Player->GetName(), static_cast<int32>(Team));
}

void AMF_GameState::UnregisterPlayer(AMF_PlayerCharacter *Player)
{
    if (!HasAuthority() || !Player)
    {
        return;
    }

    TeamAPlayers.Remove(Player);
    TeamBPlayers.Remove(Player);
}

TArray<AMF_PlayerCharacter *> AMF_GameState::GetTeamPlayers(EMF_TeamID Team) const
{
    if (Team == EMF_TeamID::TeamA)
    {
        return TeamAPlayers;
    }
    else if (Team == EMF_TeamID::TeamB)
    {
        return TeamBPlayers;
    }
    return TArray<AMF_PlayerCharacter *>();
}

int32 AMF_GameState::GetTeamPlayerCount(EMF_TeamID Team) const
{
    if (Team == EMF_TeamID::TeamA)
    {
        return TeamAPlayers.Num();
    }
    else if (Team == EMF_TeamID::TeamB)
    {
        return TeamBPlayers.Num();
    }
    return 0;
}

TArray<FString> AMF_GameState::GetTeamPlayerNames(EMF_TeamID Team) const
{
    TArray<FString> Names;
    TArray<AMF_PlayerCharacter *> Players = GetTeamPlayers(Team);

    for (AMF_PlayerCharacter *Player : Players)
    {
        if (Player)
        {
            // Use explicit APawn:: scope since AMF_PlayerCharacter has its own GetPlayerState()
            // that returns EMF_PlayerState (game state), not APlayerState*
            // APawn::GetPlayerState<T>() IS replicated to all clients
            APawn *PawnPtr = Player;
            APlayerState *PS = PawnPtr->GetPlayerState<APlayerState>();
            if (PS)
            {
                Names.Add(PS->GetPlayerName());
            }
            else
            {
                // Fallback to actor name if no PlayerState (shouldn't happen for possessed pawns)
                Names.Add(Player->GetName());
            }
        }
    }

    return Names;
}

FMF_TeamRosterData AMF_GameState::GetTeamRoster(EMF_TeamID Team) const
{
    FMF_TeamRosterData RosterData;
    RosterData.TeamID = Team;
    RosterData.PlayerNames = GetTeamPlayerNames(Team);
    RosterData.CurrentPlayerCount = GetTeamPlayerCount(Team);
    RosterData.MaxPlayerCount = 3; // Default max players per team

    return RosterData;
}

// ==================== State Getters ====================

int32 AMF_GameState::GetScore(EMF_TeamID Team) const
{
    if (Team == EMF_TeamID::TeamA)
    {
        return ScoreTeamA;
    }
    else if (Team == EMF_TeamID::TeamB)
    {
        return ScoreTeamB;
    }
    return 0;
}

bool AMF_GameState::IsMatchInProgress() const
{
    return CurrentPhase == EMF_MatchPhase::Playing ||
           CurrentPhase == EMF_MatchPhase::Kickoff ||
           CurrentPhase == EMF_MatchPhase::GoalScored;
}

EMF_TeamID AMF_GameState::GetWinningTeam() const
{
    if (ScoreTeamA > ScoreTeamB)
    {
        return EMF_TeamID::TeamA;
    }
    else if (ScoreTeamB > ScoreTeamA)
    {
        return EMF_TeamID::TeamB;
    }
    return EMF_TeamID::None; // Tie
}

FString AMF_GameState::GetFormattedTime() const
{
    int32 Minutes = FMath::FloorToInt(MatchTimeRemaining / 60.0f);
    int32 Seconds = FMath::FloorToInt(FMath::Fmod(MatchTimeRemaining, 60.0f));
    return FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
}

AMF_Ball *AMF_GameState::GetMatchBall() const
{
    return MatchBall;
}

EMF_TeamID AMF_GameState::GetTeamForController(APlayerController *PC) const
{
    if (!PC)
    {
        return EMF_TeamID::None;
    }

    // Resolve team from possessed pawn
    AMF_PlayerCharacter *Character = Cast<AMF_PlayerCharacter>(PC->GetPawn());
    if (Character)
    {
        return Character->GetTeamID();
    }

    // Fallback: Check if any team character is registered to this controller
    for (AMF_PlayerCharacter *Player : TeamAPlayers)
    {
        if (Player && Player->GetController() == PC)
        {
            return EMF_TeamID::TeamA;
        }
    }

    for (AMF_PlayerCharacter *Player : TeamBPlayers)
    {
        if (Player && Player->GetController() == PC)
        {
            return EMF_TeamID::TeamB;
        }
    }

    return EMF_TeamID::None;
}

bool AMF_GameState::TeamHasBall(EMF_TeamID Team) const
{
    const TArray<AMF_PlayerCharacter*>& Roster = (Team == EMF_TeamID::TeamA) ? TeamAPlayers : TeamBPlayers;
    for (AMF_PlayerCharacter* Player : Roster)
    {
        if (Player && Player->HasBall())
        {
            return true;
        }
    }
    return false;
}

// ==================== Rep Notifies ====================

void AMF_GameState::OnRep_MatchPhase()
{
    OnMatchPhaseChanged.Broadcast(CurrentPhase);
}

void AMF_GameState::OnRep_ScoreTeamA()
{
    OnScoreChanged.Broadcast(EMF_TeamID::TeamA, ScoreTeamA);
}

void AMF_GameState::OnRep_ScoreTeamB()
{
    OnScoreChanged.Broadcast(EMF_TeamID::TeamB, ScoreTeamB);
}

void AMF_GameState::OnRep_MatchTimeRemaining()
{
    OnMatchTimeUpdated.Broadcast(MatchTimeRemaining);
}

void AMF_GameState::OnRep_TeamAPlayers()
{
    UE_LOG(LogTemp, Log, TEXT("MF_GameState::OnRep_TeamAPlayers - Count: %d"), TeamAPlayers.Num());
    OnTeamRosterChanged.Broadcast(EMF_TeamID::TeamA);
}

void AMF_GameState::OnRep_TeamBPlayers()
{
    UE_LOG(LogTemp, Log, TEXT("MF_GameState::OnRep_TeamBPlayers - Count: %d"), TeamBPlayers.Num());
    OnTeamRosterChanged.Broadcast(EMF_TeamID::TeamB);
}

// ==================== Internal Functions ====================

void AMF_GameState::UpdateMatchTimer(float DeltaTime)
{
    if (!bMatchTimerActive)
    {
        return;
    }

    MatchTimeRemaining -= DeltaTime;

    if (MatchTimeRemaining <= 0.0f)
    {
        MatchTimeRemaining = 0.0f;

        if (CurrentHalf == 1)
        {
            HandleHalfTime();
        }
        else
        {
            HandleMatchEnd();
        }
    }
}

void AMF_GameState::CheckWinCondition()
{
    if (ScoreToWin > 0)
    {
        if (ScoreTeamA >= ScoreToWin || ScoreTeamB >= ScoreToWin)
        {
            HandleMatchEnd();
        }
    }
}

void AMF_GameState::HandleHalfTime()
{
    SetMatchPhase(EMF_MatchPhase::HalfTime);

    // Switch sides, swap kickoff
    EMF_TeamID NextKickoff = (KickoffTeam == EMF_TeamID::TeamA) ? EMF_TeamID::TeamB : EMF_TeamID::TeamA;
    CurrentHalf = 2;
    MatchTimeRemaining = HalfDuration;

    // Resume after halftime break
    FTimerDelegate TimerDel;
    TimerDel.BindLambda([this, NextKickoff]()
                        { ResetForKickoff(NextKickoff); });
    GetWorld()->GetTimerManager().SetTimer(PhaseTimerHandle, TimerDel, 5.0f, false);

    UE_LOG(LogTemp, Log, TEXT("MF_GameState::HandleHalfTime - Starting second half"));
}

void AMF_GameState::HandleMatchEnd()
{
    SetMatchPhase(EMF_MatchPhase::MatchEnd);

    EMF_TeamID Winner = GetWinningTeam();
    OnMatchEnded.Broadcast(Winner);

    UE_LOG(LogTemp, Log, TEXT("MF_GameState::HandleMatchEnd - Winner: %d (Score: %d - %d)"),
           static_cast<int32>(Winner), ScoreTeamA, ScoreTeamB);
}
