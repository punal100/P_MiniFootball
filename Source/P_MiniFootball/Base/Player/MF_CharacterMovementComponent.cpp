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
        return bWantsToSprint ? MF_Constants::SprintSpeed : MF_Constants::WalkSpeed;
    }

    return BaseMaxSpeed;
}
