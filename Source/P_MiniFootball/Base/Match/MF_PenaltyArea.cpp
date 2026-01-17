/*
 * @Author: Punal Manalan
 * @Description: MF_PenaltyArea - Implementation
 * @Date: 17/01/2026
 */

#include "Match/MF_PenaltyArea.h"

#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"

AMF_PenaltyArea::AMF_PenaltyArea()
{
#if !UE_BUILD_SHIPPING
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
#else
    PrimaryActorTick.bCanEverTick = false;
#endif

    // Create penalty area bounds
    PenaltyAreaBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("PenaltyAreaBounds"));

    // Default size from constants (BoxExtent is half-size)
    PenaltyAreaBounds->SetBoxExtent(FVector(MF_Constants::PenaltyAreaLength / 2.0f, MF_Constants::PenaltyAreaWidth / 2.0f, 200.0f));

    // Collision: Query only for overlap detection
    PenaltyAreaBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PenaltyAreaBounds->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    PenaltyAreaBounds->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    PenaltyAreaBounds->SetGenerateOverlapEvents(true);
    PenaltyAreaBounds->SetCanEverAffectNavigation(false);

    PenaltyAreaBounds->SetMobility(EComponentMobility::Static);
    RootComponent = PenaltyAreaBounds;

    bReplicates = false;

    Tags.Add(FName("PenaltyArea"));
}

void AMF_PenaltyArea::BeginPlay()
{
    Super::BeginPlay();

#if !UE_BUILD_SHIPPING
#if WITH_EDITORONLY_DATA
    if (bShowDebugInEditor)
    {
        SetActorTickEnabled(true);
    }
#endif
#endif
}

bool AMF_PenaltyArea::IsLocationInside(const FVector &Location) const
{
    if (!PenaltyAreaBounds)
    {
        return false;
    }

    const FVector LocalLocation = GetActorTransform().InverseTransformPosition(Location);
    const FVector Extent = PenaltyAreaBounds->GetScaledBoxExtent();

    return FMath::Abs(LocalLocation.X) <= Extent.X &&
           FMath::Abs(LocalLocation.Y) <= Extent.Y &&
           FMath::Abs(LocalLocation.Z) <= Extent.Z;
}

FVector AMF_PenaltyArea::GetPenaltyAreaCenter() const
{
    return GetActorLocation();
}

FVector AMF_PenaltyArea::GetPenaltyAreaExtent() const
{
    return PenaltyAreaBounds ? PenaltyAreaBounds->GetScaledBoxExtent() : FVector::ZeroVector;
}

#if !UE_BUILD_SHIPPING
void AMF_PenaltyArea::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

#if WITH_EDITORONLY_DATA
    if (!bShowDebugInEditor || !PenaltyAreaBounds)
    {
        return;
    }

    const FVector Center = PenaltyAreaBounds->GetComponentLocation();
    const FVector Extent = PenaltyAreaBounds->GetScaledBoxExtent();
    const FQuat Rot = PenaltyAreaBounds->GetComponentQuat();

    const FColor DebugColor = (DefendingTeam == EMF_TeamID::TeamA) ? FColor::Cyan :
                              (DefendingTeam == EMF_TeamID::TeamB) ? FColor::Orange : FColor::White;

    DrawDebugBox(GetWorld(), Center, Extent, Rot, DebugColor, false, 0.0f, 0, 3.0f);
#endif
}
#endif
