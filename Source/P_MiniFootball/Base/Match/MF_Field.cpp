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
#include "TimerManager.h"
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

#if WITH_EDITOR
    // Skip spawning for preview actors during drag-drop from Content Browser
    // bIsEditorPreviewActor is true for the temporary preview actor shown while dragging
    if (bIsEditorPreviewActor)
    {
        return;
    }
    
    // CRITICAL: Skip spawn/destroy operations during undo/redo transactions to prevent crashes
    // GIsTransacting is true when an undo/redo operation is in progress
    const bool bIsInTransaction = GIsTransacting;
    
    // Check if this is just a transform change (move/rotate) vs a property change
    const bool bTransformOnly = CachedTransform.Equals(FTransform::Identity) ? false : 
        (CachedTransform.GetLocation() != Transform.GetLocation() || 
         CachedTransform.GetRotation() != Transform.GetRotation() ||
         CachedTransform.GetScale3D() != Transform.GetScale3D()) && !NeedsRespawn();
    
    CachedTransform = Transform;
    
    // During undo/redo, only update positions of existing valid actors, never spawn/destroy
    if (bIsInTransaction)
    {
        // Just update positions if actors exist and are valid
        if (bAutoSpawnGoals && IsValid(GoalA) && IsValid(GoalB))
        {
            const FVector FieldCenter = GetActorLocation();
            const FVector FieldExtent = FieldBounds->GetScaledBoxExtent();
            const FVector Right = GetActorRightVector();
            const FVector Forward = GetActorForwardVector();
            const float GoalDepthExtent = GoalDepth / 2.0f;
            
            // TeamA goal on RIGHT side (+Right), TeamB goal on LEFT side (-Right)
            const FVector GoalALocation = FieldCenter + Right * (FieldExtent.Y - GoalDepthExtent);
            const FRotator GoalARotation = Forward.Rotation();  // 0 degrees
            const FVector GoalBLocation = FieldCenter - Right * (FieldExtent.Y - GoalDepthExtent);
            const FRotator GoalBRotation = (-Forward).Rotation();  // 180 degrees
            
            GoalA->SetActorLocationAndRotation(GoalALocation, GoalARotation);
            GoalB->SetActorLocationAndRotation(GoalBLocation, GoalBRotation);
        }
        
        if (bAutoSpawnPenaltyAreas && IsValid(PenaltyAreaA) && IsValid(PenaltyAreaB))
        {
            const FVector FieldCenter = GetActorLocation();
            const FVector FieldExtent = FieldBounds->GetScaledBoxExtent();
            const FVector Right = GetActorRightVector();
            const FVector Forward = GetActorForwardVector();
            const float PenaltyDepthExtent = PenaltyAreaLength / 2.0f;
            
            // TeamA penalty on RIGHT side (+Right), TeamB penalty on LEFT side (-Right)
            const FVector PenaltyALocation = FieldCenter + Right * (FieldExtent.Y - PenaltyDepthExtent);
            const FRotator PenaltyARotation = Forward.Rotation();  // 0 degrees
            const FVector PenaltyBLocation = FieldCenter - Right * (FieldExtent.Y - PenaltyDepthExtent);
            const FRotator PenaltyBRotation = (-Forward).Rotation();  // 180 degrees
            
            PenaltyAreaA->SetActorLocationAndRotation(PenaltyALocation, PenaltyARotation);
            PenaltyAreaB->SetActorLocationAndRotation(PenaltyBLocation, PenaltyBRotation);
        }
        
        // Skip nav mesh update during transaction as well
        return;
    }
#else
    const bool bTransformOnly = false;
#endif

    if (bAutoSpawnGoals)
    {
        // If we already have valid goals and this is just a transform change, only update positions
        if (bTransformOnly && IsValid(GoalA) && IsValid(GoalB))
        {
            // Just update goal positions without respawning
            const FVector FieldCenter = GetActorLocation();
            const FVector FieldExtent = FieldBounds->GetScaledBoxExtent();
            const FVector Right = GetActorRightVector();
            const FVector Forward = GetActorForwardVector();
            const float GoalDepthExtent = GoalDepth / 2.0f;
            
            // TeamA goal on RIGHT side (+Right), TeamB goal on LEFT side (-Right)
            const FVector GoalALocation = FieldCenter + Right * (FieldExtent.Y - GoalDepthExtent);
            const FRotator GoalARotation = Forward.Rotation();  // 0 degrees
            const FVector GoalBLocation = FieldCenter - Right * (FieldExtent.Y - GoalDepthExtent);
            const FRotator GoalBRotation = (-Forward).Rotation();  // 180 degrees
            
            GoalA->SetActorLocationAndRotation(GoalALocation, GoalARotation);
            GoalB->SetActorLocationAndRotation(GoalBLocation, GoalBRotation);
        }
        else
        {
            SpawnOrUpdateGoals();
        }
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
        // If we already have valid penalty areas and this is just a transform change, only update positions
        if (bTransformOnly && IsValid(PenaltyAreaA) && IsValid(PenaltyAreaB))
        {
            // Just update penalty area positions without respawning
            const FVector FieldCenter = GetActorLocation();
            const FVector FieldExtent = FieldBounds->GetScaledBoxExtent();
            const FVector Right = GetActorRightVector();
            const FVector Forward = GetActorForwardVector();
            const float PenaltyDepthExtent = PenaltyAreaLength / 2.0f;
            
            // TeamA penalty on RIGHT side (+Right), TeamB penalty on LEFT side (-Right)
            const FVector PenaltyALocation = FieldCenter + Right * (FieldExtent.Y - PenaltyDepthExtent);
            const FRotator PenaltyARotation = Forward.Rotation();  // 0 degrees
            const FVector PenaltyBLocation = FieldCenter - Right * (FieldExtent.Y - PenaltyDepthExtent);
            const FRotator PenaltyBRotation = (-Forward).Rotation();  // 180 degrees
            
            PenaltyAreaA->SetActorLocationAndRotation(PenaltyALocation, PenaltyARotation);
            PenaltyAreaB->SetActorLocationAndRotation(PenaltyBLocation, PenaltyBRotation);
        }
        else
        {
            SpawnOrUpdatePenaltyAreas();
        }
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
        // Use debounced update instead of immediate update
        ScheduleNavMeshUpdate();
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

#if WITH_EDITOR
void AMF_Field::PostEditUndo()
{
    Super::PostEditUndo();
    
    // After undo completes, we need to safely recover references to attached actors
    // This is the safe point after GIsTransacting is false
    
    // Update cached transform to current state
    CachedTransform = GetActorTransform();
    
    // Recover references to existing attached actors that may have been restored by undo
    TArray<AActor*> AttachedActors;
    GetAttachedActors(AttachedActors);
    
    // Clear invalid references first
    if (GoalA && (!IsValid(GoalA) || GoalA->IsActorBeingDestroyed()))
    {
        GoalA = nullptr;
    }
    if (GoalB && (!IsValid(GoalB) || GoalB->IsActorBeingDestroyed()))
    {
        GoalB = nullptr;
    }
    if (PenaltyAreaA && (!IsValid(PenaltyAreaA) || PenaltyAreaA->IsActorBeingDestroyed()))
    {
        PenaltyAreaA = nullptr;
    }
    if (PenaltyAreaB && (!IsValid(PenaltyAreaB) || PenaltyAreaB->IsActorBeingDestroyed()))
    {
        PenaltyAreaB = nullptr;
    }
    
    // Recover references from attached actors
    for (AActor* Attached : AttachedActors)
    {
        if (!Attached || !IsValid(Attached) || Attached->IsActorBeingDestroyed())
        {
            continue;
        }
        
        if (AMF_Goal* Goal = Cast<AMF_Goal>(Attached))
        {
            if (Goal->Tags.Contains(FName("TeamA")) && !GoalA)
            {
                GoalA = Goal;
            }
            else if (Goal->Tags.Contains(FName("TeamB")) && !GoalB)
            {
                GoalB = Goal;
            }
        }
        else if (AMF_PenaltyArea* Penalty = Cast<AMF_PenaltyArea>(Attached))
        {
            if (Penalty->Tags.Contains(FName("TeamA")) && !PenaltyAreaA)
            {
                PenaltyAreaA = Penalty;
            }
            else if (Penalty->Tags.Contains(FName("TeamB")) && !PenaltyAreaB)
            {
                PenaltyAreaB = Penalty;
            }
        }
    }
    
    // If references are still missing after undo, spawn new actors
    // Use a deferred approach to avoid any transaction issues
    if (bAutoSpawnGoals && (!IsValid(GoalA) || !IsValid(GoalB)))
    {
        SpawnOrUpdateGoals();
    }
    
    if (bAutoSpawnPenaltyAreas && (!IsValid(PenaltyAreaA) || !IsValid(PenaltyAreaB)))
    {
        SpawnOrUpdatePenaltyAreas();
    }
    
    // Schedule nav mesh update
    ScheduleNavMeshUpdate();
}
#endif

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

    const auto AttachToFieldIfNeeded = [this](AActor *Child)
    {
        if (!Child)
        {
            return;
        }

        if (Child->GetAttachParentActor() != this)
        {
            Child->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
        }
    };

    const FVector FieldCenter = GetActorLocation();
    const FVector FieldExtent = FieldBounds->GetScaledBoxExtent();
    const FVector Right = GetActorRightVector();
    const FVector Forward = GetActorForwardVector();

    const float GoalDepthExtent = GoalDepth / 2.0f;
    const FVector GoalExtent(GoalWidth / 2.0f, GoalDepthExtent, GoalHeight / 2.0f);

    // TeamA goal on RIGHT side (+Right), TeamB goal on LEFT side (-Right)
    const FVector GoalALocation = FieldCenter + Right * (FieldExtent.Y - GoalDepthExtent);
    const FRotator GoalARotation = Forward.Rotation();  // 0 degrees

    const FVector GoalBLocation = FieldCenter - Right * (FieldExtent.Y - GoalDepthExtent);
    const FRotator GoalBRotation = (-Forward).Rotation();  // 180 degrees

    // Always scan for existing attached goals to recover lost references and prevent duplicates
    TArray<AActor*> AttachedActors;
    GetAttachedActors(AttachedActors);
    
    AMF_Goal* FoundGoalA = nullptr;
    AMF_Goal* FoundGoalB = nullptr;
    
    for (AActor* Attached : AttachedActors)
    {
        if (AMF_Goal* Goal = Cast<AMF_Goal>(Attached))
        {
            if (Goal->IsActorBeingDestroyed())
            {
                continue;
            }
            
            // GoalA belongs to TeamA (has TeamA tag)
            if (Goal->Tags.Contains(FName("TeamA")))
            {
                if (!FoundGoalA)
                {
                    FoundGoalA = Goal;
                }
                else
                {
                    // Duplicate found - destroy it
                    UE_LOG(LogTemp, Warning, TEXT("MF_Field: Destroying duplicate Goal (TeamA)"));
                    Goal->Destroy();
                }
            }
            // GoalB belongs to TeamB (has TeamB tag)
            else if (Goal->Tags.Contains(FName("TeamB")))
            {
                if (!FoundGoalB)
                {
                    FoundGoalB = Goal;
                }
                else
                {
                    // Duplicate found - destroy it
                    UE_LOG(LogTemp, Warning, TEXT("MF_Field: Destroying duplicate Goal (TeamB)"));
                    Goal->Destroy();
                }
            }
        }
    }
    
    // Update references
    if (FoundGoalA)
    {
        GoalA = FoundGoalA;
    }
    if (FoundGoalB)
    {
        GoalB = FoundGoalB;
    }
    
    // Clear invalid references
    if (GoalA && (!IsValid(GoalA) || GoalA->IsActorBeingDestroyed()))
    {
        GoalA = nullptr;
    }
    if (GoalB && (!IsValid(GoalB) || GoalB->IsActorBeingDestroyed()))
    {
        GoalB = nullptr;
    }

    // Goal A
    if (!IsValid(GoalA) || GoalA->IsActorBeingDestroyed())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.ObjectFlags |= RF_Transactional;

        UClass* GoalClassToSpawn = GoalClass ? GoalClass.Get() : AMF_Goal::StaticClass();
        GoalA = Cast<AMF_Goal>(World->SpawnActor(GoalClassToSpawn, &GoalALocation, &GoalARotation, SpawnParams));

        if (GoalA)
        {
#if WITH_EDITOR
            GoalA->SetActorLabel(TEXT("MF_Goal_TeamA"));
#endif
            UE_LOG(LogTemp, Log, TEXT("MF_Field: Spawned Goal A (TeamA) at %s"), *GoalALocation.ToString());
        }
    }

    if (GoalA)
    {
        AttachToFieldIfNeeded(GoalA);
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

        UClass* GoalClassToSpawn = GoalClass ? GoalClass.Get() : AMF_Goal::StaticClass();
        GoalB = World->SpawnActor<AMF_Goal>(GoalClassToSpawn, GoalBLocation, GoalBRotation, SpawnParams);

        if (GoalB)
        {
#if WITH_EDITOR
            GoalB->SetActorLabel(TEXT("MF_Goal_TeamB"));
#endif
            UE_LOG(LogTemp, Log, TEXT("MF_Field: Spawned Goal B (TeamB) at %s"), *GoalBLocation.ToString());
        }
    }

    if (GoalB)
    {
        AttachToFieldIfNeeded(GoalB);
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

    const auto AttachToFieldIfNeeded = [this](AActor *Child)
    {
        if (!Child)
        {
            return;
        }

        if (Child->GetAttachParentActor() != this)
        {
            Child->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
        }
    };

    const FVector FieldCenter = GetActorLocation();
    const FVector FieldExtent = FieldBounds->GetScaledBoxExtent();
    const FVector Right = GetActorRightVector();
    const FVector Forward = GetActorForwardVector();

    const float PenaltyAreaHeight = 200.0f;
    // Extent: X=Width/2 (2015), Y=Length/2 (825), Z=Height (200)
    const FVector PenaltyExtent(PenaltyAreaWidth / 2.0f, PenaltyAreaLength / 2.0f, PenaltyAreaHeight);

    // TeamA penalty on RIGHT side (+Right), TeamB penalty on LEFT side (-Right)
    const FVector PenaltyALocation = FieldCenter + Right * (FieldExtent.Y - PenaltyAreaLength / 2.0f);
    const FRotator PenaltyARotation = Forward.Rotation();  // 0 degrees

    const FVector PenaltyBLocation = FieldCenter - Right * (FieldExtent.Y - PenaltyAreaLength / 2.0f);
    const FRotator PenaltyBRotation = (-Forward).Rotation();  // 180 degrees

    // ALWAYS scan attached actors to find duplicates and recover lost references
    TArray<AActor*> AttachedActors;
    GetAttachedActors(AttachedActors);
    
    TArray<AMF_PenaltyArea*> TeamAPenaltyAreas;  // PenaltyAreaA belongs to TeamA
    TArray<AMF_PenaltyArea*> TeamBPenaltyAreas;  // PenaltyAreaB belongs to TeamB
    
    for (AActor* Attached : AttachedActors)
    {
        if (AMF_PenaltyArea* Area = Cast<AMF_PenaltyArea>(Attached))
        {
            if (Area->IsActorBeingDestroyed())
            {
                continue;
            }
            
            if (Area->Tags.Contains(FName("TeamA")))
            {
                TeamAPenaltyAreas.Add(Area);
            }
            else if (Area->Tags.Contains(FName("TeamB")))
            {
                TeamBPenaltyAreas.Add(Area);
            }
        }
    }
    
    // Handle TeamA penalty areas (PenaltyAreaA) - keep first valid, destroy duplicates
    if (TeamAPenaltyAreas.Num() > 0)
    {
        // Find the one matching our reference, or use the first one
        int32 KeepIndex = 0;
        for (int32 i = 0; i < TeamAPenaltyAreas.Num(); ++i)
        {
            if (TeamAPenaltyAreas[i] == PenaltyAreaA)
            {
                KeepIndex = i;
                break;
            }
        }
        
        PenaltyAreaA = TeamAPenaltyAreas[KeepIndex];
        
        // Destroy duplicates
        for (int32 i = 0; i < TeamAPenaltyAreas.Num(); ++i)
        {
            if (i != KeepIndex && IsValid(TeamAPenaltyAreas[i]))
            {
                UE_LOG(LogTemp, Warning, TEXT("MF_Field: Destroying duplicate PenaltyAreaA (TeamA): %s"), *TeamAPenaltyAreas[i]->GetName());
                TeamAPenaltyAreas[i]->Destroy();
            }
        }
    }
    else if (!IsValid(PenaltyAreaA))
    {
        PenaltyAreaA = nullptr;
    }
    
    // Handle TeamB penalty areas (PenaltyAreaB) - keep first valid, destroy duplicates
    if (TeamBPenaltyAreas.Num() > 0)
    {
        // Find the one matching our reference, or use the first one
        int32 KeepIndex = 0;
        for (int32 i = 0; i < TeamBPenaltyAreas.Num(); ++i)
        {
            if (TeamBPenaltyAreas[i] == PenaltyAreaB)
            {
                KeepIndex = i;
                break;
            }
        }
        
        PenaltyAreaB = TeamBPenaltyAreas[KeepIndex];
        
        // Destroy duplicates
        for (int32 i = 0; i < TeamBPenaltyAreas.Num(); ++i)
        {
            if (i != KeepIndex && IsValid(TeamBPenaltyAreas[i]))
            {
                UE_LOG(LogTemp, Warning, TEXT("MF_Field: Destroying duplicate PenaltyAreaB (TeamB): %s"), *TeamBPenaltyAreas[i]->GetName());
                TeamBPenaltyAreas[i]->Destroy();
            }
        }
    }
    else if (!IsValid(PenaltyAreaB))
    {
        PenaltyAreaB = nullptr;
    }
    
    // Validate references are still valid after cleanup
    if (PenaltyAreaA && PenaltyAreaA->IsActorBeingDestroyed())
    {
        PenaltyAreaA = nullptr;
    }
    if (PenaltyAreaB && PenaltyAreaB->IsActorBeingDestroyed())
    {
        PenaltyAreaB = nullptr;
    }

    // Penalty Area A
    if (!IsValid(PenaltyAreaA) || PenaltyAreaA->IsActorBeingDestroyed())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.ObjectFlags |= RF_Transactional;

        UClass* PenaltyClassToSpawn = PenaltyAreaClass ? PenaltyAreaClass.Get() : AMF_PenaltyArea::StaticClass();
        PenaltyAreaA = World->SpawnActor<AMF_PenaltyArea>(PenaltyClassToSpawn, PenaltyALocation, PenaltyARotation, SpawnParams);

        if (PenaltyAreaA)
        {
#if WITH_EDITOR
            PenaltyAreaA->SetActorLabel(TEXT("MF_PenaltyArea_TeamA"));
#endif
            UE_LOG(LogTemp, Log, TEXT("MF_Field: Spawned Penalty Area A (TeamA) at %s"), *PenaltyALocation.ToString());
        }
    }

    if (PenaltyAreaA)
    {
        AttachToFieldIfNeeded(PenaltyAreaA);
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

        UClass* PenaltyClassToSpawn = PenaltyAreaClass ? PenaltyAreaClass.Get() : AMF_PenaltyArea::StaticClass();
        PenaltyAreaB = World->SpawnActor<AMF_PenaltyArea>(PenaltyClassToSpawn, PenaltyBLocation, PenaltyBRotation, SpawnParams);

        if (PenaltyAreaB)
        {
#if WITH_EDITOR
            PenaltyAreaB->SetActorLabel(TEXT("MF_PenaltyArea_TeamB"));
#endif
            UE_LOG(LogTemp, Log, TEXT("MF_Field: Spawned Penalty Area B (TeamB) at %s"), *PenaltyBLocation.ToString());
        }
    }

    if (PenaltyAreaB)
    {
        AttachToFieldIfNeeded(PenaltyAreaB);
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

#if WITH_EDITOR
void AMF_Field::ScheduleNavMeshUpdate()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Clear any existing timer to reset the debounce
    World->GetTimerManager().ClearTimer(NavMeshUpdateTimerHandle);
    
    // Schedule a new timer - navigation will only be rebuilt after movement stops
    World->GetTimerManager().SetTimer(
        NavMeshUpdateTimerHandle,
        this,
        &AMF_Field::OnNavMeshUpdateTimer,
        NavMeshUpdateDelay,
        false  // Don't loop
    );
}

void AMF_Field::OnNavMeshUpdateTimer()
{
    // Timer fired - movement has stopped, now update NavigationMesh
    UpdateNavMesh();
}

bool AMF_Field::NeedsRespawn() const
{
    // Check if any spawned actors have been invalidated (e.g., by undo/redo or level operations)
    if (bAutoSpawnGoals)
    {
        // If goals should exist but don't, we need to respawn
        if (!IsValid(GoalA) || !IsValid(GoalB))
        {
            return true;
        }
        
        // If goals aren't attached to us anymore, we need to respawn
        if (GoalA->GetAttachParentActor() != this || GoalB->GetAttachParentActor() != this)
        {
            return true;
        }
    }
    
    if (bAutoSpawnPenaltyAreas)
    {
        // If penalty areas should exist but don't, we need to respawn
        if (!IsValid(PenaltyAreaA) || !IsValid(PenaltyAreaB))
        {
            return true;
        }
        
        // If penalty areas aren't attached to us anymore, we need to respawn
        if (PenaltyAreaA->GetAttachParentActor() != this || PenaltyAreaB->GetAttachParentActor() != this)
        {
            return true;
        }
    }
    
    return false;
}
#endif
