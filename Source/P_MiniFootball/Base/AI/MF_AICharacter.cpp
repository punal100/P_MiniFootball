/*
 * @Author: Punal Manalan
 * @Description: Implementation of MF_AICharacter
 * @Date: 29/12/2025
 */

#include "MF_AICharacter.h"
#include "../Ball/MF_Ball.h"
#include "AIComponent.h"
#include "AIBehaviour.h"
#include "EAISSubsystem.h"
#include "Engine/World.h"
#include "Engine/World.h"

AMF_AICharacter::AMF_AICharacter()
{
    // Create the AI component
    AIComponent = CreateDefaultSubobject<UAIComponent>(TEXT("AIComponent"));
    
    // Configure default AI settings
    AIComponent->bAutoStart = false; // We control start timing
    AIComponent->TickInterval = AITickInterval;
}

void AMF_AICharacter::BeginPlay()
{
    Super::BeginPlay();

    // Only run AI on server (or standalone)
    if (!HasAuthority())
    {
        return;
    }

    // Configure AI component
    AIComponent->TickInterval = AITickInterval;
    AIComponent->bDebugMode = bDebugAI;

    // Load behavior from profile or asset
    if (AIBehaviour)
    {
        AIComponent->InitializeAI(AIBehaviour);
    }
    else if (!AIProfile.IsEmpty())
    {
        FString ProfilePath = AIProfile;
        if (!ProfilePath.EndsWith(TEXT(".json")))
        {
            ProfilePath += TEXT(".json");
        }
        AIComponent->JsonFilePath = ProfilePath;
    }

    // Auto-start if enabled
    if (bAutoStartAI)
    {
        StartAI();
    }
}

void AMF_AICharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Sync game state to blackboard
    if (HasAuthority() && AIComponent && AIComponent->IsValid())
    {
        SyncBlackboard();
    }
}

void AMF_AICharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // Update AI blackboard with controller info
    if (AIComponent)
    {
        AIComponent->SetBlackboardObject(TEXT("Controller"), NewController);
    }
}

void AMF_AICharacter::StartAI()
{
    if (AIComponent)
    {
        AIComponent->StartAI();
        
        if (bDebugAI)
        {
            UE_LOG(LogTemp, Log, TEXT("MF_AICharacter: Started AI with profile '%s'"), *AIProfile);
        }
    }
}

void AMF_AICharacter::StopAI()
{
    if (AIComponent)
    {
        AIComponent->StopAI();
    }
}

void AMF_AICharacter::ResetAI()
{
    if (AIComponent)
    {
        AIComponent->ResetAI();
    }
}

bool AMF_AICharacter::SetAIProfile(const FString& ProfileName)
{
    if (!AIComponent)
    {
        return false;
    }

    // Stop current AI
    AIComponent->StopAI();

    // Load new profile
    FString ProfilePath = ProfileName;
    if (!ProfilePath.EndsWith(TEXT(".json")))
    {
        ProfilePath += TEXT(".json");
    }

    AIComponent->JsonFilePath = ProfilePath;
    AIComponent->ResetAI();

    AIProfile = ProfileName;
    
    // Restart if was running
    if (bAutoStartAI)
    {
        AIComponent->StartAI();
    }

    return true;
}

void AMF_AICharacter::InjectAIEvent(const FString& EventName)
{
    if (AIComponent)
    {
        AIComponent->EnqueueSimpleEvent(EventName);
    }
}

bool AMF_AICharacter::IsAIRunning() const
{
    return AIComponent && AIComponent->IsRunning();
}

FString AMF_AICharacter::GetCurrentAIState() const
{
    if (AIComponent)
    {
        return AIComponent->GetCurrentState();
    }
    return TEXT("");
}

void AMF_AICharacter::SyncBlackboard()
{
    if (!AIComponent)
    {
        return;
    }

    // Sync ball possession
    AIComponent->SetBlackboardBool(TEXT("HasBall"), HasBall());
    
    // Sync ball reference
    if (AMF_Ball* Ball = GetPossessedBall())
    {
        AIComponent->SetBlackboardObject(TEXT("Ball"), Ball);
        AIComponent->SetBlackboardVector(TEXT("BallPosition"), Ball->GetActorLocation());
    }

    // Sync team
    AIComponent->SetBlackboardFloat(TEXT("TeamID"), static_cast<float>(GetTeamID()));

    // Sync player state
    AIComponent->SetBlackboardBool(TEXT("IsStunned"), IsStunned());
    AIComponent->SetBlackboardBool(TEXT("IsSprinting"), IsSprinting());

    // Sync position
    AIComponent->SetBlackboardVector(TEXT("MyPosition"), GetActorLocation());
}

void AMF_AICharacter::OnBallPossessionChanged()
{
    if (AIComponent)
    {
        // Inject event for AI state transition
        if (HasBall())
        {
            AIComponent->EnqueueSimpleEvent(TEXT("GotBall"));
        }
        else
        {
            AIComponent->EnqueueSimpleEvent(TEXT("LostBall"));
        }
    }
}
