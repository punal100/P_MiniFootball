/*
 * @Author: Punal Manalan
 * @Description: MF_CharacterMovementComponent - Implementation
 *               Packs sprint intent into saved moves for correct network prediction.
 * @Date: 01/07/2026
 */

#include "Player/MF_CharacterMovementComponent.h"

#include "Core/MF_Types.h"
#include "Player/MF_PlayerCharacter.h"

UMF_CharacterMovementComponent::UMF_CharacterMovementComponent()
{
    bWantsToSprint = false;
}

void UMF_CharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    // For AI characters on the server, we need to manually drive movement
    // because base CharacterMovementComponent only processes input for locally controlled pawns
    AMF_PlayerCharacter* Player = Cast<AMF_PlayerCharacter>(CharacterOwner);
    const bool bIsServerAI = Player && Player->HasAuthority() && !Player->IsLocallyControlled() && Player->IsAIRunning();
    
    if (bIsServerAI)
    {
        // Get pending input from AddMovementInput calls (set by AI actions like MF.MoveTo)
        const FVector InputVector = PawnOwner ? PawnOwner->ConsumeMovementInputVector() : FVector::ZeroVector;
        
        if (!InputVector.IsNearlyZero())
        {
            // Calculate velocity from input direction and max speed
            const float CurrentMaxSpeed = GetMaxSpeed();
            Velocity = InputVector.GetClampedToMaxSize(1.0f) * CurrentMaxSpeed;
            Velocity.Z = 0.0f; // Keep grounded
            
            // Apply movement
            const FVector Delta = Velocity * DeltaTime;
            if (!Delta.IsNearlyZero())
            {
                // Move with collision
                FHitResult Hit;
                SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);
                
                if (Hit.IsValidBlockingHit())
                {
                    // Slide along walls
                    SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit, true);
                }
            }
        }
        else
        {
            // No input - stop movement
            Velocity = FVector::ZeroVector;
        }
    }
    
    // Always call base implementation for non-AI or client-side handling
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UMF_CharacterMovementComponent::FSavedMove_MF::Clear()
{
    Super::Clear();
    bSavedWantsToSprint = false;
}

uint8 UMF_CharacterMovementComponent::FSavedMove_MF::GetCompressedFlags() const
{
    uint8 Result = Super::GetCompressedFlags();
    if (bSavedWantsToSprint)
    {
        Result |= FSavedMove_Character::FLAG_Custom_0;
    }
    return Result;
}

bool UMF_CharacterMovementComponent::FSavedMove_MF::CanCombineWith(const FSavedMovePtr &NewMove, ACharacter *Character, float MaxDelta) const
{
    const FSavedMove_MF *NewMoveMF = static_cast<const FSavedMove_MF *>(NewMove.Get());
    if (bSavedWantsToSprint != NewMoveMF->bSavedWantsToSprint)
    {
        return false;
    }

    return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void UMF_CharacterMovementComponent::FSavedMove_MF::SetMoveFor(ACharacter *Character, float InDeltaTime, FVector const &NewAccel, FNetworkPredictionData_Client_Character &ClientData)
{
    Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

    if (const UMF_CharacterMovementComponent *MoveComp = Cast<UMF_CharacterMovementComponent>(Character->GetCharacterMovement()))
    {
        bSavedWantsToSprint = MoveComp->bWantsToSprint;
    }
}

void UMF_CharacterMovementComponent::FSavedMove_MF::PrepMoveFor(ACharacter *Character)
{
    Super::PrepMoveFor(Character);

    if (UMF_CharacterMovementComponent *MoveComp = Cast<UMF_CharacterMovementComponent>(Character->GetCharacterMovement()))
    {
        MoveComp->bWantsToSprint = bSavedWantsToSprint;
    }
}

UMF_CharacterMovementComponent::FNetworkPredictionData_Client_MF::FNetworkPredictionData_Client_MF(const UCharacterMovementComponent &ClientMovement)
    : Super(ClientMovement)
{
}

FSavedMovePtr UMF_CharacterMovementComponent::FNetworkPredictionData_Client_MF::AllocateNewMove()
{
    return FSavedMovePtr(new FSavedMove_MF());
}

FNetworkPredictionData_Client *UMF_CharacterMovementComponent::GetPredictionData_Client() const
{
    if (ClientPredictionData == nullptr)
    {
        UMF_CharacterMovementComponent *MutableThis = const_cast<UMF_CharacterMovementComponent *>(this);
        MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_MF(*this);
    }

    return ClientPredictionData;
}

void UMF_CharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
    Super::UpdateFromCompressedFlags(Flags);

    bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;

    // Keep replicated sprint state in sync on the server.
    if (CharacterOwner && CharacterOwner->HasAuthority())
    {
        if (AMF_PlayerCharacter *Player = Cast<AMF_PlayerCharacter>(CharacterOwner))
        {
            Player->SetSprinting(bWantsToSprint);
        }
    }
}

float UMF_CharacterMovementComponent::GetMaxSpeed() const
{
    const float BaseMaxSpeed = Super::GetMaxSpeed();

    // Only override walking speeds; keep other movement modes unchanged.
    if (MovementMode == MOVE_Walking || MovementMode == MOVE_NavWalking)
    {
        float Speed = bWantsToSprint ? MF_Constants::SprintSpeed : MF_Constants::WalkSpeed;

        // Apply ball carrier speed reduction when player has the ball
        if (const AMF_PlayerCharacter* Player = Cast<AMF_PlayerCharacter>(CharacterOwner))
        {
            if (Player->HasBall())
            {
                // Apply both percentage and absolute reduction
                // First apply percentage reduction, then subtract absolute value
                Speed = Speed * (1.0f - MF_Constants::BallCarrierSpeedReductionPercent);
                Speed = FMath::Max(Speed - MF_Constants::BallCarrierSpeedReductionAbsolute, 100.0f); // Minimum 100 cm/s
            }
        }

        return Speed;
    }

    return BaseMaxSpeed;
}
