/*
 * Shared helpers for creating built-in input templates for MiniFootball.
 */

#pragma once

#include "CoreMinimal.h"

struct FS_InputProfile;

namespace MF_DefaultInputTemplates
{
    /** Build the built-in MiniFootball default bindings template (keyboard + gamepad). */
    FS_InputProfile BuildDefaultInputTemplate(const FName TemplateName);
}
