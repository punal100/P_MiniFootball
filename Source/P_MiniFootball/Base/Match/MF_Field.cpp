/*
 * @Author: Punal Manalan
 * @Description: MF_Field - Implementation
 * @Date: 04/01/2026
 */

#include "Match/MF_Field.h"
#include "Components/BoxComponent.h"
#include "Core/MF_Types.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BrushComponent.h"
#include "Model.h"
#if WITH_EDITOR
#include "Engine/BrushBuilder.h"
#include "Builders/CubeBuilder.h"
#include "Editor.h"
#endif
#include "NavigationSystem.h"

AMF_Field::AMF_Field()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create field bounds
    FieldBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("FieldBounds"));
    
    // Set default size from constants
    // BoxExtent is Half-Size
    FieldBounds->SetBoxExtent(FVector(MF_Constants::FieldWidth / 2.0f, MF_Constants::FieldLength / 2.0f, 500.0f));
    
    // Collision: Query only, ignore everything but support custom queries if needed
    FieldBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    FieldBounds->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    FieldBounds->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    FieldBounds->SetCanEverAffectNavigation(false);
    
    // Set to static so we can attach static NavMeshBoundsVolume to it
    FieldBounds->SetMobility(EComponentMobility::Static);
    
    RootComponent = FieldBounds;
}

void AMF_Field::BeginPlay()
{
    Super::BeginPlay();
}

void AMF_Field::UpdateNavMesh()
{
#if WITH_EDITOR
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find existing NavMeshBoundsVolume
    ANavMeshBoundsVolume* NavVolume = nullptr;
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ANavMeshBoundsVolume::StaticClass(), FoundActors);

    if (FoundActors.Num() > 0)
    {
        NavVolume = Cast<ANavMeshBoundsVolume>(FoundActors[0]);
        UE_LOG(LogTemp, Log, TEXT("MF_Field: Found existing NavMeshBoundsVolume"));
    }
    else
    {
        // Automatically spawn the volume
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        NavVolume = World->SpawnActor<ANavMeshBoundsVolume>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        
        if (NavVolume)
        {
            // Create a default cube brush (200x200x200) so we can scale it
            UCubeBuilder* CubeBuilder = NewObject<UCubeBuilder>(NavVolume);
            CubeBuilder->X = 200.0f;
            CubeBuilder->Y = 200.0f;
            CubeBuilder->Z = 200.0f;
            
            CubeBuilder->Build(World, NavVolume);
            
            // DO NOT attach to this field - verified that attaching breaks NavMesh generation
            // NavVolume->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
            
            UE_LOG(LogTemp, Log, TEXT("MF_Field: Created and Attached new NavMeshBoundsVolume"));
        }
    }

    if (NavVolume && FieldBounds)
    {
        // Ensure the NavVolume has a valid Brush (UModel)
        if (NavVolume->Brush == nullptr)
        {
            NavVolume->Brush = NewObject<UModel>(NavVolume, NAME_None, RF_Transactional);
            NavVolume->Brush->Initialize(NavVolume, 1);
            NavVolume->GetBrushComponent()->Brush = NavVolume->Brush;
            UE_LOG(LogTemp, Log, TEXT("MF_Field: Created new UModel for NavVolume"));
        }

        // Ensure it is NOT attached (Attaching breaks NavMesh generation)
        if (NavVolume->GetAttachParentActor() == this)
        {
            NavVolume->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
            UE_LOG(LogTemp, Warning, TEXT("MF_Field: Detached NavVolume from MF_Field to fix generation"));
        }

        FVector Origin = GetActorLocation();
        FVector BoxExtent = FieldBounds->GetScaledBoxExtent();
        
        // Add margin
        BoxExtent += FVector(NavMeshMargin, NavMeshMargin, NavMeshMargin);
        
        // Update volume location
        NavVolume->SetActorLocation(Origin);
        
        // Rebuild Brush with exact dimensions
        // Reset scale to 1,1,1
        NavVolume->SetActorScale3D(FVector::OneVector);
        NavVolume->SetActorRotation(GetActorRotation());
        
        UCubeBuilder* CubeBuilder = NewObject<UCubeBuilder>(NavVolume);
        
        FVector DesiredSize = BoxExtent * 2.0f;
        CubeBuilder->X = DesiredSize.X;
        CubeBuilder->Y = DesiredSize.Y;
        CubeBuilder->Z = DesiredSize.Z;
        
        CubeBuilder->Build(World, NavVolume);
        
        UE_LOG(LogTemp, Log, TEXT("MF_Field: Built NavMeshBoundsBrush with Size: %s"), *DesiredSize.ToString());

        if (GEditor)
        {
            GEditor->RebuildAlteredBSP();
        }
        
        // Force update brush component
        if (auto* BrushComp = NavVolume->GetBrushComponent())
        {
            if (NavVolume->Brush)
            {
                BrushComp->Brush = NavVolume->Brush;
                // UE_LOG(LogTemp, Log, TEXT("MF_Field: Assigned Brush UModel to Component"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("MF_Field: NavVolume->Brush is NULL!"));
            }

            BrushComp->UpdateBounds();
            UE_LOG(LogTemp, Log, TEXT("MF_Field: BrushComponent Bounds Extent after update: %s"), *BrushComp->Bounds.BoxExtent.ToString());
        }

        NavVolume->ReregisterAllComponents();
        
        UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
        if (NavSys)
        {
            NavSys->OnNavigationBoundsUpdated(NavVolume);
            NavSys->Build();
            UE_LOG(LogTemp, Log, TEXT("MF_Field: Triggered Navigation Build"));
        }
    }
#endif
}

void AMF_Field::ForceRebuildNavigation()
{
    UWorld* World = GetWorld();
    if (World)
    {
        if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
        {
            NavSys->Build();
            UE_LOG(LogTemp, Warning, TEXT("MF_Field: ForceRebuildNavigation triggered!"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("MF_Field: ForceRebuildNavigation FAILED - No NavigationSystem found."));
        }
    }
}
