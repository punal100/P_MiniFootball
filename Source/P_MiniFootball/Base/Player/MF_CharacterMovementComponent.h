/*
 * @Author: Punal Manalan
 * @Description: MF_CharacterMovementComponent - Custom movement component
 *               Packs sprint intent into saved moves for correct network prediction.
 * @Date: 01/07/2026
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MF_CharacterMovementComponent.generated.h"

UCLASS()
class P_MINIFOOTBALL_API UMF_CharacterMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:
    UMF_CharacterMovementComponent();

    /** Sprint intent used by network prediction (packed into saved moves). */
    uint8 bWantsToSprint : 1;

    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual float GetMaxSpeed() const override;
    virtual void UpdateFromCompressedFlags(uint8 Flags) override;
    virtual FNetworkPredictionData_Client *GetPredictionData_Client() const override;

private:
    class FSavedMove_MF : public FSavedMove_Character
    {
    public:
        typedef FSavedMove_Character Super;

        uint8 bSavedWantsToSprint : 1;

        virtual void Clear() override;
        virtual uint8 GetCompressedFlags() const override;
        virtual bool CanCombineWith(const FSavedMovePtr &NewMove, ACharacter *Character, float MaxDelta) const override;
        virtual void SetMoveFor(ACharacter *Character, float InDeltaTime, FVector const &NewAccel, FNetworkPredictionData_Client_Character &ClientData) override;
        virtual void PrepMoveFor(ACharacter *Character) override;
    };

    class FNetworkPredictionData_Client_MF : public FNetworkPredictionData_Client_Character
    {
    public:
        typedef FNetworkPredictionData_Client_Character Super;

        explicit FNetworkPredictionData_Client_MF(const UCharacterMovementComponent &ClientMovement);
        virtual FSavedMovePtr AllocateNewMove() override;
    };
};
