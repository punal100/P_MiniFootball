#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "MF_WidgetCreatorUtilityWidget.generated.h"

/**
 * UMF_WidgetCreatorUtilityWidget
 *
 * Editor-only utility widget class used as the C++ parent for the
 * MF Widget Blueprint Creator EUW (EUW_MF_WidgetCreator).
 *
 * Exposes a self-describing JSON spec via GetWidgetSpec(), mirroring
 * the runtime UMF_* widget pattern used in the P_MiniFootball module.
 */
UCLASS(Blueprintable, BlueprintType)
class P_MINIFOOTBALLEDITOR_API UMF_WidgetCreatorUtilityWidget : public UEditorUtilityWidget
{
    GENERATED_BODY()

public:
    /**
     * Return a JSON specification describing the recommended layout
     * and bindings for the Editor Utility Widget UI.
     *
     * This is primarily for documentation and tooling. MWCS ships its own
     * editor UI and does not require this EUW.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MiniFootball|WidgetSpec")
    static FString GetWidgetSpec();
};
