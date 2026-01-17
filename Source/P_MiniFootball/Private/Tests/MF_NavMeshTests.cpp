/*
 * @Author: Punal Manalan
 * @Description: Automation tests for P_MiniFootball plugin
 */

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "../../Base/Match/MF_Field.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Components/BrushComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMFFieldNavMeshTest, "P_MiniFootball.Match.FieldNavMeshGen", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMFFieldNavMeshTest::RunTest(const FString& Parameters)
{
    // Create a temporary world
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, true);
    if (!World)
    {
        AddError("Failed to create World");
        return false;
    }

    // FWorldContext& WorldContext = GEngine->GetWorldContextFromWorldChecked(World);
    // WorldContext.SetCurrentWorld(World);

    // Spawn MF_Field
    AMF_Field* Field = World->SpawnActor<AMF_Field>();
    if (!TestNotNull("MF_Field spawned", Field))
    {
        World->DestroyWorld(false);
        return false;
    }

    // Trigger EnsureNavMesh directly
    // World->InitializeActorsForPlay(FURL());
    // World->BeginPlay();
    Field->EnsureNavMesh();

    // Check for NavMeshBoundsVolume (avoid GameplayStatics which requires a valid WorldContext)
    ANavMeshBoundsVolume* FirstNavVolume = nullptr;
    for (TActorIterator<ANavMeshBoundsVolume> It(World); It; ++It)
    {
        FirstNavVolume = *It;
        break;
    }

    TestNotNull(TEXT("NavMeshBoundsVolume should be spawned"), FirstNavVolume);

    if (FirstNavVolume)
    {
        UBrushComponent* BrushComponent = FirstNavVolume->GetBrushComponent();
        if (TestNotNull(TEXT("NavVolume BrushComponent"), BrushComponent))
        {
            // Check bounds roughly match expectation (won't be 0 if brush is set)
            const FVector Extent = BrushComponent->Bounds.BoxExtent;
            // We expect Extent to be > 1000.f
            TestTrue(TEXT("NavMesh volume should have significant extent (Brush updated)"), Extent.Size() > 1000.0f);
            UE_LOG(LogTemp, Log, TEXT("Verified NavMesh Volume Extent: %s"), *Extent.ToString());
        }
    }

    // Clean up
    World->DestroyWorld(false);
    if (GEngine)
    {
        GEngine->DestroyWorldContext(World);
    }
    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
