/*
 * @Author: Punal Manalan
 * @Description: Automation tests for P_MiniFootball plugin
 */

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "../../Base/Match/MF_Field.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BrushComponent.h"
#include "Engine/World.h"

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

    // Check for NavMeshBoundsVolume
    TArray<AActor*> NavVolumes;
    UGameplayStatics::GetAllActorsOfClass(World, ANavMeshBoundsVolume::StaticClass(), NavVolumes);

    TestTrue("NavMeshBoundsVolume should be spawned", NavVolumes.Num() > 0);

    if (NavVolumes.Num() > 0)
    {
         ANavMeshBoundsVolume* Vol = Cast<ANavMeshBoundsVolume>(NavVolumes[0]);
         if (TestNotNull("NavVolume cast check", Vol))
         {
             // Check bounds roughly match expectation (won't be 0 if brush is set)
             FVector Extent = Vol->GetBrushComponent()->Bounds.BoxExtent;
             // We expect Extent to be > 1000.f
             TestTrue("NavMesh volume should have significant extent (Brush updated)", Extent.Size() > 1000.0f);
             
             UE_LOG(LogTemp, Log, TEXT("Verified NavMesh Volume Extent: %s"), *Extent.ToString());
         }
    }

    // Clean up
    World->DestroyWorld(false);
    return true;
}
