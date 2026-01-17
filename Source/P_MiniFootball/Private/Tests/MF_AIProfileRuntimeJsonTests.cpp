/*
 * @Author: Punal Manalan
 * @Description: Automation tests for P_MiniFootball AIProfiles runtime JSON validity
 */

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"

#include "AIBehaviour.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMFAIProfilesRuntimeJsonValidTest, "P_MiniFootball.AIProfiles.RuntimeJsonValid",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMFAIProfilesRuntimeJsonValidTest::RunTest(const FString& Parameters)
{
    const TSharedPtr<IPlugin> MiniFootballPlugin = IPluginManager::Get().FindPlugin(TEXT("P_MiniFootball"));
    if (!TestTrue(TEXT("P_MiniFootball plugin should be found"), MiniFootballPlugin.IsValid()))
    {
        return true;
    }

    const FString ProfilesDir = FPaths::Combine(MiniFootballPlugin->GetBaseDir(), TEXT("Content"), TEXT("AIProfiles"));
    if (!TestTrue(TEXT("AIProfiles directory should exist"), IFileManager::Get().DirectoryExists(*ProfilesDir)))
    {
        AddError(FString::Printf(TEXT("AIProfiles directory missing: %s"), *ProfilesDir));
        return true;
    }

    TArray<FString> RuntimeProfileFiles;
    IFileManager::Get().FindFiles(RuntimeProfileFiles, *FPaths::Combine(ProfilesDir, TEXT("*.runtime.json")), true, false);

    if (!TestTrue(TEXT("Should find at least one *.runtime.json AI profile"), RuntimeProfileFiles.Num() > 0))
    {
        AddError(FString::Printf(TEXT("No runtime profiles found in: %s"), *ProfilesDir));
        return true;
    }

    for (const FString& FileName : RuntimeProfileFiles)
    {
        const FString FullPath = FPaths::Combine(ProfilesDir, FileName);

        FString JsonString;
        if (!FFileHelper::LoadFileToString(JsonString, *FullPath))
        {
            AddError(FString::Printf(TEXT("Failed to read AI profile: %s"), *FullPath));
            continue;
        }

        UAIBehaviour* Behavior = NewObject<UAIBehaviour>();
        Behavior->EmbeddedJson = JsonString;

        FString ParseError;
        const bool bParsed = Behavior->ParseBehavior(ParseError);

        TestTrue(*FString::Printf(TEXT("%s should parse successfully"), *FileName), bParsed);
        TestTrue(*FString::Printf(TEXT("%s parse error should be empty"), *FileName), ParseError.IsEmpty());
        TestTrue(*FString::Printf(TEXT("%s behavior should be valid"), *FileName), Behavior->IsValid());

        const FAIBehaviorDef& Def = Behavior->GetBehaviorDef();
        TestTrue(*FString::Printf(TEXT("%s behavior name should not be empty"), *FileName), !Def.Name.IsEmpty());
        TestTrue(*FString::Printf(TEXT("%s initial state should not be empty"), *FileName), !Def.InitialState.IsEmpty());
        TestTrue(*FString::Printf(TEXT("%s should have at least 1 state"), *FileName), Def.States.Num() > 0);

        TSet<FString> StateIds;
        StateIds.Reserve(Def.States.Num());
        for (const FAIState& State : Def.States)
        {
            StateIds.Add(State.Id);
        }

        if (!TestTrue(*FString::Printf(TEXT("%s initial state should exist in states"), *FileName), StateIds.Contains(Def.InitialState)))
        {
            AddError(FString::Printf(TEXT("%s initialState '%s' not found among states"), *FileName, *Def.InitialState));
        }

        for (const FAIState& State : Def.States)
        {
            TMap<int32, int32> NonZeroPriorityCounts;
            for (const FAITransition& Transition : State.Transitions)
            {
                if (Transition.To.IsEmpty())
                {
                    AddError(FString::Printf(TEXT("%s: state '%s' has transition with empty 'to'"), *FileName, *State.Id));
                    continue;
                }

                if (!StateIds.Contains(Transition.To))
                {
                    AddError(FString::Printf(TEXT("%s: state '%s' transitions to missing state '%s'"), *FileName, *State.Id, *Transition.To));
                }

                // Only enforce uniqueness for explicit (non-zero) priorities.
                // Priority 0 is commonly used as a default; ties at 0 are now deterministic in the interpreter.
                if (Transition.Priority != 0)
                {
                    int32& Count = NonZeroPriorityCounts.FindOrAdd(Transition.Priority);
                    Count += 1;
                }
            }

            for (const auto& Pair : NonZeroPriorityCounts)
            {
                const int32 Priority = Pair.Key;
                const int32 Count = Pair.Value;
                if (Count > 1)
                {
                    AddError(FString::Printf(TEXT("%s: state '%s' has duplicate non-zero transition priority %d (avoid priority ties)"), *FileName, *State.Id, Priority));
                }
            }
        }
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
