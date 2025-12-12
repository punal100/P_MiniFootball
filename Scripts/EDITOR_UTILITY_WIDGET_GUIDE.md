# MF Widget Blueprint Creator - Editor Utility Widget Guide

This guide explains how to create an **Editor Utility Widget (EUW)** that provides a UI for the `MF_WidgetBlueprintCreator.py` script.

Important note: the script treats `EUW_MF_WidgetCreator` as a **manual shell**. UE5 Python does not reliably expose or support editing the EUW designer widget tree, so the script will create the EUW asset, but you must build the UI layout manually.

---

## 📋 Prerequisites

1. **Python Editor Script Plugin** must be enabled

   - Edit → Plugins → Search "Python" → Enable "Python Editor Script Plugin"
   - Restart editor

2. **Editor Scripting Utilities** plugin enabled
   - Edit → Plugins → Search "Editor Scripting" → Enable it

---

## 🛠️ Step 1: Create the Editor Utility Widget

You can create this asset manually (steps below), or auto-create/repair the EUW **asset shell** via Python (no designer UI is generated):

```python
exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); MF_WBP.run_create_euw()
```

Optional (if you prefer an explicit module alias in the Python environment):

```python
exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); import MF_WidgetBlueprintCreator as MF_WBP; MF_WBP.run_create_euw()
```

1. In **Content Browser**, navigate to: `/P_MiniFootball/BP/` (or create it)
2. **Right-click** → Editor Utilities → **Editor Utility Widget**
3. Name it: `EUW_MF_WidgetCreator`
4. **Double-click** to open the Widget Blueprint editor

---

## 🎨 Step 2: Design the UI

This step is **manual**: build the widget hierarchy in UMG using the layout below.

### Recommended Layout:

```
[CanvasPanel - Root]
└── [VerticalBox - MainContainer]
    ├── [TextBlock - Title] "MF Widget Blueprint Creator"
    ├── [HorizontalBox - StatusBar]
    │   ├── [TextBlock] "Existing:"
    │   ├── [TextBlock - ExistingCount] "0"
    │   ├── [TextBlock] "Missing:"
    │   ├── [TextBlock - MissingCount] "0"
    │   ├── [TextBlock] "Issues:"
    │   └── [TextBlock - IssuesCount] "0"
    ├── [Spacer]
    ├── [Button - PreviewButton]
    │   └── [TextBlock] "🔍 Preview Changes (Dry Run)"
    ├── [Button - CreateButton]
    │   └── [TextBlock] "➕ Create Missing Widgets"
    ├── [Button - ValidateButton]
    │   └── [TextBlock] "✓ Validate All Widgets"
    ├── [Button - ForceRecreateButton]
    │   └── [TextBlock] "🔄 Force Recreate All (DANGER!)"
    ├── [Spacer]
    ├── [TextBlock] "Output Log:"
    └── [MultiLineEditableTextBox - OutputLog]
        (Read Only = true, Size: Fill)
```

### Widget Properties:

| Widget              | Property           | Value             |
| ------------------- | ------------------ | ----------------- |
| Title TextBlock     | Font Size          | 24                |
| All Buttons         | Size               | Fill Horizontally |
| ForceRecreateButton | Background Color   | Red tint          |
| OutputLog           | Is Read Only       | ✓ Checked         |
| OutputLog           | Min Desired Height | 300               |

---

## 📜 Step 3: Setup Button Events

For each button, we'll use the **Execute Python Script** node.

### 3.1 Preview Button (OnClicked)

```
Event OnClicked (PreviewButton)
    └── Execute Python Script
        Python Command: exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); MF_WBP.run_dry()
```

### 3.2 Create Button (OnClicked)

```
Event OnClicked (CreateButton)
    └── Execute Python Script
        Python Command: exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); MF_WBP.run_create()
```

### 3.3 Validate Button (OnClicked)

```
Event OnClicked (ValidateButton)
    └── Execute Python Script
        Python Command: exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); MF_WBP.run_validate()
```

### 3.4 Force Recreate Button (OnClicked)

```
Event OnClicked (ForceRecreateButton)
    └── Execute Python Script
        Python Command: exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); MF_WBP.run_force_recreate()
```

---

## 🔧 Step 4: Advanced - Status Display (Optional)

To show widget status in the UI, use **Event Construct**:

```
Event Construct
    └── Execute Python Script
        Python Command: exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); print(MF_WBP.get_status_text())
```

---

## 🚀 Step 5: Run the Editor Utility Widget

### Method 1: Right-Click Run

1. In Content Browser, **right-click** `EUW_MF_WidgetCreator`
2. Select **Run Editor Utility Widget**
3. Widget opens in a dockable Editor tab

### Method 2: Tools Menu

1. After running once, go to **Tools** menu
2. Select **Editor Utility Widgets**
3. Click `EUW_MF_WidgetCreator`

### Method 3: Keyboard Shortcut (Optional)

1. Edit → Editor Preferences → Keyboard Shortcuts
2. Search for your widget name
3. Assign a shortcut (e.g., Ctrl+Shift+W)

---

## 📝 Python Commands Reference

### Core Commands

| Action             | Python Command                                                                                                                                     |
| ------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Create EUW**     | `exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); MF_WBP.run_create_euw()`         |
| **Only EUW**       | `exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); MF_WBP.run_only_euw()`           |
| **Dry Run**        | `exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); MF_WBP.run_dry()`                |
| **Create**         | `exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); MF_WBP.run_create()`             |
| **Validate**       | `exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); MF_WBP.run_validate()`           |
| **Force Recreate** | `exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); MF_WBP.run_force_recreate()`     |
| **Get Status**     | `exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); print(MF_WBP.get_status_text())` |
| **Setup Guide**    | `exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); print_euw_setup_guide()`         |

### Diagnostic Commands

Note: diagnostics intentionally skip the EUW.

| Action                  | Python Command                                                                                                                                                                                                                   |
| ----------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Diagnose All**        | `exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); diagnose_all_mf_widgets()`                                                                                     |
| **Diagnose One Widget** | `exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); diagnose_widget_blueprint("/P_MiniFootball/BP/Widget/Components/WBP_MF_HUD")`                                  |
| **Check Bindings**      | `exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); diagnose_missing_bindings("/P_MiniFootball/BP/Widget/Components/WBP_MF_HUD", "/Script/P_MiniFootball.MF_HUD")` |

---

## 🎯 Quick Alternative: Python Console

If you prefer not to create an EUW, you can use the Python console directly:

1. **Window** → **Developer Tools** → **Output Log**
2. Change dropdown from `Cmd` to `Python`
3. Type:
   ```python
   exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read())
   ```
4. Then run any function:

   ```python
   # Core functions
   MF_WBP.run_dry()
   MF_WBP.run_create()
   MF_WBP.run_validate()
   run_only_euw()  # Create ONLY the EUW, skip all WBPs

   # Diagnostic functions
   diagnose_all_mf_widgets()  # Check all widgets
   diagnose_widget_blueprint("/P_MiniFootball/BP/Widget/Components/WBP_MF_HUD")
   ```

---

## 🔄 Widget Build Order

The script creates widgets in dependency order:

1. `WBP_MF_ActionButton` - Base control
2. `WBP_MF_VirtualJoystick` - Base control
3. `WBP_MF_SprintButton` - Base control
4. `WBP_MF_MatchInfo` - Base display
5. `WBP_MF_TeamIndicator` - Base display
6. `WBP_MF_TransitionOverlay` - Base overlay
7. `WBP_MF_QuickTeamPanel` - Base panel
8. `WBP_MF_TeamPanel` - Base panel
9. `WBP_MF_SpectatorControls` - Container (uses QuickTeamPanel)
10. `WBP_MF_GameplayControls` - Container (uses Joystick, ActionButton, SprintButton)
11. `WBP_MF_TeamSelectionPopup` - Container (uses TeamPanel)
12. `WBP_MF_PauseMenu` - Modal
13. `WBP_MF_HUD` - Master (uses all above)

---

## ⚠️ Troubleshooting

### "Python command failed"

- Ensure Python Editor Script Plugin is enabled
- Check file path is correct (use forward slashes `/`)
- Look at Output Log for detailed error

### "Parent class not found"

- Build the P_MiniFootball module first
- Ensure plugin is enabled in project

### "Asset already exists"

- Use `run_validate()` to check status
- Use `run_force_recreate()` to rebuild (caution: loses manual changes)

### Widget not appearing in Designer

- Check "Is Variable" is enabled on bound widgets
- Compile the widget blueprint
- Widget names are CASE-SENSITIVE

---

## 📚 Related Documentation

- [README.md](../README.md) - Plugin overview
- [UI_WIDGETS.md](../UI_WIDGETS.md) - Widget binding reference
- [PLAN_WBP_SCRIPT.md](../../../PLAN_WBP_SCRIPT.md) - Full automation plan

---

_Last Updated: 12/12/2025_
