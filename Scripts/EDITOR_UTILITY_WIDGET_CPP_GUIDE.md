# MF Widget Blueprint Creator - C++ Editor Module & Utility Widget Guide

This guide explains how to use the **editor-only C++ module** and **C++ Editor Utility Widget base class** that ship with `P_MiniFootball` to host the `MF_WidgetBlueprintCreator.py` UI.

It complements `EDITOR_UTILITY_WIDGET_GUIDE.md`, which shows how to build the same EUW entirely in Blueprint. Use this guide if you prefer a typed C++ parent class with a `GetWidgetSpec()` JSON description.

---

## 📦 Editor-Only Module: `P_MiniFootballEditor`

The plugin now includes a separate **editor-only** module:

- Module name: `P_MiniFootballEditor`
- Type: `Editor` (in `P_MiniFootball.uplugin`)
- Not compiled into non-editor targets
- Not packaged in **Development** or **Shipping** builds

This module is intended only for tools and editor utilities (like the MF Widget Blueprint Creator EUW). Runtime game code continues to live in the `P_MiniFootball` runtime module.

You do **not** need to reference `P_MiniFootballEditor` from your game modules; Unreal will load it automatically in the editor based on the `.uplugin` configuration.

---

## 🧱 C++ Editor Utility Widget Base Class

The editor module defines a C++ base class for the EUW:

- Header: `Source/P_MiniFootballEditor/Base/UI/MF_WidgetCreatorUtilityWidget.h`
- Class: `UMF_WidgetCreatorUtilityWidget`
- Parent: `UEditorUtilityWidget`
- API macro: `P_MINIFOOTBALLEDITOR_API`
- Key function: `static FString GetWidgetSpec()`

Example declaration:

```cpp
UCLASS()
class P_MINIFOOTBALLEDITOR_API UMF_WidgetCreatorUtilityWidget : public UEditorUtilityWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="MiniFootball|WidgetSpec")
    static FString GetWidgetSpec();
};
```

> Note: `UFUNCTION` return types should be **by value** (e.g., `FString`), not `const FString&`, to remain compatible with Unreal reflection (Blueprint/Python).

The implementation returns a **JSON specification** describing the recommended layout and bindings for the Editor Utility Widget, mirroring the style used by the runtime UMF\_\* widgets in `P_MiniFootball`.

---

## 🧩 JSON Spec: `GetWidgetSpec()`

`UMF_WidgetCreatorUtilityWidget::GetWidgetSpec()` returns JSON with:

- `WidgetClass`: `UMF_WidgetCreatorUtilityWidget`
- `BlueprintName`: `EUW_MF_WidgetCreator`
- `ParentClass`: `/Script/P_MiniFootballEditor.MF_WidgetCreatorUtilityWidget`
- `Hierarchy`: Root panel + children (Title, StatusBar, buttons, OutputLog)
- `Bindings`: Required vs optional widget names

This spec is **informational/documentation only**; it is not currently consumed by `MF_WidgetBlueprintCreator.py`. You can use it for tooling, validation, or to keep your EUW layout self-describing alongside C++.

Example usage (C++):

```cpp
FString JsonSpec = UMF_WidgetCreatorUtilityWidget::GetWidgetSpec();
// Optionally parse or log the JSON for tooling.
```

Example usage (Python in the editor):

```python
import unreal, json

json_str = unreal.MF_WidgetCreatorUtilityWidget.get_widget_spec()
spec = json.loads(json_str)
print(spec["BlueprintName"])  # "EUW_MF_WidgetCreator"
```

---

## 🛠️ Creating the EUW from C++ Base

You still create the Editor Utility Widget asset in the **Content Browser**, but now you set its parent class to `UMF_WidgetCreatorUtilityWidget`.

Alternatively, the Python automation script can create/repair the EUW **asset shell** automatically (including setting the correct C++ parent). The EUW designer layout is still created manually in UMG.

```python
exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); MF_WBP.run_create_euw()
```

1. In **Content Browser**, navigate to: `/P_MiniFootball/BP/` (or create it).
2. **Right-click** → Editor Utilities → **Editor Utility Widget**.
3. Name it: `EUW_MF_WidgetCreator`.
4. Open the asset, and in the **Class Settings** panel:
   - Set **Parent Class** to `UMF_WidgetCreatorUtilityWidget`.

This gives you:

- A typed C++ base (for future logic or common helpers).
- Access to `GetWidgetSpec()` directly in Blueprint and C++.

> Layout & bindings are still created visually in UMG, as described in `EDITOR_UTILITY_WIDGET_GUIDE.md`. The JSON spec exists as a self-describing contract and for optional tooling.

---

## 🎨 Recommended Layout (Recap)

`GetWidgetSpec()` encodes the same recommended layout as the original guide:

- Root: `CanvasPanel` → `VerticalBox` (`MainContainer`).
- Header text: `Title` `TextBlock`.
- Status bar: `StatusBar` `HorizontalBox` with `ExistingCount`, `MissingCount`, `IssuesCount` `TextBlock`s.
- Action buttons: `PreviewButton`, `CreateButton`, `ValidateButton`, `ForceRecreateButton`.
- Output area: `OutputLog` `MultiLineEditableTextBox`.

See `EDITOR_UTILITY_WIDGET_GUIDE.md` for detailed widget property recommendations (font sizes, colors, etc.).

---

## 🔗 Hooking Buttons to Python (Same as Blueprint-Only Guide)

With the C++ parent in place, you still wire up the buttons exactly as before, using **Execute Python Script** nodes in the EUW graph. Typical commands:

```text
exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); MF_WBP.run_dry()
exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); MF_WBP.run_create()
exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); MF_WBP.run_validate()
exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); MF_WBP.run_force_recreate()
exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); run_only_euw()
```

### Diagnostic Commands (for troubleshooting)

```text
exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); diagnose_all_mf_widgets()
exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); diagnose_widget_blueprint("/P_MiniFootball/BP/Widget/Components/WBP_MF_HUD")
```

Refer back to `EDITOR_UTILITY_WIDGET_GUIDE.md` for the full per-button event setup.

---

## 🚫 Packaging Behavior

Because:

- `P_MiniFootballEditor` is declared as an **Editor** module in `P_MiniFootball.uplugin`, and
- `P_MiniFootballEditor.Build.cs` explicitly disables builds for non-editor targets,

this module (and the `UMF_WidgetCreatorUtilityWidget` class) are **never included** in packaged **Development** or **Shipping** builds. They exist purely for editor-time tooling.

You do not need to take any additional steps to exclude them from packages.

---

## 📚 Related Files

- Runtime plugin guide: `Plugins/P_MiniFootball/README.md`
- Blueprint-only EUW guide: `Plugins/P_MiniFootball/Scripts/EDITOR_UTILITY_WIDGET_GUIDE.md`
- C++ editor module & EUW guide (this file): `Plugins/P_MiniFootball/Scripts/EDITOR_UTILITY_WIDGET_CPP_GUIDE.md`
- Editor module build: `Plugins/P_MiniFootball/Source/P_MiniFootballEditor/P_MiniFootballEditor.Build.cs`
- Editor module entry: `Plugins/P_MiniFootball/P_MiniFootball.uplugin`
- C++ EUW base class: `Plugins/P_MiniFootball/Source/P_MiniFootballEditor/Base/UI/MF_WidgetCreatorUtilityWidget.h/.cpp`

---

_Last Updated: 12/12/2025_
