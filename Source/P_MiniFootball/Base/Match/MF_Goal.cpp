/*
 * @Author: Punal Manalan
 * @Description: MF_Goal - Implementation
 *               Goal trigger volume for detecting ball entry
 * @Date: 07/12/2025
 */

#include "Match/MF_Goal.h"
#include "Match/MF_GameState.h"
#include "Ball/MF_Ball.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AMF_Goal::AMF_Goal()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create goal trigger box
    GoalTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("GoalTrigger"));
    GoalTrigger->SetBoxExtent(FVector(50.0f, MF_Constants::GoalWidth / 2.0f, MF_Constants::GoalHeight / 2.0f));
    GoalTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GoalTrigger->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    GoalTrigger->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    GoalTrigger->SetGenerateOverlapEvents(true);
    RootComponent = GoalTrigger;

    // Network - only server needs to detect goals
    bReplicates = false;

    // Tag required for AI target resolution (UAIAction_MoveTo)
    Tags.Add(FName("Goal"));
}

void AMF_Goal::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap event
    if (GoalTrigger)
    {
        GoalTrigger->OnComponentBeginOverlap.AddDynamic(this, &AMF_Goal::OnGoalOverlap);
    }
}

void AMF_Goal::OnGoalOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor,
                             UPrimitiveComponent *OtherComp, int32 OtherBodyIndex,
                             bool bFromSweep, const FHitResult &SweepResult)
{
    // Only process on server
    if (!HasAuthority())
    {
        return;
    }

    // Prevent multiple triggers for same goal
    if (bGoalScoredThisFrame)
    {
        return;
    }

    // Check if it's the ball
    AMF_Ball *Ball = Cast<AMF_Ball>(OtherActor);
    if (!Ball)
    {
        return;
    }

    // Make sure we have a valid team
    if (DefendingTeam == EMF_TeamID::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("MF_Goal::OnGoalOverlap - GoalTeam not set!"));
        return;
    }

    // Score! The OPPOSITE of DefendingTeam scores when ball enters this goal
    EMF_TeamID ScoringTeam = (DefendingTeam == EMF_TeamID::TeamA) ? EMF_TeamID::TeamB : EMF_TeamID::TeamA;

    // Prevent multiple detections
    bGoalScoredThisFrame = true;

    // Reset flag after short delay
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMF_Goal::ResetGoalFlag, 2.0f, false);

    // Notify game state
    AMF_GameState *GS = Cast<AMF_GameState>(UGameplayStatics::GetGameState(this));
    if (GS)
    {
        GS->AddScore(ScoringTeam, 1);
    }

    // Broadcast event
    OnGoalTriggered.Broadcast(this, Ball);

    // Reset ball to center
    Ball->ResetToPosition(FVector(0.0f, 0.0f, MF_Constants::GroundZ + MF_Constants::BallRadius + MF_Constants::CharacterSpawnZOffset));
}

void AMF_Goal::ResetGoalFlag()
{
    bGoalScoredThisFrame = false;
}
