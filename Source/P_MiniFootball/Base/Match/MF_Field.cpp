/*
 * @Author: Punal Manalan
 * @Description: MF_Field - Implementation
 * @Date: 04/01/2026
 */

#include "Match/MF_Field.h"
#include "Components/BoxComponent.h"
#include "Core/MF_Types.h"
#include "Match/MF_Goal.h"
#include "Match/MF_PenaltyArea.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BrushComponent.h"
#include "Model.h"
#include "DrawDebugHelpers.h"
#if WITH_EDITOR
#include "Engine/BrushBuilder.h"
#include "Builders/CubeBuilder.h"
#include "Editor.h"
#endif
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"

AMF_Field::AMF_Field()
{
#if !UE_BUILD_SHIPPING
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
#else
    PrimaryActorTick.bCanEverTick = false;
#endif

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

    if (HasAuthority())
    {
        EnsureNavMesh();
    }
}

void AMF_Field::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (bAutoSpawnGoals)
    {
        SpawnOrUpdateGoals();
    }
    else
    {
        if (IsValid(GoalA) && !GoalA->IsActorBeingDestroyed())
        {
            GoalA->Destroy();
        }
        GoalA = nullptr;

        if (IsValid(GoalB) && !GoalB->IsActorBeingDestroyed())
        {
            GoalB->Destroy();
        }
        GoalB = nullptr;
    }

    if (bAutoSpawnPenaltyAreas)
    {
        SpawnOrUpdatePenaltyAreas();
    }
    else
    {
        if (IsValid(PenaltyAreaA) && !PenaltyAreaA->IsActorBeingDestroyed())
        {
            PenaltyAreaA->Destroy();
        }
        PenaltyAreaA = nullptr;

        if (IsValid(PenaltyAreaB) && !PenaltyAreaB->IsActorBeingDestroyed())
        {
            PenaltyAreaB->Destroy();
        }
        PenaltyAreaB = nullptr;
    }

#if !UE_BUILD_SHIPPING
#if WITH_EDITORONLY_DATA
    if (bShowFieldDebug || bShowGoalDebug || bShowPenaltyAreaDebug)
    {
        SetActorTickEnabled(true);
    }
    else
    {
        SetActorTickEnabled(false);
    }
#endif
#endif

#if WITH_EDITOR
    if (GIsEditor && !GIsPlayInEditorWorld)
    {
        UpdateNavMesh();
    }
#endif
}

void AMF_Field::EnsureNavMesh()
{
    UWorld* World = GetWorld();
    if (!World) return;

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!NavSys)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MF_Field] NavigationSystem not found! NavMesh will not work."));
    }

#if WITH_EDITOR
    // Only update bounds in Editor mode, NEVER in PIE to avoid Mobility warnings
    if (!GIsPlayInEditorWorld)
    {
        UpdateNavMesh();
    }
#else
    UE_LOG(LogTemp, Log, TEXT("[MF_Field] EnsureNavMesh - Runtime generation relies on NavigationInvokers or pre-placed Bounds."));
#endif
}

#if !UE_BUILD_SHIPPING
void AMF_Field::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

#if WITH_EDITORONLY_DATA
    DrawFieldDebug();
#endif
}
#endif

void AMF_Field::SpawnOrUpdateGoals()
{
    UWorld *World = GetWorld();
    if (!World || !FieldBounds)
    {
        return;
    }

    const FVector FieldCenter = GetActorLocation();
    const FVector FieldExtent = FieldBounds->GetScaledBoxExtent();
    const FVector Right = GetActorRightVector();

    const float GoalDepthExtent = GoalDepth / 2.0f;
    const FVector GoalExtent(GoalDepthExtent, GoalWidth / 2.0f, GoalHeight / 2.0f);

    const FVector GoalALocation = FieldCenter - Right * (FieldExtent.Y + GoalDepthExtent);
    const FRotator GoalARotation = Right.Rotation();

    const FVector GoalBLocation = FieldCenter + Right * (FieldExtent.Y + GoalDepthExtent);
    const FRotator GoalBRotation = (-Right).Rotation();

    // Goal A
    if (!IsValid(GoalA) || GoalA->IsActorBeingDestroyed())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.ObjectFlags |= RF_Transactional;

        GoalA = World->SpawnActor<AMF_Goal>(GoalALocation, GoalARotation, SpawnParams);

        if (GoalA)
        {
#if WITH_EDITOR
            GoalA->SetActorLabel(TEXT("MF_Goal_TeamA"));
#endif
            UE_LOG(LogTemp, Log, TEXT("MF_Field: Spawned Goal A (TeamA defends) at %s"), *GoalALocation.ToString());
        }
    }

    if (GoalA)
    {
        GoalA->SetActorLocationAndRotation(GoalALocation, GoalARotation);
        GoalA->DefendingTeam = EMF_TeamID::TeamA;
        if (GoalA->GoalTrigger)
        {
            GoalA->GoalTrigger->SetBoxExtent(GoalExtent);
        }

        GoalA->Tags.AddUnique(FName("TeamA"));
        GoalA->Tags.Remove(FName("TeamB"));

#if WITH_EDITORONLY_DATA
        GoalA->bShowDebugInEditor = bShowGoalDebug;
#endif
    }

    // Goal B
    if (!IsValid(GoalB) || GoalB->IsActorBeingDestroyed())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.ObjectFlags |= RF_Transactional;

        GoalB = World->SpawnActor<AMF_Goal>(GoalBLocation, GoalBRotation, SpawnParams);

        if (GoalB)
        {
#if WITH_EDITOR
            GoalB->SetActorLabel(TEXT("MF_Goal_TeamB"));
#endif
            UE_LOG(LogTemp, Log, TEXT("MF_Field: Spawned Goal B (TeamB defends) at %s"), *GoalBLocation.ToString());
        }
    }

    if (GoalB)
    {
        GoalB->SetActorLocationAndRotation(GoalBLocation, GoalBRotation);
        GoalB->DefendingTeam = EMF_TeamID::TeamB;
        if (GoalB->GoalTrigger)
        {
            GoalB->GoalTrigger->SetBoxExtent(GoalExtent);
        }

        GoalB->Tags.AddUnique(FName("TeamB"));
        GoalB->Tags.Remove(FName("TeamA"));

#if WITH_EDITORONLY_DATA
        GoalB->bShowDebugInEditor = bShowGoalDebug;
#endif
    }
}

void AMF_Field::SpawnOrUpdatePenaltyAreas()
{
    UWorld *World = GetWorld();
    if (!World || !FieldBounds)
    {
        return;
    }

    const FVector FieldCenter = GetActorLocation();
    const FVector FieldExtent = FieldBounds->GetScaledBoxExtent();
    const FVector Right = GetActorRightVector();

    const float PenaltyAreaHeight = 200.0f;
    const FVector PenaltyExtent(PenaltyAreaLength / 2.0f, PenaltyAreaWidth / 2.0f, PenaltyAreaHeight);

    const FVector PenaltyALocation = FieldCenter - Right * (FieldExtent.Y - PenaltyAreaLength / 2.0f);
    const FRotator PenaltyARotation = Right.Rotation();

    const FVector PenaltyBLocation = FieldCenter + Right * (FieldExtent.Y - PenaltyAreaLength / 2.0f);
    const FRotator PenaltyBRotation = (-Right).Rotation();

    // Penalty Area A
    if (!IsValid(PenaltyAreaA) || PenaltyAreaA->IsActorBeingDestroyed())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.ObjectFlags |= RF_Transactional;

        PenaltyAreaA = World->SpawnActor<AMF_PenaltyArea>(PenaltyALocation, PenaltyARotation, SpawnParams);

        if (PenaltyAreaA)
        {
#if WITH_EDITOR
            PenaltyAreaA->SetActorLabel(TEXT("MF_PenaltyArea_TeamA"));
#endif
            UE_LOG(LogTemp, Log, TEXT("MF_Field: Spawned Penalty Area A (TeamA defends) at %s"), *PenaltyALocation.ToString());
        }
    }

    if (PenaltyAreaA)
    {
        PenaltyAreaA->SetActorLocationAndRotation(PenaltyALocation, PenaltyARotation);
        PenaltyAreaA->DefendingTeam = EMF_TeamID::TeamA;
        if (PenaltyAreaA->PenaltyAreaBounds)
        {
            PenaltyAreaA->PenaltyAreaBounds->SetBoxExtent(PenaltyExtent);
        }

        PenaltyAreaA->Tags.AddUnique(FName("TeamA"));
        PenaltyAreaA->Tags.Remove(FName("TeamB"));

#if WITH_EDITORONLY_DATA
        PenaltyAreaA->bShowDebugInEditor = bShowPenaltyAreaDebug;
#endif
    }

    // Penalty Area B
    if (!IsValid(PenaltyAreaB) || PenaltyAreaB->IsActorBeingDestroyed())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.ObjectFlags |= RF_Transactional;

        PenaltyAreaB = World->SpawnActor<AMF_PenaltyArea>(PenaltyBLocation, PenaltyBRotation, SpawnParams);

        if (PenaltyAreaB)
        {
#if WITH_EDITOR
            PenaltyAreaB->SetActorLabel(TEXT("MF_PenaltyArea_TeamB"));
#endif
            UE_LOG(LogTemp, Log, TEXT("MF_Field: Spawned Penalty Area B (TeamB defends) at %s"), *PenaltyBLocation.ToString());
        }
    }

    if (PenaltyAreaB)
    {
        PenaltyAreaB->SetActorLocationAndRotation(PenaltyBLocation, PenaltyBRotation);
        PenaltyAreaB->DefendingTeam = EMF_TeamID::TeamB;
        if (PenaltyAreaB->PenaltyAreaBounds)
        {
            PenaltyAreaB->PenaltyAreaBounds->SetBoxExtent(PenaltyExtent);
        }

        PenaltyAreaB->Tags.AddUnique(FName("TeamB"));
        PenaltyAreaB->Tags.Remove(FName("TeamA"));

#if WITH_EDITORONLY_DATA
        PenaltyAreaB->bShowDebugInEditor = bShowPenaltyAreaDebug;
#endif
    }
}

void AMF_Field::DestroySpawnedComponents()
{
    if (IsValid(GoalA) && !GoalA->IsActorBeingDestroyed())
    {
        GoalA->Destroy();
    }
    GoalA = nullptr;

    if (IsValid(GoalB) && !GoalB->IsActorBeingDestroyed())
    {
        GoalB->Destroy();
    }
    GoalB = nullptr;

    if (IsValid(PenaltyAreaA) && !PenaltyAreaA->IsActorBeingDestroyed())
    {
        PenaltyAreaA->Destroy();
    }
    PenaltyAreaA = nullptr;

    if (IsValid(PenaltyAreaB) && !PenaltyAreaB->IsActorBeingDestroyed())
    {
        PenaltyAreaB->Destroy();
    }
    PenaltyAreaB = nullptr;
}

void AMF_Field::DrawFieldDebug()
{
#if WITH_EDITORONLY_DATA
    UWorld *World = GetWorld();
    if (!World || !FieldBounds)
    {
        return;
    }

    const FVector FieldCenter = FieldBounds->GetComponentLocation();
    const FVector FieldExtent = FieldBounds->GetScaledBoxExtent();
    const FQuat FieldRot = FieldBounds->GetComponentQuat();

    if (bShowFieldDebug)
    {
        DrawDebugBox(World, FieldCenter, FieldExtent, FieldRot, FColor::Green, false, 0.0f, 0, 3.0f);

        // Center circle on the field plane
        DrawDebugCircle(World, FieldCenter, MF_Constants::CenterCircleRadius, 48, FColor::Green, false, 0.0f, 0, 2.0f, GetActorForwardVector(), GetActorRightVector(), false);
    }

    if (bShowGoalDebug)
    {
        if (GoalA && GoalA->GoalTrigger)
        {
            DrawDebugBox(World, GoalA->GoalTrigger->GetComponentLocation(), GoalA->GoalTrigger->GetScaledBoxExtent(), GoalA->GoalTrigger->GetComponentQuat(), FColor::Blue, false, 0.0f, 0, 2.0f);
        }
        if (GoalB && GoalB->GoalTrigger)
        {
            DrawDebugBox(World, GoalB->GoalTrigger->GetComponentLocation(), GoalB->GoalTrigger->GetScaledBoxExtent(), GoalB->GoalTrigger->GetComponentQuat(), FColor::Red, false, 0.0f, 0, 2.0f);
        }
    }

    if (bShowPenaltyAreaDebug)
    {
        if (PenaltyAreaA && PenaltyAreaA->PenaltyAreaBounds)
        {
            DrawDebugBox(World, PenaltyAreaA->PenaltyAreaBounds->GetComponentLocation(), PenaltyAreaA->PenaltyAreaBounds->GetScaledBoxExtent(), PenaltyAreaA->PenaltyAreaBounds->GetComponentQuat(), FColor::Cyan, false, 0.0f, 0, 2.0f);
        }
        if (PenaltyAreaB && PenaltyAreaB->PenaltyAreaBounds)
        {
            DrawDebugBox(World, PenaltyAreaB->PenaltyAreaBounds->GetComponentLocation(), PenaltyAreaB->PenaltyAreaBounds->GetScaledBoxExtent(), PenaltyAreaB->PenaltyAreaBounds->GetComponentQuat(), FColor::Orange, false, 0.0f, 0, 2.0f);
        }
    }
#endif
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
