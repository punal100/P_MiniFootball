/*
 * @Author: Punal Manalan
 * @Description: MF_Formation - Player roles and team formation structures for 11v11
 * @Date: 10/01/2026
 */

#pragma once

#include "CoreMinimal.h"
#include "MF_Types.h"
#include "MF_Formation.generated.h"

/**
 * Player roles for formation positioning
 */
UENUM(BlueprintType)
enum class EMF_PlayerRole : uint8
{
    Goalkeeper UMETA(DisplayName = "Goalkeeper"),
    Defender UMETA(DisplayName = "Defender"),
    Midfielder UMETA(DisplayName = "Midfielder"),
    Striker UMETA(DisplayName = "Striker"),
    None UMETA(DisplayName = "None")
};

/**
 * Formation slot data - defines a single position in the formation
 */
USTRUCT(BlueprintType)
struct FMF_FormationSlot
{
    GENERATED_BODY()
    
    /** Role of this slot */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    EMF_PlayerRole Role = EMF_PlayerRole::None;
    
    /** Slot name (e.g., "LB", "CM1", "ST2") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FString SlotName;
    
    /** Relative position X (-0.5 to 0.5, where 0 is center, -0.5 is own goal, 0.5 is opponent goal) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float RelativeX = 0.0f;
    
    /** Relative position Y (-0.5 to 0.5, where 0 is center, -0.5 is left, 0.5 is right) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float RelativeY = 0.0f;
    
    /** AI Profile to use for this role */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FString AIProfile = TEXT("Striker");
    
    /** Default constructor */
    FMF_FormationSlot() = default;
    
    /** Constructor with parameters */
    FMF_FormationSlot(EMF_PlayerRole InRole, const FString& InSlotName, float InX, float InY, const FString& InProfile)
        : Role(InRole), SlotName(InSlotName), RelativeX(InX), RelativeY(InY), AIProfile(InProfile) {}
};

/**
 * Team formation configuration - defines positions for all 11 players
 */
USTRUCT(BlueprintType)
struct FMF_Formation
{
    GENERATED_BODY()
    
    /** Formation name (e.g., "4-4-2", "4-3-3") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FString FormationName = TEXT("4-4-2");
    
    /** Formation slots - one for each player position */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<FMF_FormationSlot> Slots;
    
    /** Get world position for slot index based on team */
    FVector GetSlotWorldPosition(int32 SlotIndex, EMF_TeamID Team) const
    {
        if (!Slots.IsValidIndex(SlotIndex))
        {
            return FVector::ZeroVector;
        }
        
        const FMF_FormationSlot& Slot = Slots[SlotIndex];
        
        // Mirror X for Team B (opponent side)
        float X = Slot.RelativeX * MF_Constants::FieldLength;
        if (Team == EMF_TeamID::TeamB)
        {
            X = -X; // Mirror for opposite team
        }
        
        float Y = Slot.RelativeY * MF_Constants::FieldWidth;
        
        return FVector(X, Y, MF_Constants::GroundZ + MF_Constants::CharacterSpawnZOffset);
    }
    
    /** Get AI profile for slot index */
    FString GetSlotAIProfile(int32 SlotIndex) const
    {
        if (Slots.IsValidIndex(SlotIndex))
        {
            return Slots[SlotIndex].AIProfile;
        }
        return TEXT("Striker"); // Default fallback
    }
    
    /** Get role for slot index */
    EMF_PlayerRole GetSlotRole(int32 SlotIndex) const
    {
        if (Slots.IsValidIndex(SlotIndex))
        {
            return Slots[SlotIndex].Role;
        }
        return EMF_PlayerRole::None;
    }
    
    /** Create default 4-4-2 formation for 11v11 */
    static FMF_Formation Create442()
    {
        FMF_Formation Formation;
        Formation.FormationName = TEXT("4-4-2");
        
        // Goalkeeper (1)
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Goalkeeper, TEXT("GK"), -0.45f, 0.0f, TEXT("Goalkeeper")));
        
        // Defenders (4)
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Defender, TEXT("LB"), -0.35f, -0.35f, TEXT("Defender")));
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Defender, TEXT("CB1"), -0.35f, -0.12f, TEXT("Defender")));
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Defender, TEXT("CB2"), -0.35f, 0.12f, TEXT("Defender")));
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Defender, TEXT("RB"), -0.35f, 0.35f, TEXT("Defender")));
        
        // Midfielders (4)
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Midfielder, TEXT("LM"), -0.10f, -0.35f, TEXT("Midfielder")));
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Midfielder, TEXT("CM1"), -0.10f, -0.12f, TEXT("Midfielder")));
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Midfielder, TEXT("CM2"), -0.10f, 0.12f, TEXT("Midfielder")));
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Midfielder, TEXT("RM"), -0.10f, 0.35f, TEXT("Midfielder")));
        
        // Strikers (2)
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Striker, TEXT("ST1"), 0.30f, -0.15f, TEXT("Striker")));
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Striker, TEXT("ST2"), 0.30f, 0.15f, TEXT("Striker")));
        
        return Formation;
    }
    
    /** Create 4-3-3 formation for 11v11 */
    static FMF_Formation Create433()
    {
        FMF_Formation Formation;
        Formation.FormationName = TEXT("4-3-3");
        
        // Goalkeeper (1)
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Goalkeeper, TEXT("GK"), -0.45f, 0.0f, TEXT("Goalkeeper")));
        
        // Defenders (4)
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Defender, TEXT("LB"), -0.35f, -0.35f, TEXT("Defender")));
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Defender, TEXT("CB1"), -0.35f, -0.12f, TEXT("Defender")));
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Defender, TEXT("CB2"), -0.35f, 0.12f, TEXT("Defender")));
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Defender, TEXT("RB"), -0.35f, 0.35f, TEXT("Defender")));
        
        // Midfielders (3)
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Midfielder, TEXT("CDM"), -0.15f, 0.0f, TEXT("Midfielder")));
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Midfielder, TEXT("CM1"), -0.05f, -0.20f, TEXT("Midfielder")));
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Midfielder, TEXT("CM2"), -0.05f, 0.20f, TEXT("Midfielder")));
        
        // Strikers/Wingers (3)
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Striker, TEXT("LW"), 0.25f, -0.35f, TEXT("Striker")));
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Striker, TEXT("ST"), 0.30f, 0.0f, TEXT("Striker")));
        Formation.Slots.Add(FMF_FormationSlot(EMF_PlayerRole::Striker, TEXT("RW"), 0.25f, 0.35f, TEXT("Striker")));
        
        return Formation;
    }
};
