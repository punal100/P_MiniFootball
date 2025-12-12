#include "P_MiniFootballEditor.h"

#include "Modules/ModuleManager.h"

class FP_MiniFootballEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FP_MiniFootballEditorModule, P_MiniFootballEditor);
