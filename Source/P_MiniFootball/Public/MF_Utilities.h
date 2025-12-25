// MF_Utilities.h
// Shared utility functions for P_MiniFootball to avoid Unity Build conflicts
// These functions were previously defined in anonymous namespaces in multiple .cpp files

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Manager/CPP_InputBindingManager.h"

namespace MF_Utilities
{
    // Get the MEIS InputBindingManager from GEngine
    inline UCPP_InputBindingManager* GetMEISManager()
    {
        return GEngine ? GEngine->GetEngineSubsystem<UCPP_InputBindingManager>() : nullptr;
    }

    // Join an array of strings with a separator
    inline FString JoinStrings(const TArray<FString>& Arr, const FString& Separator)
    {
        FString Result;
        for (int32 i = 0; i < Arr.Num(); ++i)
        {
            if (i > 0)
            {
                Result += Separator;
            }
            Result += Arr[i];
        }
        return Result;
    }
}
