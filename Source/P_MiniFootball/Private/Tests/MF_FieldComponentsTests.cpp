/*
 * @Author: Punal Manalan
 * @Description: Automation tests for MF_Field automatic components (Goals + Penalty Areas)
 * @Date: 17/01/2026
 */

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/BoxComponent.h"

#include "../../Base/Match/MF_Field.h"
#include "../../Base/Match/MF_Goal.h"
#include "../../Base/Match/MF_PenaltyArea.h"

static void MF_DestroyTestWorld(UWorld *World)
{
    if (!World)
    {
        return;
    }

    World->DestroyWorld(false);
    if (GEngine)
    {
        GEngine->DestroyWorldContext(World);
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMF_FieldAutoSpawnGoals,
                                 "P_MiniFootball.Match.Field.AutoSpawnGoals",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMF_FieldAutoSpawnGoals::RunTest(const FString &Parameters)
{
    UWorld *World = UWorld::CreateWorld(EWorldType::Game, true);
    if (!World)
    {
        AddError("Failed to create World");
        return false;
    }

    AMF_Field *Field = World->SpawnActor<AMF_Field>();
    if (!TestNotNull(TEXT("MF_Field spawned"), Field))
    {
        MF_DestroyTestWorld(World);
        return false;
    }

    Field->bAutoSpawnGoals = true;
    Field->GoalDepth = 50.0f;
    Field->GoalWidth = MF_Constants::GoalWidth;
    Field->GoalHeight = MF_Constants::GoalHeight;

    Field->RerunConstructionScripts();

    TestNotNull(TEXT("GoalA should be spawned"), Field->GoalA);
    TestNotNull(TEXT("GoalB should be spawned"), Field->GoalB);

    if (Field->GoalA)
    {
        TestEqual(TEXT("GoalA DefendingTeam"), Field->GoalA->DefendingTeam, EMF_TeamID::TeamA);
        TestTrue(TEXT("GoalA has Goal tag"), Field->GoalA->Tags.Contains(FName("Goal")));
        TestTrue(TEXT("GoalA has TeamA tag"), Field->GoalA->Tags.Contains(FName("TeamA")));
        TestFalse(TEXT("GoalA does not have TeamB tag"), Field->GoalA->Tags.Contains(FName("TeamB")));

        if (Field->GoalA->GoalTrigger)
        {
            const FVector Extent = Field->GoalA->GoalTrigger->GetUnscaledBoxExtent();
            TestTrue(TEXT("GoalA depth extent ~= GoalDepth/2"), FMath::IsNearlyEqual(Extent.X, Field->GoalDepth / 2.0f, 0.01f));
            TestTrue(TEXT("GoalA width extent ~= GoalWidth/2"), FMath::IsNearlyEqual(Extent.Y, Field->GoalWidth / 2.0f, 0.01f));
            TestTrue(TEXT("GoalA height extent ~= GoalHeight/2"), FMath::IsNearlyEqual(Extent.Z, Field->GoalHeight / 2.0f, 0.01f));
        }
    }

    if (Field->GoalB)
    {
        TestEqual(TEXT("GoalB DefendingTeam"), Field->GoalB->DefendingTeam, EMF_TeamID::TeamB);
        TestTrue(TEXT("GoalB has Goal tag"), Field->GoalB->Tags.Contains(FName("Goal")));
        TestTrue(TEXT("GoalB has TeamB tag"), Field->GoalB->Tags.Contains(FName("TeamB")));
        TestFalse(TEXT("GoalB does not have TeamA tag"), Field->GoalB->Tags.Contains(FName("TeamA")));
    }

    MF_DestroyTestWorld(World);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMF_FieldAutoSpawnPenaltyAreas,
                                 "P_MiniFootball.Match.Field.AutoSpawnPenaltyAreas",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMF_FieldAutoSpawnPenaltyAreas::RunTest(const FString &Parameters)
{
    UWorld *World = UWorld::CreateWorld(EWorldType::Game, true);
    if (!World)
    {
        AddError("Failed to create World");
        return false;
    }

    AMF_Field *Field = World->SpawnActor<AMF_Field>();
    if (!TestNotNull(TEXT("MF_Field spawned"), Field))
    {
        MF_DestroyTestWorld(World);
        return false;
    }

    Field->bAutoSpawnPenaltyAreas = true;
    Field->PenaltyAreaLength = MF_Constants::PenaltyAreaLength;
    Field->PenaltyAreaWidth = MF_Constants::PenaltyAreaWidth;

    Field->RerunConstructionScripts();

    TestNotNull(TEXT("PenaltyAreaA should be spawned"), Field->PenaltyAreaA);
    TestNotNull(TEXT("PenaltyAreaB should be spawned"), Field->PenaltyAreaB);

    if (Field->PenaltyAreaA)
    {
        TestEqual(TEXT("PenaltyAreaA DefendingTeam"), Field->PenaltyAreaA->DefendingTeam, EMF_TeamID::TeamA);
        TestTrue(TEXT("PenaltyAreaA has PenaltyArea tag"), Field->PenaltyAreaA->Tags.Contains(FName("PenaltyArea")));
        TestTrue(TEXT("PenaltyAreaA has TeamA tag"), Field->PenaltyAreaA->Tags.Contains(FName("TeamA")));
        TestFalse(TEXT("PenaltyAreaA does not have TeamB tag"), Field->PenaltyAreaA->Tags.Contains(FName("TeamB")));

        if (Field->PenaltyAreaA->PenaltyAreaBounds)
        {
            const FVector Extent = Field->PenaltyAreaA->PenaltyAreaBounds->GetUnscaledBoxExtent();
            TestTrue(TEXT("PenaltyAreaA length extent ~= PenaltyAreaLength/2"), FMath::IsNearlyEqual(Extent.X, Field->PenaltyAreaLength / 2.0f, 0.01f));
            TestTrue(TEXT("PenaltyAreaA width extent ~= PenaltyAreaWidth/2"), FMath::IsNearlyEqual(Extent.Y, Field->PenaltyAreaWidth / 2.0f, 0.01f));
        }
    }

    if (Field->PenaltyAreaB)
    {
        TestEqual(TEXT("PenaltyAreaB DefendingTeam"), Field->PenaltyAreaB->DefendingTeam, EMF_TeamID::TeamB);
        TestTrue(TEXT("PenaltyAreaB has PenaltyArea tag"), Field->PenaltyAreaB->Tags.Contains(FName("PenaltyArea")));
        TestTrue(TEXT("PenaltyAreaB has TeamB tag"), Field->PenaltyAreaB->Tags.Contains(FName("TeamB")));
        TestFalse(TEXT("PenaltyAreaB does not have TeamA tag"), Field->PenaltyAreaB->Tags.Contains(FName("TeamA")));
    }

    MF_DestroyTestWorld(World);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMF_PenaltyAreaIsLocationInside,
                                 "P_MiniFootball.Match.PenaltyArea.IsLocationInside",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMF_PenaltyAreaIsLocationInside::RunTest(const FString &Parameters)
{
    UWorld *World = UWorld::CreateWorld(EWorldType::Game, true);
    if (!World)
    {
        AddError("Failed to create World");
        return false;
    }

    AMF_PenaltyArea *PenaltyArea = World->SpawnActor<AMF_PenaltyArea>();
    if (!TestNotNull(TEXT("MF_PenaltyArea spawned"), PenaltyArea))
    {
        MF_DestroyTestWorld(World);
        return false;
    }

    if (PenaltyArea->PenaltyAreaBounds)
    {
        PenaltyArea->PenaltyAreaBounds->SetBoxExtent(FVector(100.0f, 200.0f, 50.0f));
    }

    const FVector Center = PenaltyArea->GetActorLocation();

    TestTrue(TEXT("Center should be inside"), PenaltyArea->IsLocationInside(Center));
    TestTrue(TEXT("Point inside X"), PenaltyArea->IsLocationInside(Center + FVector(50.0f, 0.0f, 0.0f)));
    TestTrue(TEXT("Point inside Y"), PenaltyArea->IsLocationInside(Center + FVector(0.0f, 100.0f, 0.0f)));
    TestFalse(TEXT("Point outside X"), PenaltyArea->IsLocationInside(Center + FVector(150.0f, 0.0f, 0.0f)));
    TestFalse(TEXT("Point outside Y"), PenaltyArea->IsLocationInside(Center + FVector(0.0f, 250.0f, 0.0f)));

    MF_DestroyTestWorld(World);
    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
