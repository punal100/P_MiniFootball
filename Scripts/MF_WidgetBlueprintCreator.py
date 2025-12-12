# -*- coding: utf-8 -*-
"""
MF_WidgetBlueprintCreator.py
============================
Automated Widget Blueprint creation script for P_MiniFootball plugin.

This script creates and configures Widget Blueprints based on:
- UI_WIDGETS.md specifications
- Self-describing JSON specs embedded in C++ UMF_* widget classes

USAGE:
======
1. Python Console:
   py "D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py"

2. Editor Utility Widget (Recommended):
   - Create Editor Utility Widget in Content Browser
   - Add buttons that call: Execute Python Command node
   - Use commands like: MF_WBP.run_dry() or MF_WBP.run_create()

3. Quick Functions (after running script once):
   import MF_WidgetBlueprintCreator as MF_WBP
   MF_WBP.run_dry()            # Preview changes
   MF_WBP.run_create()         # Create missing widgets
    MF_WBP.run_create_euw()     # Create/repair EUW_MF_WidgetCreator tool asset
   MF_WBP.run_validate()       # Validate existing widgets
   MF_WBP.run_force_recreate() # Recreate all (CAUTION!)

Features:
- Creates Widget Blueprints with correct C++ parent classes
- Reads JSON specs from UMF_* C++ classes (GetWidgetSpec())
- Handles existing assets (skip, update, or recreate)
- Validates widget bindings and hierarchy
- Auto-applies theme colors and layout rules
- Detailed reporting and error handling

Author: P_MiniFootball Team
Based on: UI_WIDGETS.md documentation + C++ JSON Widget Specifications
Last Updated: 11/12/2025
"""

import unreal
import json
import sys
import os
from enum import Enum
from typing import Dict, List, Optional, Tuple, Any

# =============================================================================
# CONFIGURATION
# =============================================================================

# Plugin content paths
PLUGIN_CONTENT_PATH = "/P_MiniFootball/BP/Widget"
PLUGIN_COMPONENTS_PATH = "/P_MiniFootball/BP/Widget/Components"

# Editor Utility Widget (EUW) asset path
PLUGIN_TOOLS_PATH = "/P_MiniFootball/BP"

# C++ Module name
CPP_MODULE = "/Script/P_MiniFootball"

# C++ Editor module (editor-only)
CPP_EDITOR_MODULE = "/Script/P_MiniFootballEditor"

# Script path (for Editor Utility Widget reference)
SCRIPT_PATH = "D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py"

# =============================================================================
# JSON SPEC READING FROM C++ CLASSES
# =============================================================================

def get_widget_spec_from_cpp(class_name: str) -> Optional[Dict]:
    """
    Read the embedded JSON widget specification from a C++ UMF widget class.
    
    Args:
        class_name: Name of the class (e.g., "MF_ActionButton" or "UMF_ActionButton")
    
    Returns:
        Parsed JSON dict or None if not available
    """
    try:
        # Normalize class name (remove U prefix if present)
        clean_name = class_name
        if clean_name.startswith("U"):
            clean_name = clean_name[1:]
        if not clean_name.startswith("MF_"):
            clean_name = f"MF_{clean_name}"
        
        # Try to load the class
        class_path = f"{CPP_MODULE}.{clean_name}"
        widget_class = unreal.load_class(None, class_path)
        
        if not widget_class:
            return None
        
        # Get the CDO (Class Default Object) and call GetWidgetSpec
        cdo = unreal.get_default_object(widget_class)
        if cdo and hasattr(cdo, 'get_widget_spec'):
            json_str = cdo.get_widget_spec()
            if json_str:
                return json.loads(json_str)
        
        # Alternative: try static method on class
        if hasattr(widget_class, 'get_widget_spec'):
            json_str = widget_class.get_widget_spec()
            if json_str:
                return json.loads(json_str)
                
    except Exception as e:
        # Silent fail - JSON specs are optional enhancement
        pass
    
    return None

def get_all_widget_specs() -> Dict[str, Dict]:
    """
    Retrieve all available widget specs from C++ classes.
    
    Returns:
        Dict mapping widget name to JSON spec
    """
    specs = {}
    
    # List of known UMF widget classes
    widget_classes = [
        "MF_ActionButton",
        "MF_VirtualJoystick", 
        "MF_SprintButton",
        "MF_MatchInfo",
        "MF_TeamIndicator",
        "MF_TransitionOverlay",
        "MF_TeamPanel",
        "MF_QuickTeamPanel",
        "MF_SpectatorControls",
        "MF_GameplayControls",
        "MF_TeamSelectionPopup",
        "MF_PauseMenu",
        "MF_HUD",
    ]
    
    for class_name in widget_classes:
        spec = get_widget_spec_from_cpp(class_name)
        if spec:
            bp_name = spec.get("BlueprintName", f"WBP_{class_name}")
            specs[bp_name] = spec
    
    return specs

# =============================================================================
# CONFIGURATION: Theme/Layout Options
# =============================================================================

# Apply theme and layout ONLY to newly created widgets
APPLY_THEME_TO_NEW = True
APPLY_LAYOUT_TO_NEW = True

# For existing widgets, controlled via booleans
APPLY_THEME_TO_EXISTING = False
APPLY_LAYOUT_TO_EXISTING = False

# =============================================================================
# ENUMS AND CONSTANTS
# =============================================================================

class AssetAction(Enum):
    """Action to take when asset already exists."""
    SKIP = "skip"           # Skip existing assets
    UPDATE = "update"       # Update/validate existing assets
    RECREATE = "recreate"   # Delete and recreate assets

class ValidationResult(Enum):
    """Result of widget validation."""
    VALID = "valid"
    MISSING_BINDINGS = "missing_bindings"
    WRONG_PARENT = "wrong_parent"
    INVALID_STRUCTURE = "invalid_structure"
    NOT_FOUND = "not_found"

# =============================================================================
# VISUAL DESIGN CONSTANTS (from UI_WIDGETS.md)
# =============================================================================

COLORS = {
    # Team Colors
    "TEAM_A_RED": unreal.LinearColor(r=0.8, g=0.2, b=0.2, a=1.0),      # #CC3333
    "TEAM_B_BLUE": unreal.LinearColor(r=0.2, g=0.2, b=0.8, a=1.0),     # #3333CC
    "SPECTATOR_GRAY": unreal.LinearColor(r=0.3, g=0.3, b=0.3, a=1.0),  # #4D4D4D
    "NEUTRAL_DARK": unreal.LinearColor(r=0.1, g=0.1, b=0.1, a=1.0),    # #1A1A1A
    
    # UI Colors
    "BACKGROUND": unreal.LinearColor(r=0.0, g=0.0, b=0.0, a=0.7),      # Black 70%
    "PANEL_BG": unreal.LinearColor(r=0.165, g=0.165, b=0.165, a=1.0),  # #2A2A2A
    "BUTTON_NORMAL": unreal.LinearColor(r=0.24, g=0.24, b=0.24, a=1.0), # #3D3D3D
    "BUTTON_HOVER": unreal.LinearColor(r=0.3, g=0.3, b=0.3, a=1.0),    # #4D4D4D
    "BUTTON_PRESSED": unreal.LinearColor(r=0.165, g=0.165, b=0.165, a=1.0), # #2A2A2A
    "ACCENT_GREEN": unreal.LinearColor(r=0.2, g=0.8, b=0.2, a=1.0),    # #33CC33
    
    # Text Colors
    "TEXT_WHITE": unreal.LinearColor(r=1.0, g=1.0, b=1.0, a=1.0),      # #FFFFFF
    "TEXT_GRAY": unreal.LinearColor(r=0.7, g=0.7, b=0.7, a=1.0),       # #B3B3B3
    "TEXT_HINT": unreal.LinearColor(r=0.4, g=0.4, b=0.4, a=1.0),       # #666666
    "WARNING_YELLOW": unreal.LinearColor(r=1.0, g=0.8, b=0.0, a=1.0),  # #FFCC00
}

# Widget sizes (width, height) - for Custom size mode widgets
WIDGET_SIZES = {
    "WBP_MF_MatchInfo": (800, 80),
    "WBP_MF_TeamIndicator": (180, 40),
    "WBP_MF_VirtualJoystick": (200, 200),
    "WBP_MF_ActionButton": (120, 120),
    "WBP_MF_SprintButton": (80, 80),
    "WBP_MF_TeamPanel": (280, 350),
    "WBP_MF_QuickTeamPanel": (220, 50),
}

# =============================================================================
# WIDGET DEFINITIONS (from UI_WIDGETS.md)
# =============================================================================

# Widget type mappings for UE5
WIDGET_TYPE_MAP = {
    "TextBlock": unreal.TextBlock,
    "Button": unreal.Button,
    "Image": unreal.Image,
    "Border": unreal.Border,
    "CanvasPanel": unreal.CanvasPanel,
    "VerticalBox": unreal.VerticalBox,
    "HorizontalBox": unreal.HorizontalBox,
    "Overlay": unreal.Overlay,
    "WidgetSwitcher": unreal.WidgetSwitcher,
    "Throbber": unreal.Throbber,
    # EUW-only convenience widgets
    "Spacer": unreal.Spacer,
    "MultiLineEditableTextBox": unreal.MultiLineEditableTextBox,
}

# =============================================================================
# EDITOR UTILITY WIDGET (EUW) DEFINITION
# =============================================================================

EUW_NAME = "EUW_MF_WidgetCreator"
EUW_DEFINITION = {
    "parent_class": f"{CPP_EDITOR_MODULE}.MF_WidgetCreatorUtilityWidget",
    "path": PLUGIN_TOOLS_PATH,
    # Names mirror EDITOR_UTILITY_WIDGET_GUIDE.md
    "required_widgets": [
        {"name": "MainContainer", "type": "VerticalBox"},
        {"name": "Title", "type": "TextBlock", "text": "MF Widget Blueprint Creator"},
        {"name": "StatusBar", "type": "HorizontalBox"},
        {"name": "ExistingCount", "type": "TextBlock", "text": "0"},
        {"name": "MissingCount", "type": "TextBlock", "text": "0"},
        {"name": "IssuesCount", "type": "TextBlock", "text": "0"},
        {"name": "PreviewButton", "type": "Button"},
        {"name": "CreateButton", "type": "Button"},
        {"name": "ValidateButton", "type": "Button"},
        {"name": "ForceRecreateButton", "type": "Button"},
        {"name": "OutputLog", "type": "MultiLineEditableTextBox"},
    ],
    "optional_widgets": [
        {"name": "OutputLabel", "type": "TextBlock", "text": "Output Log:"},
        {"name": "Spacer_Top", "type": "Spacer"},
        {"name": "Spacer_Bottom", "type": "Spacer"},
    ],
}

# Complete widget definitions based on UI_WIDGETS.md
WIDGET_DEFINITIONS = {
    # =========================================================================
    # PHASE 1: Base Widgets (no WBP dependencies)
    # =========================================================================
    
    "WBP_MF_MatchInfo": {
        "parent_class": f"{CPP_MODULE}.MF_MatchInfo",
        "path": PLUGIN_COMPONENTS_PATH,
        "fill_screen": False,
        "design_size": (800, 80),
        "required_widgets": [
            {"name": "TeamAScoreText", "type": "TextBlock", "text": "0"},
            {"name": "TeamBScoreText", "type": "TextBlock", "text": "0"},
            {"name": "MatchTimerText", "type": "TextBlock", "text": "00:00"},
        ],
        "optional_widgets": [
            {"name": "MatchPhaseText", "type": "TextBlock", "text": "Kickoff"},
            {"name": "TeamANameText", "type": "TextBlock", "text": "TEAM A"},
            {"name": "TeamBNameText", "type": "TextBlock", "text": "TEAM B"},
        ],
    },
    
    "WBP_MF_TeamIndicator": {
        "parent_class": f"{CPP_MODULE}.MF_TeamIndicator",
        "path": PLUGIN_COMPONENTS_PATH,
        "fill_screen": False,
        "design_size": (180, 40),
        "required_widgets": [
            {"name": "TeamText", "type": "TextBlock", "text": "SPECTATOR"},
        ],
        "optional_widgets": [
            {"name": "TeamColorBorder", "type": "Border"},
            {"name": "TeamIcon", "type": "Image"},
        ],
    },
    
    "WBP_MF_TransitionOverlay": {
        "parent_class": f"{CPP_MODULE}.MF_TransitionOverlay",
        "path": PLUGIN_COMPONENTS_PATH,
        "fill_screen": True,
        "required_widgets": [
            {"name": "StatusText", "type": "TextBlock", "text": "Loading..."},
        ],
        "optional_widgets": [
            {"name": "LoadingThrobber", "type": "Throbber"},
            {"name": "BackgroundOverlay", "type": "Image"},
        ],
    },
    
    "WBP_MF_VirtualJoystick": {
        "parent_class": f"{CPP_MODULE}.MF_VirtualJoystick",
        "path": PLUGIN_COMPONENTS_PATH,
        "fill_screen": False,
        "design_size": (200, 200),
        "required_widgets": [
            {"name": "JoystickBase", "type": "Image"},
            {"name": "JoystickThumb", "type": "Image"},
        ],
        "optional_widgets": [],
    },
    
    "WBP_MF_ActionButton": {
        "parent_class": f"{CPP_MODULE}.MF_ActionButton",
        "path": PLUGIN_COMPONENTS_PATH,
        "fill_screen": False,
        "design_size": (120, 120),
        "required_widgets": [
            {"name": "ActionButton", "type": "Button"},
        ],
        "optional_widgets": [
            {"name": "ActionIcon", "type": "Image"},
            {"name": "ActionText", "type": "TextBlock", "text": "KICK"},
        ],
    },
    
    "WBP_MF_SprintButton": {
        "parent_class": f"{CPP_MODULE}.MF_SprintButton",
        "path": PLUGIN_COMPONENTS_PATH,
        "fill_screen": False,
        "design_size": (80, 80),
        "required_widgets": [
            {"name": "SprintButton", "type": "Button"},
        ],
        "optional_widgets": [
            {"name": "SprintIcon", "type": "Image"},
            {"name": "SprintText", "type": "TextBlock", "text": "SPRINT"},
        ],
    },
    
    "WBP_MF_TeamPanel": {
        "parent_class": f"{CPP_MODULE}.MF_TeamPanel",
        "path": PLUGIN_COMPONENTS_PATH,
        "fill_screen": False,
        "design_size": (280, 350),
        "required_widgets": [
            {"name": "TeamNameText", "type": "TextBlock", "text": "TEAM A"},
            {"name": "PlayerCountText", "type": "TextBlock", "text": "Players: 0/3"},
            {"name": "PlayerListBox", "type": "VerticalBox"},
            {"name": "JoinButton", "type": "Button"},
        ],
        "optional_widgets": [
            {"name": "PanelBorder", "type": "Border"},
            {"name": "JoinButtonText", "type": "TextBlock", "text": "JOIN TEAM"},
        ],
    },
    
    "WBP_MF_QuickTeamPanel": {
        "parent_class": f"{CPP_MODULE}.MF_QuickTeamPanel",
        "path": PLUGIN_COMPONENTS_PATH,
        "fill_screen": False,
        "design_size": (220, 50),
        "required_widgets": [
            {"name": "TeamNameText", "type": "TextBlock", "text": "TEAM A"},
            {"name": "PlayerCountText", "type": "TextBlock", "text": "(0)"},
            {"name": "QuickJoinButton", "type": "Button"},
        ],
        "optional_widgets": [
            {"name": "PanelBorder", "type": "Border"},
            {"name": "PlayerListBox", "type": "VerticalBox"},
            {"name": "ShortcutHintText", "type": "TextBlock", "text": "(1)"},
        ],
    },
    
    "WBP_MF_ScorePopup": {
        "parent_class": f"{CPP_MODULE}.MF_ScorePopup",
        "path": PLUGIN_COMPONENTS_PATH,
        "fill_screen": False,
        "design_size": (400, 200),
        "required_widgets": [],
        "optional_widgets": [
            {"name": "ScoreText", "type": "TextBlock", "text": "GOAL!"},
            {"name": "ScorerNameText", "type": "TextBlock", "text": "Player"},
            {"name": "BackgroundImage", "type": "Image"},
        ],
    },
    
    # =========================================================================
    # PHASE 2: Container Widgets (reference other WBPs)
    # =========================================================================
    
    "WBP_MF_SpectatorControls": {
        "parent_class": f"{CPP_MODULE}.MF_SpectatorControls",
        "path": PLUGIN_COMPONENTS_PATH,
        "fill_screen": True,
        "required_widgets": [],  # All optional!
        "optional_widgets": [
            {"name": "SpectatingLabel", "type": "TextBlock", "text": "SPECTATING"},
            {"name": "CameraModeText", "type": "TextBlock", "text": "Free Camera"},
            {"name": "OpenTeamSelectButton", "type": "Button"},
            {"name": "ControlHintsText", "type": "TextBlock", "text": "[F] Camera [TAB] Teams"},
        ],
        "custom_widgets": [
            {"name": "QuickTeamA", "type": "WBP_MF_QuickTeamPanel"},
            {"name": "QuickTeamB", "type": "WBP_MF_QuickTeamPanel"},
        ],
    },
    
    "WBP_MF_GameplayControls": {
        "parent_class": f"{CPP_MODULE}.MF_GameplayControls",
        "path": PLUGIN_COMPONENTS_PATH,
        "fill_screen": True,
        "required_widgets": [],
        "optional_widgets": [
            {"name": "LeftControlContainer", "type": "Overlay"},
            {"name": "RightControlContainer", "type": "Overlay"},
        ],
        "custom_widgets": [
            {"name": "MovementJoystick", "type": "WBP_MF_VirtualJoystick", "required": True},
            {"name": "ActionButton", "type": "WBP_MF_ActionButton", "required": True},
            {"name": "SprintButton", "type": "WBP_MF_SprintButton", "required": False},
        ],
    },
    
    "WBP_MF_TeamSelectionPopup": {
        "parent_class": f"{CPP_MODULE}.MF_TeamSelectionPopup",
        "path": PLUGIN_COMPONENTS_PATH,
        "fill_screen": True,
        "required_widgets": [
            {"name": "CloseButton", "type": "Button"},
        ],
        "optional_widgets": [
            {"name": "TitleText", "type": "TextBlock", "text": "SELECT TEAM"},
            {"name": "AutoAssignButton", "type": "Button"},
            {"name": "BackgroundOverlay", "type": "Overlay"},
            {"name": "StatusText", "type": "TextBlock", "text": ""},
        ],
        "custom_widgets": [
            {"name": "TeamAPanel", "type": "WBP_MF_TeamPanel", "required": True},
            {"name": "TeamBPanel", "type": "WBP_MF_TeamPanel", "required": True},
        ],
    },
    
    "WBP_MF_PauseMenu": {
        "parent_class": f"{CPP_MODULE}.MF_PauseMenu",
        "path": PLUGIN_COMPONENTS_PATH,
        "fill_screen": True,
        "required_widgets": [
            {"name": "ResumeButton", "type": "Button"},
            {"name": "LeaveTeamButton", "type": "Button"},
            {"name": "QuitButton", "type": "Button"},
        ],
        "optional_widgets": [
            {"name": "TitleText", "type": "TextBlock", "text": "PAUSED"},
            {"name": "CurrentTeamText", "type": "TextBlock", "text": ""},
            {"name": "ChangeTeamButton", "type": "Button"},
            {"name": "SettingsButton", "type": "Button"},
            {"name": "MenuContainer", "type": "VerticalBox"},
            {"name": "BackgroundOverlay", "type": "Overlay"},
        ],
    },
    
    # =========================================================================
    # PHASE 3: Main HUD (references all above)
    # =========================================================================
    
    "WBP_MF_HUD": {
        "parent_class": f"{CPP_MODULE}.MF_HUD",
        "path": PLUGIN_CONTENT_PATH,
        "fill_screen": True,
        "required_widgets": [
            {"name": "ModeSwitcher", "type": "WidgetSwitcher"},
        ],
        "optional_widgets": [
            {"name": "RootCanvas", "type": "CanvasPanel"},
        ],
        "custom_widgets": [
            {"name": "MatchInfo", "type": "WBP_MF_MatchInfo", "required": True},
            {"name": "TeamIndicator", "type": "WBP_MF_TeamIndicator", "required": True},
            {"name": "SpectatorControls", "type": "WBP_MF_SpectatorControls", "required": True},
            {"name": "GameplayControls", "type": "WBP_MF_GameplayControls", "required": True},
            {"name": "TransitionOverlay", "type": "WBP_MF_TransitionOverlay", "required": False},
            {"name": "TeamSelectionPopup", "type": "WBP_MF_TeamSelectionPopup", "required": False},
            {"name": "PauseMenu", "type": "WBP_MF_PauseMenu", "required": False},
        ],
    },
}

# Creation order based on dependencies
CREATION_ORDER = [
    # Phase 1: Base Widgets
    "WBP_MF_MatchInfo",
    "WBP_MF_TeamIndicator",
    "WBP_MF_TransitionOverlay",
    "WBP_MF_VirtualJoystick",
    "WBP_MF_ActionButton",
    "WBP_MF_SprintButton",
    "WBP_MF_TeamPanel",
    "WBP_MF_QuickTeamPanel",
    "WBP_MF_ScorePopup",
    # Phase 2: Container Widgets
    "WBP_MF_SpectatorControls",
    "WBP_MF_GameplayControls",
    "WBP_MF_TeamSelectionPopup",
    "WBP_MF_PauseMenu",
    # Phase 3: Main HUD
    "WBP_MF_HUD",
]

# =============================================================================
# UTILITY FUNCTIONS
# =============================================================================

class Logger:
    """Centralized logging with levels and statistics."""
    
    def __init__(self):
        self.errors = []
        self.warnings = []
        self.info_messages = []
    
    def log(self, message: str, level: str = "INFO"):
        """Log message to UE5 Output Log."""
        prefix = f"[MF_WBP_Creator] [{level}]"
        full_message = f"{prefix} {message}"
        
        if level == "ERROR":
            unreal.log_error(full_message)
            self.errors.append(message)
        elif level == "WARNING":
            unreal.log_warning(full_message)
            self.warnings.append(message)
        else:
            unreal.log(full_message)
            self.info_messages.append(message)
    
    def error(self, message: str):
        self.log(message, "ERROR")
    
    def warning(self, message: str):
        self.log(message, "WARNING")
    
    def info(self, message: str):
        self.log(message, "INFO")
    
    def get_summary(self) -> Dict[str, int]:
        return {
            "errors": len(self.errors),
            "warnings": len(self.warnings),
            "info": len(self.info_messages)
        }

# Global logger instance
logger = Logger()

def log(message: str, level: str = "INFO"):
    """Legacy log function for backwards compatibility."""
    logger.log(message, level)

def safe_call(func, *args, default=None, error_msg: str = None, **kwargs):
    """
    Safely call a function with exception handling.
    
    Args:
        func: Function to call
        *args: Positional arguments
        default: Default value on failure
        error_msg: Custom error message
        **kwargs: Keyword arguments
    
    Returns:
        Function result or default value
    """
    try:
        return func(*args, **kwargs)
    except Exception as e:
        msg = error_msg or f"Error calling {func.__name__}"
        logger.error(f"{msg}: {e}")
        return default

def get_asset_tools():
    """Get the AssetTools instance safely."""
    try:
        return unreal.AssetToolsHelpers.get_asset_tools()
    except Exception as e:
        logger.error(f"Failed to get AssetTools: {e}")
        return None

def does_asset_exist(asset_path: str) -> bool:
    """Check if an asset exists at the given path."""
    try:
        return unreal.EditorAssetLibrary.does_asset_exist(asset_path)
    except Exception as e:
        logger.warning(f"Error checking asset existence for '{asset_path}': {e}")
        return False

def load_asset(asset_path: str):
    """Load an asset from the given path safely."""
    try:
        if not does_asset_exist(asset_path):
            logger.warning(f"Asset does not exist: {asset_path}")
            return None
        return unreal.EditorAssetLibrary.load_asset(asset_path)
    except Exception as e:
        logger.error(f"Failed to load asset '{asset_path}': {e}")
        return None

def save_asset(asset_path: str, force: bool = True) -> bool:
    """Save an asset to disk safely."""
    try:
        result = unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=not force)
        if result:
            logger.info(f"Saved asset: {asset_path}")
        return result
    except Exception as e:
        logger.error(f"Failed to save asset '{asset_path}': {e}")
        return False

def delete_asset(asset_path: str) -> bool:
    """Delete an asset safely."""
    try:
        if not does_asset_exist(asset_path):
            logger.warning(f"Cannot delete - asset does not exist: {asset_path}")
            return True  # Consider it success if already gone
        
        result = unreal.EditorAssetLibrary.delete_asset(asset_path)
        if result:
            logger.info(f"Deleted asset: {asset_path}")
        else:
            logger.error(f"Failed to delete asset: {asset_path}")
        return result
    except Exception as e:
        logger.error(f"Exception deleting asset '{asset_path}': {e}")
        return False

def get_parent_class(class_path: str):
    """Load a C++ class by path safely."""
    try:
        cls = unreal.load_class(None, class_path)
        if cls:
            return cls
        else:
            logger.error(f"Parent class not found: {class_path}")
            return None
    except Exception as e:
        logger.error(f"Failed to load parent class '{class_path}': {e}")
        return None

def ensure_directory_exists(package_path: str) -> bool:
    """Ensure the content directory exists."""
    try:
        # EditorAssetLibrary doesn't have make_directory, but create_asset will create it
        # We just verify the path format is valid
        if not package_path.startswith("/"):
            logger.error(f"Invalid package path (must start with /): {package_path}")
            return False
        return True
    except Exception as e:
        logger.error(f"Error verifying directory '{package_path}': {e}")
        return False

# =============================================================================
# WIDGET VALIDATION AND INSPECTION
# =============================================================================

def get_widget_blueprint_parent_class(widget_bp) -> Optional[str]:
    """
    Get the parent class path of a WidgetBlueprint.
    
    Args:
        widget_bp: WidgetBlueprint asset
    
    Returns:
        Parent class path or None
    """
    try:
        if widget_bp is None:
            return None
        
        # Try to get parent class from the generated class
        gen_class = widget_bp.get_editor_property("generated_class")
        if gen_class:
            parent = gen_class.get_editor_property("parent_class")
            if parent:
                return parent.get_path_name()
        
        # Alternative: check parent_class on the blueprint itself
        try:
            parent = widget_bp.get_editor_property("parent_class")
            if parent:
                return parent.get_path_name()
        except:
            pass
        
        return None
    except Exception as e:
        logger.warning(f"Could not determine parent class: {e}")
        return None

def check_parent_class_matches(widget_bp, expected_class_path: str) -> bool:
    """
    Check if a WidgetBlueprint has the expected parent class.
    
    Args:
        widget_bp: WidgetBlueprint to check
        expected_class_path: Expected parent class path
    
    Returns:
        True if parent matches, False otherwise
    """
    try:
        actual_parent = get_widget_blueprint_parent_class(widget_bp)
        if actual_parent is None:
            logger.warning("Could not determine actual parent class")
            return True  # Assume match if we can't check
        
        # Normalize paths for comparison
        expected_normalized = expected_class_path.replace("'", "").lower()
        actual_normalized = actual_parent.replace("'", "").lower()
        
        # Check if the expected class name is contained in the actual path
        # This handles both full paths and short names
        expected_class_name = expected_normalized.split(".")[-1]
        actual_class_name = actual_normalized.split(".")[-1]
        
        return expected_class_name == actual_class_name
    except Exception as e:
        logger.warning(f"Error checking parent class: {e}")
        return True  # Assume match on error

def get_widget_tree(widget_bp) -> Optional[Any]:
    """
    Get the WidgetTree from a WidgetBlueprint.
    
    Implements ordered fallbacks to handle UE5.2–5.4 engine API drift:
    1. Direct properties (most common UE versions)
    2. Method-based access (older UE versions)
    3. UE5.3+: widget tree on GeneratedClass (NOT CDO)
    4. UE5.2+: widget tree on GeneratedClass CDO
    
    Args:
        widget_bp: WidgetBlueprint asset
    
    Returns:
        WidgetTree or None
    """
    if widget_bp is None:
        return None

    gen_class = None  # Track for diagnostic logging

    # 1) Direct properties (most common UE versions)
    for prop in ("default_widget_tree", "designer_widget_tree", "widget_tree", "preview_widget_tree"):
        try:
            wt = widget_bp.get_editor_property(prop)
            if wt:
                return wt
        except Exception:
            pass

    # 2) Method-based access (older UE versions)
    for method_name in ("get_designer_widget_tree", "get_widget_tree", "get_preview_widget_tree"):
        try:
            if hasattr(widget_bp, method_name):
                wt = getattr(widget_bp, method_name)()
                if wt:
                    return wt
        except Exception:
            pass

    # 3) UE5.3+: widget tree on GeneratedClass (NOT CDO)
    try:
        gen_class = widget_bp.get_editor_property("generated_class")
        if gen_class and hasattr(gen_class, "get_editor_property"):
            wt = gen_class.get_editor_property("widget_tree")
            if wt:
                return wt
    except Exception:
        pass

    # 4) UE5.2+: widget tree on GeneratedClass CDO
    cdo = None
    try:
        if gen_class is None:
            gen_class = widget_bp.get_editor_property("generated_class")
        if gen_class and hasattr(gen_class, "get_default_object"):
            cdo = gen_class.get_default_object()
            if cdo:
                wt = cdo.get_editor_property("widget_tree")
                if wt:
                    return wt
    except Exception:
        pass

    # Log diagnostic info when widget tree not found
    try:
        asset_name = widget_bp.get_name() if hasattr(widget_bp, 'get_name') else str(widget_bp)
        logger.warning(f"Widget tree unavailable for {asset_name}:")
        logger.warning(f"  - generated_class: {gen_class is not None}")
        logger.warning(f"  - CDO: {cdo is not None}")
    except Exception:
        logger.warning("Could not get widget tree from blueprint (UE version API mismatch)")
    return None


def get_euw_widget_tree(euw_bp) -> Optional[Any]:
    """Get the WidgetTree for an Editor Utility Widget Blueprint.

    EUWs store their designer tree in designer_widget_tree property first.
    Implements EUW-specific fallbacks for UE5.2–5.4 API variations.
    
    Args:
        euw_bp: EditorUtilityWidgetBlueprint asset
    
    Returns:
        WidgetTree or None
    """
    if euw_bp is None:
        return None

    # EUW designer tree is stored in designer_widget_tree property first
    try:
        wt = euw_bp.get_editor_property("designer_widget_tree")
        if wt:
            return wt
    except Exception:
        pass

    # Try method fallback (some versions expose getter)
    try:
        if hasattr(euw_bp, "get_designer_widget_tree"):
            wt = euw_bp.get_designer_widget_tree()
            if wt:
                return wt
    except Exception:
        pass

    # Fall back to generic widget tree access
    return get_widget_tree(euw_bp)


def compile_blueprint(bp) -> bool:
    """Best-effort compile for blueprint assets (WidgetBlueprint/EUW)."""
    if bp is None:
        return False
    try:
        if hasattr(unreal, "BlueprintEditorLibrary") and hasattr(unreal.BlueprintEditorLibrary, "compile_blueprint"):
            unreal.BlueprintEditorLibrary.compile_blueprint(bp)
            return True
    except Exception as e:
        logger.warning(f"Blueprint compile failed for {bp}: {e}")
    return False


def save_loaded_asset(asset) -> bool:
    """Best-effort save for a loaded asset."""
    if asset is None:
        return False
    try:
        if hasattr(unreal, "EditorAssetLibrary") and hasattr(unreal.EditorAssetLibrary, "save_loaded_asset"):
            return bool(unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False))
    except Exception as e:
        logger.warning(f"save_loaded_asset failed for {asset}: {e}")
    return False


def _attach_child_widget(parent_panel, child_widget) -> bool:
    """Attach child_widget to parent_panel, preferring slot-safe APIs."""
    if not parent_panel or not child_widget:
        return False
    try:
        # CanvasPanel: prefer add_child_to_canvas to ensure CanvasPanelSlot.
        if isinstance(parent_panel, unreal.CanvasPanel) and hasattr(parent_panel, "add_child_to_canvas"):
            parent_panel.add_child_to_canvas(child_widget)
            return True
    except Exception:
        pass

    try:
        if hasattr(parent_panel, "add_child"):
            parent_panel.add_child(child_widget)
            return True
    except Exception:
        return False

    return False

def get_all_widgets_in_tree(widget_bp) -> List[Tuple[str, str]]:
    """
    Get all widgets in a WidgetBlueprint's widget tree.
    
    Args:
        widget_bp: WidgetBlueprint asset
    
    Returns:
        List of (widget_name, widget_type) tuples
    """
    widgets = []
    try:
        widget_tree = get_widget_tree(widget_bp)
        if widget_tree is None:
            return widgets
        
        # Get root widget
        root = widget_tree.get_editor_property("root_widget")
        if root:
            widgets.extend(_collect_widgets_recursive(root))
        
        # Also check all widgets property if available
        try:
            all_widgets = widget_tree.get_editor_property("all_widgets")
            if all_widgets:
                for w in all_widgets:
                    name = w.get_name() if hasattr(w, 'get_name') else str(w)
                    w_type = w.get_class().get_name() if hasattr(w, 'get_class') else "Unknown"
                    if (name, w_type) not in widgets:
                        widgets.append((name, w_type))
        except:
            pass
        
    except Exception as e:
        logger.warning(f"Error getting widgets from tree: {e}")
    
    return widgets

def _collect_widgets_recursive(widget, depth: int = 0) -> List[Tuple[str, str]]:
    """
    Recursively collect widgets from a widget hierarchy.
    
    Args:
        widget: Root widget to start from
        depth: Current recursion depth (safety limit)
    
    Returns:
        List of (widget_name, widget_type) tuples
    """
    widgets: List[Tuple[str, str]] = []
    max_depth = 50  # Safety limit
    
    if widget is None or depth > max_depth:
        return widgets
    
    try:
        # Get this widget's info
        name = widget.get_name() if hasattr(widget, "get_name") else str(widget)
        w_type = widget.get_class().get_name() if hasattr(widget, "get_class") else "Unknown"
        widgets.append((name, w_type))
        
        # Try to get children (for panel widgets)
        try:
            # Panel widgets expose children via get_children_count()/get_child_at()
            if hasattr(widget, "get_child_at") and hasattr(widget, "get_children_count"):
                try:
                    child_count = widget.get_children_count()
                except Exception:
                    child_count = 0
                
                for i in range(child_count):
                    try:
                        child = widget.get_child_at(i)
                    except Exception:
                        child = None
                    
                    if child:
                        widgets.extend(_collect_widgets_recursive(child, depth + 1))
            
            # Some widgets store content/children via a 'content' property or 'slots'
            if hasattr(widget, "get_editor_property"):
                # Single child/content
                try:
                    content = widget.get_editor_property("content")
                    if content:
                        widgets.extend(_collect_widgets_recursive(content, depth + 1))
                except Exception:
                    pass
                
                # Slots array (e.g., Overlay, VerticalBox)
                try:
                    slots = widget.get_editor_property("slots")
                    if slots:
                        for slot in slots:
                            try:
                                slot_content = slot.get_editor_property("content")
                                if slot_content:
                                    widgets.extend(_collect_widgets_recursive(slot_content, depth + 1))
                            except Exception:
                                continue
                except Exception:
                    pass
        
        except Exception:
            # Child traversal failures shouldn't abort the whole scan
            pass
            
    except Exception as e:
        logger.warning(f"Error collecting widgets at depth {depth}: {e}")
    
    return widgets

def widget_exists_in_tree(widget_bp, widget_name: str) -> bool:
    """
    Check if a widget with the given name exists in the widget tree.
    
    Args:
        widget_bp: WidgetBlueprint to check
        widget_name: Name of widget to find
    
    Returns:
        True if widget exists, False otherwise
    """
    widgets = get_all_widgets_in_tree(widget_bp)
    for name, _ in widgets:
        if name == widget_name:
            return True
    return False

def validate_widget_blueprint(
    widget_bp, 
    definition: Dict, 
    name: str
) -> Tuple[ValidationResult, List[str]]:
    """
    Validate an existing WidgetBlueprint against its definition.
    
    Args:
        widget_bp: WidgetBlueprint to validate
        definition: Expected widget definition
        name: Widget name for logging
    
    Returns:
        Tuple of (ValidationResult, list of issues)
    """
    issues = []

    # EUW is treated as a manual shell: UE Python cannot reliably inspect/validate
    # its designer widget tree, so we intentionally skip all validation.
    if name == EUW_NAME:
        return ValidationResult.VALID, issues
    
    if widget_bp is None:
        return ValidationResult.NOT_FOUND, ["Widget blueprint not found"]
    
    # Check parent class
    if not check_parent_class_matches(widget_bp, definition["parent_class"]):
        issues.append(f"Parent class mismatch. Expected: {definition['parent_class']}")
        return ValidationResult.WRONG_PARENT, issues
    
    # Check for required bindings
    existing_widgets = get_all_widgets_in_tree(widget_bp)
    existing_names = {w[0] for w in existing_widgets}
    
    required = definition.get("required_widgets", [])
    custom_required = [w for w in definition.get("custom_widgets", []) if w.get("required", False)]
    
    missing_bindings = []
    for w in required:
        if w["name"] not in existing_names:
            missing_bindings.append(w["name"])
    
    for w in custom_required:
        if w["name"] not in existing_names:
            missing_bindings.append(w["name"])
    
    if missing_bindings:
        issues.append(f"Missing required widgets: {', '.join(missing_bindings)}")
        return ValidationResult.MISSING_BINDINGS, issues
    
    return ValidationResult.VALID, issues

def get_action_for_existing_widget(
    widget_bp,
    definition: Dict,
    name: str,
    default_action: AssetAction = AssetAction.SKIP
) -> Tuple[AssetAction, ValidationResult, List[str]]:
    """
    Determine what action to take for an existing widget blueprint.
    
    Args:
        widget_bp: Existing WidgetBlueprint
        definition: Widget definition
        name: Widget name
        default_action: Default action if widget is valid
    
    Returns:
        Tuple of (action, validation_result, issues)
    """
    # EUW is treated as a manual shell: skip validation and treat as always valid.
    if name == EUW_NAME:
        return default_action, ValidationResult.VALID, []

    result, issues = validate_widget_blueprint(widget_bp, definition, name)
    
    if result == ValidationResult.VALID:
        return default_action, result, issues
    elif result == ValidationResult.WRONG_PARENT:
        # Wrong parent requires recreation
        return AssetAction.RECREATE, result, issues
    elif result == ValidationResult.MISSING_BINDINGS:
        # Missing bindings can potentially be updated (manual step needed)
        return AssetAction.UPDATE, result, issues
    elif result == ValidationResult.INVALID_STRUCTURE:
        # Invalid structure might need recreation
        return AssetAction.RECREATE, result, issues
    else:
        return default_action, result, issues

def validate_bindings() -> Dict[str, List[str]]:
    """
    Validate bindings for all widget blueprints defined in CREATION_ORDER.
    
    This does not create or delete any assets. It only:
    - Loads each widget (if it exists)
    - Validates parent class and required bindings
    - Collects issues per widget name
    
    Returns:
        Dict mapping widget name -> list of issues (empty dict if all valid)
    """
    validation_issues: Dict[str, List[str]] = {}
    
    for name in CREATION_ORDER:
        definition = WIDGET_DEFINITIONS.get(name)
        if not definition:
            logger.warning(f"No definition found for widget during validation: {name}")
            continue
        
        asset_path = f"{definition['path']}/{name}"
        
        if not does_asset_exist(asset_path):
            # If the asset isn't there at all, that's still a binding issue for validation
            validation_issues[name] = ["Widget blueprint not found at expected path"]
            continue
        
        widget_bp = load_asset(asset_path)
        result, issues = validate_widget_blueprint(widget_bp, definition, name)
        
        if result != ValidationResult.VALID:
            validation_issues[name] = issues
    
    return validation_issues

# =============================================================================
# WIDGET BLUEPRINT CREATION
# =============================================================================

def create_widget_blueprint(
    name: str, 
    definition: Dict,
    action: AssetAction = AssetAction.SKIP
) -> Tuple[Optional[Any], bool]:
    """
    Create or update a Widget Blueprint with the specified parent class.
    
    Handles:
    - Creating new widget blueprints
    - Skipping existing valid blueprints
    - Recreating blueprints with wrong parent class
    - Reporting missing bindings for manual update
    
    Args:
        name: Widget blueprint name (e.g., "WBP_MF_MatchInfo")
        definition: Widget definition dictionary
        action: Action to take for existing assets (SKIP, UPDATE, RECREATE)
    
    Returns:
        Tuple of (WidgetBlueprint or None, was_created: bool)
    """
    asset_path = f"{definition['path']}/{name}"
    was_created = False
    
    # Check if asset already exists
    if does_asset_exist(asset_path):
        existing_bp = load_asset(asset_path)
        
        if existing_bp is None:
            logger.error(f"Asset exists but failed to load: {asset_path}")
            return None, False
        
        # Validate existing asset
        determined_action, validation_result, issues = get_action_for_existing_widget(
            existing_bp, definition, name, action
        )
        
        # Log validation results
        if validation_result == ValidationResult.VALID:
            logger.info(f"Existing asset is valid: {name}")
        else:
            logger.warning(f"Existing asset has issues: {name}")
            for issue in issues:
                logger.warning(f"  - {issue}")
        
        # Handle based on determined action
        if determined_action == AssetAction.SKIP:
            logger.info(f"Skipping existing asset: {name}")
            return existing_bp, False
        
        elif determined_action == AssetAction.UPDATE:
            logger.warning(f"Asset has missing bindings: {name}")
            logger.warning("Attempting to automatically create missing widgets...")

            # Auto-fix: ensure required widgets exist in the widget tree
            try:
                setup_widget_structure(existing_bp, definition, name, auto_create_optional=True)
                save_asset(asset_path)
                logger.info(f"Auto-updated widget structure for: {name}")
            except Exception as e:
                logger.error(f"Auto-update of widget structure failed for {name}: {e}")

            # Even if auto-fix is partial, return the existing asset (not recreated)
            return existing_bp, False
        
        elif determined_action == AssetAction.RECREATE:
            logger.warning(f"Recreating asset due to issues: {name}")
            # Delete existing asset
            if not delete_asset(asset_path):
                logger.error(f"Failed to delete asset for recreation: {name}")
                return existing_bp, False
            # Continue to create new asset below
        
    # Verify path is valid
    if not ensure_directory_exists(definition["path"]):
        logger.error(f"Invalid path for {name}: {definition['path']}")
        return None, False

    # Create the widget blueprint
    parent_class = get_parent_class(definition["parent_class"])
    if not parent_class:
        logger.error(f"Parent class not found for {name}: {definition['parent_class']}")
        return None, False

    asset_tools = get_asset_tools()
    if not asset_tools:
        logger.error("AssetTools unavailable — cannot create WBP")
        return None, False

    factory = unreal.WidgetBlueprintFactory()
    try:
        factory.set_editor_property("parent_class", parent_class)
    except Exception as e:
        logger.error(f"Failed to set parent class on factory for {name}: {e}")
        return None, False

    try:
        widget_bp = asset_tools.create_asset(
            asset_name=name,
            package_path=definition["path"],
            asset_class=unreal.WidgetBlueprint,
            factory=factory,
        )
    except Exception as e:
        logger.error(f"Exception creating Widget Blueprint {name}: {e}")
        import traceback
        logger.error(traceback.format_exc())
        return None, False

    if not widget_bp:
        logger.error(f"Failed to create widget: {name}")
        return None, False

    was_created = True

    # Auto-construct widget tree structure (root + required/optional/custom widgets)
    try:
        setup_widget_structure(widget_bp, definition, name, auto_create_optional=True)
    except Exception as e:
        logger.error(f"Failed to setup widget structure for new WBP {name}: {e}")

    # Compile + save for reliability (designer tree + GeneratedClass)
    try:
        compile_blueprint(widget_bp)
    except Exception:
        pass
    try:
        save_loaded_asset(widget_bp)
    except Exception:
        pass

    return widget_bp, was_created


def _try_set_button_content(button_widget, content_widget) -> bool:
    """Best-effort to set a Button's content (engine-version tolerant)."""
    if not button_widget or not content_widget:
        return False
    try:
        if hasattr(button_widget, "set_content"):
            button_widget.set_content(content_widget)
            return True
    except Exception:
        pass
    try:
        if hasattr(button_widget, "set_editor_property"):
            button_widget.set_editor_property("content", content_widget)
            return True
    except Exception:
        pass
    return False


def setup_euw_widget_structure(widget_bp, definition: Dict, name: str) -> bool:
    """Create the recommended EUW layout for EUW_MF_WidgetCreator.

    This creates a usable, named widget tree matching EDITOR_UTILITY_WIDGET_GUIDE.md
    (and the C++ spec), so you can immediately wire button events in the graph.
    """
    if widget_bp is None:
        logger.error(f"Cannot setup EUW structure for {name}: widget blueprint is None")
        return False

    widget_tree = get_euw_widget_tree(widget_bp)
    if widget_tree is None:
        logger.error(f"Cannot setup EUW structure for {name}: widget tree not available")
        return False

    # Force a CanvasPanel root (safe for editor utility widgets)
    root_panel = get_or_create_root_panel(widget_tree, definition, name)
    if root_panel is None:
        logger.error(f"Cannot setup EUW structure for {name}: failed to get/create root panel")
        return False

    # If root isn't a CanvasPanel, wrap it
    try:
        if not isinstance(root_panel, unreal.CanvasPanel):
            existing_root = root_panel
            canvas_root = widget_tree.construct_widget(unreal.CanvasPanel, "RootCanvas")
            try:
                canvas_root.add_child(existing_root)
            except Exception:
                pass
            widget_tree.set_editor_property("root_widget", canvas_root)
            root_panel = canvas_root
    except Exception:
        pass

    def _get_or_create(widget_class, widget_name: str, make_variable: bool = True):
        existing = find_widget_in_tree(widget_tree, widget_name)
        if existing:
            if make_variable:
                ensure_widget_is_variable(existing)
            return existing

        try:
            w = widget_tree.construct_widget(widget_class, widget_name)
            if make_variable:
                ensure_widget_is_variable(w)
            return w
        except Exception as e:
            logger.error(f"EUW: failed to construct widget '{widget_name}': {e}")
            return None

    # MainContainer (VerticalBox) anchored to fill the canvas
    main_container = _get_or_create(unreal.VerticalBox, "MainContainer", make_variable=True)
    if not main_container:
        return False

    # Ensure attached to root
    try:
        # Try add_child_to_canvas first
        if isinstance(root_panel, unreal.CanvasPanel) and hasattr(root_panel, "add_child_to_canvas"):
            slot = root_panel.add_child_to_canvas(main_container)
        else:
            slot = root_panel.add_child(main_container)

        # Anchor fill
        try:
            if slot and hasattr(slot, "set_anchors"):
                slot.set_anchors(unreal.Anchors(unreal.Vector2D(0.0, 0.0), unreal.Vector2D(1.0, 1.0)))
        except Exception:
            pass
        try:
            if slot and hasattr(slot, "set_offsets"):
                slot.set_offsets(unreal.Margin(0.0, 0.0, 0.0, 0.0))
            else:
                # Fallback: zero position/size
                if slot and hasattr(slot, "set_position"):
                    slot.set_position(unreal.Vector2D(0.0, 0.0))
                if slot and hasattr(slot, "set_size"):
                    slot.set_size(unreal.Vector2D(0.0, 0.0))
        except Exception:
            pass
    except Exception as e:
        # If parenting fails, continue; widget still exists and can be arranged manually
        logger.warning(f"EUW: failed to attach MainContainer to root for {name}: {e}")

    # Verify MainContainer is parented; if not, force attach once more.
    try:
        if hasattr(main_container, "get_parent") and main_container.get_parent() is None:
            _attach_child_widget(root_panel, main_container)
    except Exception:
        pass

    def _vb_add(child_widget):
        try:
            if hasattr(main_container, "add_child"):
                return main_container.add_child(child_widget)
        except Exception:
            return None
        return None

    # Title
    title = _get_or_create(unreal.TextBlock, "Title", make_variable=True)
    if title:
        try:
            title.set_editor_property("text", unreal.Text.from_string("MF Widget Blueprint Creator"))
        except Exception:
            pass
        _vb_add(title)

    # StatusBar
    status_bar = _get_or_create(unreal.HorizontalBox, "StatusBar", make_variable=True)
    if status_bar:
        _vb_add(status_bar)

        def _hb_add(widget_obj):
            try:
                if hasattr(status_bar, "add_child"):
                    return status_bar.add_child(widget_obj)
            except Exception:
                return None
            return None

        # Labels + Counts
        existing_label = _get_or_create(unreal.TextBlock, "ExistingLabel", make_variable=False)
        if existing_label:
            try:
                existing_label.set_editor_property("text", unreal.Text.from_string("Existing:"))
            except Exception:
                pass
            _hb_add(existing_label)

        existing_count = _get_or_create(unreal.TextBlock, "ExistingCount", make_variable=True)
        if existing_count:
            try:
                existing_count.set_editor_property("text", unreal.Text.from_string("0"))
            except Exception:
                pass
            _hb_add(existing_count)

        missing_label = _get_or_create(unreal.TextBlock, "MissingLabel", make_variable=False)
        if missing_label:
            try:
                missing_label.set_editor_property("text", unreal.Text.from_string("Missing:"))
            except Exception:
                pass
            _hb_add(missing_label)

        missing_count = _get_or_create(unreal.TextBlock, "MissingCount", make_variable=True)
        if missing_count:
            try:
                missing_count.set_editor_property("text", unreal.Text.from_string("0"))
            except Exception:
                pass
            _hb_add(missing_count)

        issues_label = _get_or_create(unreal.TextBlock, "IssuesLabel", make_variable=False)
        if issues_label:
            try:
                issues_label.set_editor_property("text", unreal.Text.from_string("Issues:"))
            except Exception:
                pass
            _hb_add(issues_label)

        issues_count = _get_or_create(unreal.TextBlock, "IssuesCount", make_variable=True)
        if issues_count:
            try:
                issues_count.set_editor_property("text", unreal.Text.from_string("0"))
            except Exception:
                pass
            _hb_add(issues_count)

    # Spacer
    spacer_top = _get_or_create(unreal.Spacer, "Spacer_Top", make_variable=False)
    if spacer_top:
        _vb_add(spacer_top)

    def _make_button(button_name: str, button_label: str):
        btn = _get_or_create(unreal.Button, button_name, make_variable=True)
        if not btn:
            return None
        _vb_add(btn)

        label = _get_or_create(unreal.TextBlock, f"{button_name}Text", make_variable=False)
        if label:
            try:
                label.set_editor_property("text", unreal.Text.from_string(button_label))
            except Exception:
                pass
            _try_set_button_content(btn, label)
        return btn

    _make_button("PreviewButton", "🔍 Preview Changes (Dry Run)")
    _make_button("CreateButton", "➕ Create Missing Widgets")
    _make_button("ValidateButton", "✓ Validate All Widgets")
    _make_button("ForceRecreateButton", "🔄 Force Recreate All (DANGER!)")

    spacer_bottom = _get_or_create(unreal.Spacer, "Spacer_Bottom", make_variable=False)
    if spacer_bottom:
        _vb_add(spacer_bottom)

    output_label = _get_or_create(unreal.TextBlock, "OutputLabel", make_variable=False)
    if output_label:
        try:
            output_label.set_editor_property("text", unreal.Text.from_string("Output Log:"))
        except Exception:
            pass
        _vb_add(output_label)

    output_log = _get_or_create(unreal.MultiLineEditableTextBox, "OutputLog", make_variable=True)
    if output_log:
        try:
            output_log.set_editor_property("is_read_only", True)
        except Exception:
            pass
        _vb_add(output_log)

    logger.info(f"Setup EUW widget structure for {name}")
    return True


def create_editor_utility_widget_blueprint(
    name: str,
    definition: Dict,
    action: AssetAction = AssetAction.SKIP,
) -> Tuple[Optional[Any], bool]:
    """Create or update an Editor Utility Widget Blueprint."""
    asset_path = f"{definition['path']}/{name}"
    was_created = False

    if does_asset_exist(asset_path):
        existing_bp = load_asset(asset_path)
        if existing_bp is None:
            logger.error(f"EUW asset exists but failed to load: {asset_path}")
            return None, False

        # EUW is treated as a manual shell:
        # - Do NOT validate
        # - Do NOT attempt to build/inspect widget trees
        # - Do NOT auto-update structure
        if action != AssetAction.RECREATE:
            return existing_bp, False

        if not delete_asset(asset_path):
            logger.error(f"Failed to delete EUW asset for recreation: {name}")
            return existing_bp, False

    if not ensure_directory_exists(definition["path"]):
        logger.error(f"Invalid EUW path for {name}: {definition['path']}")
        return None, False

    parent_class = get_parent_class(definition["parent_class"])
    if not parent_class:
        logger.error(f"Cannot create EUW {name}: parent class not found")
        logger.error(f"  Expected: {definition['parent_class']}")
        logger.error("  Ensure P_MiniFootballEditor module is compiled and loaded")
        return None, False

    asset_tools = get_asset_tools()
    if not asset_tools:
        logger.error("Failed to get AssetTools - cannot create EUW asset")
        return None, False

    factory = unreal.EditorUtilityWidgetBlueprintFactory()
    try:
        factory.set_editor_property("parent_class", parent_class)
    except Exception as e:
        logger.error(f"Failed to set parent class on EUW factory: {e}")
        return None, False

    try:
        widget_bp = asset_tools.create_asset(
            asset_name=name,
            package_path=definition["path"],
            asset_class=unreal.EditorUtilityWidgetBlueprint,
            factory=factory,
        )
        if not widget_bp:
            logger.error(f"Failed to create EUW: {name}")
            return None, False

        was_created = True
        logger.info(f"Created Editor Utility Widget Blueprint: {name}")

        # NOTE: EUW designer trees are not reliably scriptable via UE Python.
        # We intentionally create only the asset shell and let designers author
        # the widget content manually.

        try:
            compile_blueprint(widget_bp)
        except Exception:
            pass
        try:
            save_loaded_asset(widget_bp)
        except Exception:
            pass

        return widget_bp, was_created
    except Exception as e:
        logger.error(f"Exception creating EUW {name}: {e}")
        import traceback
        logger.error(traceback.format_exc())
        return None, False

def get_widget_binding_info(definition):
    """Get formatted binding information for a widget definition."""
    info = []
    
    required = definition.get("required_widgets", [])
    optional = definition.get("optional_widgets", [])
    custom = definition.get("custom_widgets", [])
    
    for w in required:
        info.append(f"  [REQUIRED] {w['name']} : {w['type']}")
    
    for w in optional:
        info.append(f"  [Optional] {w['name']} : {w['type']}")
    
    for w in custom:
        req_str = "[REQUIRED]" if w.get("required", False) else "[Optional]"
        info.append(f"  {req_str} {w['name']} : {w['type']} (Custom WBP)")
    
    return info

# =============================================================================
# WIDGET TREE CONSTRUCTION & AUTO-BINDING
# =============================================================================

def find_widget_in_tree(widget_tree, widget_name: str):
    """
    Find a widget object by name in a WidgetTree.

    Args:
        widget_tree: WidgetTree to search
        widget_name: Name of the widget

    Returns:
        Widget object or None
    """
    if widget_tree is None:
        return None

    # Prefer attached widgets: walk from root recursively first
    try:
        root = widget_tree.get_editor_property("root_widget")
    except Exception:
        root = None

    if root:
        def _find_recursive(widget, depth: int = 0):
            if widget is None or depth > 50:
                return None
            try:
                if hasattr(widget, "get_name") and widget.get_name() == widget_name:
                    return widget

                # Children via panel interface
                if hasattr(widget, "get_child_at") and hasattr(widget, "get_children_count"):
                    try:
                        count = widget.get_children_count()
                    except Exception:
                        count = 0
                    for i in range(count):
                        try:
                            child = widget.get_child_at(i)
                        except Exception:
                            child = None
                        if child:
                            found = _find_recursive(child, depth + 1)
                            if found:
                                return found

                # Children via 'content' or 'slots'
                if hasattr(widget, "get_editor_property"):
                    try:
                        content = widget.get_editor_property("content")
                        if content:
                            found = _find_recursive(content, depth + 1)
                            if found:
                                return found
                    except Exception:
                        pass

                    try:
                        slots = widget.get_editor_property("slots")
                        if slots:
                            for slot in slots:
                                try:
                                    slot_content = slot.get_editor_property("content")
                                except Exception:
                                    slot_content = None
                                if slot_content:
                                    found = _find_recursive(slot_content, depth + 1)
                                    if found:
                                        return found
                    except Exception:
                        pass
            except Exception:
                return None
            return None

        attached = _find_recursive(root)
        if attached:
            return attached

    # Fallback: use 'all_widgets' (may include unparented widgets)
    try:
        all_widgets = widget_tree.get_editor_property("all_widgets")
        if all_widgets:
            for w in all_widgets:
                try:
                    if hasattr(w, "get_name") and w.get_name() == widget_name:
                        return w
                except Exception:
                    continue
    except Exception:
        pass

    return None


def ensure_widget_is_variable(widget):
    """
    Ensure 'Is Variable' is enabled so BindWidget can work.

    Args:
        widget: UWidget instance
    """
    try:
        if hasattr(widget, "set_editor_property"):
            widget.set_editor_property("is_variable", True)
    except Exception:
        # Non-fatal, just continue
        pass


def get_or_create_root_panel(widget_tree, definition: Dict, name: str):
    """
    Ensure the widget tree has a valid root panel widget.

    If there is no root, create a CanvasPanel and make it root.
    If root is not a PanelWidget, wrap it in a CanvasPanel.

    Returns:
        Root panel widget (PanelWidget subclass)
    """
    if widget_tree is None:
        return None

    try:
        root = widget_tree.get_editor_property("root_widget")
    except Exception:
        root = None

    # If there is already a panel root, use it
    try:
        if root and isinstance(root, unreal.PanelWidget):
            return root
    except Exception:
        pass

    # Create a new CanvasPanel as root
    try:
        root_name = "RootCanvas"
        # If definition explicitly has RootCanvas, use that name
        for w in definition.get("optional_widgets", []):
            if w["name"] == "RootCanvas":
                root_name = w["name"]
                break

        canvas_class = unreal.CanvasPanel
        canvas_root = widget_tree.construct_widget(canvas_class, root_name)

        # If there was an existing root, try to attach it as a child
        try:
            if root and hasattr(canvas_root, "add_child"):
                canvas_root.add_child(root)
        except Exception:
            pass

        widget_tree.set_editor_property("root_widget", canvas_root)
        ensure_widget_is_variable(canvas_root)
        return canvas_root
    except Exception as e:
        logger.warning(f"Failed to create root panel for {name}: {e}")
        return root  # Fall back to whatever root we had


def get_user_widget_class(widget_name: str):
    """
    Resolve a custom UUserWidget class from a WBP name.

    Uses WIDGET_DEFINITIONS first, falls back to PLUGIN_COMPONENTS_PATH.

    Args:
        widget_name: e.g., 'WBP_MF_QuickTeamPanel'

    Returns:
        UClass for the generated widget, or None
    """
    # Try to find definition for the referenced widget
    defn = WIDGET_DEFINITIONS.get(widget_name)
    if defn:
        asset_path = f"{defn['path']}/{widget_name}"
    else:
        # Fallback: assume it's in the components path
        asset_path = f"{PLUGIN_COMPONENTS_PATH}/{widget_name}"

    bp = load_asset(asset_path)
    if not bp:
        logger.warning(f"Could not load custom widget blueprint for '{widget_name}' at '{asset_path}'")
        return None

    try:
        gen_class = bp.get_editor_property("generated_class")
        if gen_class:
            return gen_class
    except Exception as e:
        logger.warning(f"Failed to get generated_class from '{widget_name}': {e}")

    # If the WBP is newly created, GeneratedClass may be None until compiled.
    try:
        if compile_blueprint(bp):
            try:
                gen_class = bp.get_editor_property("generated_class")
                if gen_class:
                    return gen_class
            except Exception:
                pass
    except Exception:
        pass

    return None


def setup_widget_structure(widget_bp, definition: Dict, name: str, auto_create_optional: bool = True) -> bool:
    """
    Ensure the widget blueprint's widget tree contains all required bindings.

    This function is ROBUST and non-destructive:
    - If WBP is new: creates root panel and all required/optional/custom widgets
    - If WBP exists: reuses existing widgets when found; only creates missing ones
    - It does NOT delete or reparent existing widgets that don't match our expectations
    - It works even if the hierarchy is very different from the "ideal" layout

    Args:
        widget_bp: WidgetBlueprint to modify
        definition: Definition from WIDGET_DEFINITIONS
        name: Widget name (for logging)
        auto_create_optional: If True, also create optional widgets

    Returns:
        True if successful, False on major failure
    """
    if widget_bp is None:
        logger.error(f"Cannot setup structure for {name}: widget blueprint is None")
        return False

    widget_tree = get_widget_tree(widget_bp)
    if widget_tree is None:
        logger.error(f"Cannot setup structure for {name}: widget tree not available")
        return False

    # Ensure a root panel exists
    root_panel = get_or_create_root_panel(widget_tree, definition, name)
    if root_panel is None:
        logger.error(f"Cannot setup structure for {name}: failed to get or create root panel")
        return False

    created_count = 0

    def _create_or_get_widget(widget_desc: Dict, is_custom: bool = False):
        nonlocal created_count
        w_name = widget_desc["name"]

        # If widget already exists anywhere in tree, just ensure it's variable and return
        existing = find_widget_in_tree(widget_tree, w_name)
        if existing:
            # If it exists but isn't parented (common with all_widgets), attach it.
            try:
                if hasattr(existing, "get_parent") and existing.get_parent() is None:
                    _attach_child_widget(root_panel, existing)
            except Exception:
                pass
            ensure_widget_is_variable(existing)
            return existing

        # Create a new widget
        try:
            if is_custom:
                # Custom nested UUserWidget
                child_type_name = widget_desc["type"]
                child_class = get_user_widget_class(child_type_name)
                if not child_class:
                    logger.warning(f"Cannot create custom widget '{w_name}' in {name}: class not found for '{child_type_name}'")
                    return None
                widget = widget_tree.construct_widget(child_class, w_name)
            else:
                # Standard widget (TextBlock, Button, etc.)
                type_name = widget_desc["type"]
                widget_class = WIDGET_TYPE_MAP.get(type_name)
                if not widget_class:
                    logger.warning(f"Unknown widget type '{type_name}' for '{w_name}' in {name}")
                    return None
                widget = widget_tree.construct_widget(widget_class, w_name)

            # Attach to root panel if possible
            attached = _attach_child_widget(root_panel, widget)
            if not attached:
                try:
                    if hasattr(widget, "get_parent") and widget.get_parent() is None:
                        logger.warning(f"Widget '{w_name}' created in {name} but could not be attached to root")
                except Exception:
                    pass

            # Set IsVariable
            ensure_widget_is_variable(widget)

            # Apply simple default text if provided
            if "text" in widget_desc:
                text_value = widget_desc["text"]
                try:
                    if hasattr(widget, "set_editor_property"):
                        try:
                            # Primary: UTextBlock property expects FText
                            widget.set_editor_property("text", unreal.Text.from_string(text_value))
                        except Exception:
                            # Fallback: some widgets allow raw string assignment
                            widget.set_editor_property("text", text_value)
                except Exception:
                    logger.warning(
                        f"Could not set text on widget '{w_name}' to '{text_value}' "
                        f"(property may not exist on this widget type)"
                    )

            created_count += 1
            return widget
        except Exception as e:
            logger.error(f"Failed to create widget '{w_name}' in {name}: {e}")
            return None

    # 1) Create all REQUIRED standard widgets
    for w in definition.get("required_widgets", []):
        _create_or_get_widget(w, is_custom=False)

    # 2) Create REQUIRED custom widgets (nested WBPs)
    for w in definition.get("custom_widgets", []):
        if w.get("required", False):
            _create_or_get_widget(w, is_custom=True)

    # 3) Optionally create OPTIONAL standard/custom widgets
    if auto_create_optional:
        for w in definition.get("optional_widgets", []):
            _create_or_get_widget(w, is_custom=False)
        for w in definition.get("custom_widgets", []):
            if not w.get("required", False):
                _create_or_get_widget(w, is_custom=True)

    if created_count > 0:
        logger.info(f"Setup widget structure for {name}: created {created_count} widgets")
    else:
        logger.info(f"Setup widget structure for {name}: no new widgets needed (all present)")

    return True

# =============================================================================
# ADVANCED FEATURES: Layout, Theme, Animations, Editor Integration
# =============================================================================

def _get_canvas_slot(widget):
    """Return the CanvasPanelSlot for a widget if it exists, else None.

    Safely handles cases where widgets may not yet have a slot at design time.
    """
    try:
        # Many UWidget instances expose a 'slot' property usable from Python
        slot = None
        if hasattr(widget, 'get_editor_property'):
            try:
                slot = widget.get_editor_property('slot')
            except Exception:
                slot = None

        # Fallback: some widgets expose get_slot() or Slot
        if not slot and hasattr(widget, 'get_slot'):
            try:
                slot = widget.get_slot()
            except Exception:
                slot = None

        # Verify it's a CanvasPanelSlot (or castable)
        if slot and isinstance(slot, unreal.CanvasPanelSlot):
            return slot
    except Exception:
        pass

    return None


def apply_layout_to_widget_blueprint(widget_bp, definition: Dict, name: str, layout_hints: Optional[Dict] = None) -> bool:
    """
    Apply automatic layout rules (anchors, position, size, alignment) to a WidgetBlueprint.

    This is conservative and non-destructive. Rules are applied only when a CanvasPanel
    slot is present (the safest design-time slot for absolute placement). If a widget
    does not have a CanvasPanel slot, we skip it and log a hint for manual placement.

    Args:
        widget_bp: WidgetBlueprint to modify
        definition: The widget definition from WIDGET_DEFINITIONS
        name: Widget name (for logging)
        layout_hints: Optional per-widget overrides. If None, sensible defaults are used.

    Returns:
        True on success (or partial success), False only on major failure.
    """
    if widget_bp is None:
        logger.error(f"apply_layout: widget_bp is None for {name}")
        return False

    widget_tree = get_widget_tree(widget_bp)
    if widget_tree is None:
        logger.warning(f"apply_layout: widget_tree not available for {name}")
        return False

    # Default heuristics for common widgets (normalized anchors and offset/size)
    default_hints = {
        "WBP_MF_MatchInfo": {"anchor_min": (0.5, 0.0), "anchor_max": (0.5, 0.0), "position": (-400, 10), "size": (800, 80), "alignment": (0.5, 0.0)},
        "WBP_MF_TeamIndicator": {"anchor_min": (0.0, 0.0), "anchor_max": (0.0, 0.0), "position": (10, 10), "size": (180, 40), "alignment": (0.0, 0.0)},
        "WBP_MF_HUD": {"anchor_min": (0.0, 0.0), "anchor_max": (1.0, 1.0), "position": (0, 0), "size": (0, 0), "alignment": (0.0, 0.0)},
        "WBP_MF_GameplayControls": {"anchor_min": (0.5, 1.0), "anchor_max": (0.5, 1.0), "position": (-400, -220), "size": (800, 220), "alignment": (0.5, 1.0)},
    }

    hints = default_hints.get(name, {})
    if layout_hints:
        # merge overrides
        hints.update(layout_hints)

    # Walk required + optional + custom and apply layout where possible
    widgets_to_process = []
    for group in ("required_widgets", "optional_widgets"):
        for w in definition.get(group, []):
            widgets_to_process.append(w)
    for w in definition.get("custom_widgets", []):
        widgets_to_process.append(w)

    applied = 0
    skipped = 0

    for w in widgets_to_process:
        wname = w["name"]
        widget_obj = find_widget_in_tree(widget_tree, wname)
        if not widget_obj:
            logger.info(f"apply_layout: widget '{wname}' not present in {name} (skip layout)")
            skipped += 1
            continue

        slot = _get_canvas_slot(widget_obj)
        if not slot:
            logger.info(f"apply_layout: widget '{wname}' in {name} has no CanvasPanelSlot (manual placement advised)")
            skipped += 1
            continue

        # Compute placement
        # If definition contains an explicit design_size for the WBP, use proportional placement
        design_size = definition.get("design_size") or WIDGET_SIZES.get(name)
        try:
            # Anchors
            a_min = hints.get("anchor_min", (0.0, 0.0))
            a_max = hints.get("anchor_max", a_min)
            anchors = unreal.Anchors(unreal.Vector2D(a_min[0], a_min[1]), unreal.Vector2D(a_max[0], a_max[1]))
            try:
                slot.set_anchors(anchors)
            except Exception:
                # Some engine versions require set_layout or set_alignment instead
                try:
                    slot.set_layout(anchors)
                except Exception:
                    # Fallback: try writing anchors via layout_data
                    try:
                        layout_data = None
                        if hasattr(slot, "get_editor_property"):
                            layout_data = slot.get_editor_property("layout_data")
                        elif hasattr(slot, "layout_data"):
                            layout_data = slot.layout_data
                        if layout_data and hasattr(layout_data, "anchors"):
                            layout_data.anchors = anchors
                            if hasattr(slot, "set_editor_property"):
                                slot.set_editor_property("layout_data", layout_data)
                    except Exception:
                        pass

            # Size & position
            pos = hints.get("position", (0, 0))
            size = hints.get("size", design_size or (0, 0))
            # Set size/position only if non-zero
            if hasattr(slot, 'set_position') and pos is not None:
                try:
                    slot.set_position(unreal.Vector2D(pos[0], pos[1]))
                except Exception:
                    pass
            if hasattr(slot, 'set_size') and size is not None and size != (0, 0):
                try:
                    slot.set_size(unreal.Vector2D(size[0], size[1]))
                except Exception:
                    pass

            # Alignment
            align = hints.get("alignment")
            if align and hasattr(slot, 'set_alignment'):
                try:
                    slot.set_alignment(unreal.Vector2D(align[0], align[1]))
                except Exception:
                    pass

            applied += 1
        except Exception as e:
            logger.warning(f"apply_layout: failed to apply layout to {wname} in {name}: {e}")
            skipped += 1

    logger.info(f"apply_layout: applied layout to {applied} widgets, skipped {skipped}")
    return True


def apply_theme_to_widget_blueprint(widget_bp, theme_name: str = "default") -> bool:
    """
    Apply colors & theme to a WidgetBlueprint using the COLORS map.

    This is conservative: it attempts to set common color properties (TextBlock.color_and_opacity,
    Image.brush.tint_color, Border.brush.color) and falls back safely when a property doesn't exist.

    Args:
        widget_bp: WidgetBlueprint to modify
        theme_name: Reserved for future multi-theme support (currently ignored)

    Returns:
        True on success (partial successes are allowed)
    """
    if widget_bp is None:
        logger.error("apply_theme: widget_bp is None")
        return False

    widget_tree = get_widget_tree(widget_bp)
    if widget_tree is None:
        logger.warning("apply_theme: widget_tree not available")
        return False

    all_widgets = get_all_widgets_in_tree(widget_bp)
    applied = 0
    skipped = 0

    for w_name, wtype in all_widgets:
        try:
            w_obj = find_widget_in_tree(widget_tree, w_name)
            if not w_obj:
                skipped += 1
                continue

            # Text widgets: set color_and_opacity if present
            if "TextBlock" in wtype:
                try:
                    if hasattr(w_obj, 'set_editor_property'):
                        # Use TEXT_WHITE as default text color
                        w_obj.set_editor_property('color_and_opacity', COLORS['TEXT_WHITE'])
                        applied += 1
                        continue
                except Exception:
                    pass

            # Image widgets: set brush tint or tint_color_and_opacity
            if "Image" in wtype:
                try:
                    if hasattr(w_obj, 'set_editor_property'):
                        # Some UImage instances accept 'brush_tint' or 'tint_color_and_opacity'
                        try:
                            w_obj.set_editor_property('brush_tint', COLORS['TEXT_WHITE'])
                            applied += 1
                            continue
                        except Exception:
                            pass

                        try:
                            w_obj.set_editor_property('tint_color_and_opacity', COLORS['TEXT_WHITE'])
                            applied += 1
                            continue
                        except Exception:
                            pass
                except Exception:
                    pass

            # Border widgets: set brush color
            if "Border" in wtype:
                try:
                    if hasattr(w_obj, 'set_editor_property'):
                        w_obj.set_editor_property('brush_color', COLORS.get('PANEL_BG', COLORS['BACKGROUND']))
                        applied += 1
                        continue
                except Exception:
                    pass

            # Buttons: try to set style colors conservatively
            if "Button" in wtype:
                try:
                    if hasattr(w_obj, 'set_editor_property'):
                        # Buttons are complex; try 'background_color' then fallback to tint
                        try:
                            w_obj.set_editor_property('background_color', COLORS.get('BUTTON_NORMAL'))
                            applied += 1
                            continue
                        except Exception:
                            pass

                        try:
                            w_obj.set_editor_property('color_and_opacity', COLORS.get('BUTTON_NORMAL'))
                            applied += 1
                            continue
                        except Exception:
                            pass
                except Exception:
                    pass

            skipped += 1
        except Exception:
            skipped += 1
            continue

    logger.info(f"apply_theme: applied theme to {applied} widgets, skipped {skipped}")
    return True


def create_simple_fade_animation(widget_bp, target_widget_name: str, duration: float = 0.5) -> Optional[str]:
    """
    Attempt to create a simple fade-in WidgetAnimation for a named widget inside the widget blueprint.

    NOTE: Creating full keyframed UMG animations programmatically is possible but
    can be complex and engine-version-dependent. This helper does a best-effort
    approach: it creates a WidgetAnimation object and registers it on the blueprint.
    If full keyframes cannot be created via Python in your engine version, the
    function will still add the animation asset placeholder so you can keyframe it in editor.

    Returns:
        Name of the created animation, or None if not created.
    """
    if widget_bp is None:
        logger.warning("create_simple_fade_animation: widget_bp is None")
        return None

    try:
        anim_name = f"AutoFade_{target_widget_name}"
        # Best-effort: construct WidgetAnimation instance and attach to BP
        anim = None
        try:
            if hasattr(unreal, "new_object"):
                anim = unreal.new_object(unreal.WidgetAnimation, outer=widget_bp, name=anim_name)
            elif hasattr(unreal, "NewObject"):
                anim = unreal.NewObject(unreal.WidgetAnimation, outer=widget_bp, name=anim_name)
        except Exception:
            anim = None

        if not anim:
            # Fallback for engine bindings that still expose a direct constructor
            anim = unreal.WidgetAnimation(widget_bp, anim_name)

        # Try to append to widget blueprint animations list/property
        try:
            current = widget_bp.get_editor_property('animations') or []
            current.append(anim)
            widget_bp.set_editor_property('animations', current)
            logger.info(f"create_simple_fade_animation: created animation placeholder '{anim_name}' on {widget_bp.get_name()}")
            return anim_name
        except Exception as e:
            logger.warning(f"create_simple_fade_animation: could not attach animation to blueprint: {e}")
            return None

    except Exception as e:
        logger.warning(f"create_simple_fade_animation: failed to create animation object: {e}")
        return None


def register_editor_menu_entry():
    """
    Create a small editor menu entry under 'Window > MF Tools' that runs the main script.

    This uses the ToolMenus API and registers a submenu + a Python command which
    executes the `main()` function when clicked. This provides a lightweight Editor
    UI to run your WBP creator without writing an Editor plugin.
    """
    try:
        menus = unreal.ToolMenus.get()
        menu_paths = [
            "LevelEditor.MainMenu",
            "MainFrame.MainMenu",
            "LevelEditor.LevelEditorMainMenu",
        ]

        main_menu = None
        for p in menu_paths:
            try:
                main_menu = menus.find_menu(p)
            except Exception:
                main_menu = None
            if main_menu:
                break

        if not main_menu:
            logger.warning("ToolMenus: could not find a main menu (LevelEditor/MainFrame variants)")
            return False

        # Create/extend a 'MF Tools' menu under the main menu
        script_menu = main_menu.add_sub_menu(main_menu.get_name(), "MFTools", "MF Tools", "MiniFootball Tools")

        # Create entry
        entry = unreal.ToolMenuEntry(
            name="MF_WBP_Creator_Run",
            type=unreal.MultiBlockType.MENU_ENTRY,
            insert_position=unreal.ToolMenuInsert("Window", unreal.ToolMenuInsertType.AFTER)
        )
        entry.set_label("Run MF WBP Creator")
        entry.set_tool_tip("Creates/validates MiniFootball Widget Blueprints (Python)")

        # Command: run the main() function of this script
        # Use a safe import path assuming this file is run from Scripts path or content
        py_command = "import MF_WidgetBlueprintCreator; MF_WidgetBlueprintCreator.main(dry_run=False, show_guide=True)"
        entry.set_string_command(unreal.ToolMenuStringCommandType.PYTHON, py_command)

        script_menu.add_menu_entry("Run", entry)
        menus.refresh_all_widgets()
        logger.info("Registered 'MF Tools' menu entry (Window > MF Tools > Run MF WBP Creator)")
        return True
    except Exception as e:
        logger.warning(f"register_editor_menu_entry: failed to register menu: {e}")
        return False


def _apply_theme_and_layout_pass(results: Dict[str, Any]):
    """
    Apply Theme & Layout Rules to widgets based on configuration.
    
    This is called after widget creation/validation but before the binding guide.
    
    Args:
        results: Results dictionary from create_all_widgets
    """
    try:
        for widget_name in CREATION_ORDER:
            definition = WIDGET_DEFINITIONS.get(widget_name)
            if not definition:
                continue
            asset_path = f"{definition['path']}/{widget_name}"
            if not does_asset_exist(asset_path):
                continue

            wbp = load_asset(asset_path)
            if not wbp:
                continue
                
            was_created = widget_name in results.get("created", []) or widget_name in results.get("recreated", [])

            # --- Theme ---
            if was_created and APPLY_THEME_TO_NEW:
                try:
                    apply_theme_to_widget_blueprint(wbp)
                except Exception:
                    pass
            elif (not was_created) and APPLY_THEME_TO_EXISTING:
                try:
                    apply_theme_to_widget_blueprint(wbp)
                except Exception:
                    pass

            # --- Layout ---
            if was_created and APPLY_LAYOUT_TO_NEW:
                try:
                    apply_layout_to_widget_blueprint(wbp, definition, widget_name)
                except Exception:
                    pass
            elif (not was_created) and APPLY_LAYOUT_TO_EXISTING:
                try:
                    apply_layout_to_widget_blueprint(wbp, definition, widget_name)
                except Exception:
                    pass
                    
            # Save if we applied anything
            if was_created or APPLY_THEME_TO_EXISTING or APPLY_LAYOUT_TO_EXISTING:
                try:
                    save_asset(asset_path)
                except Exception:
                    pass
                    
    except Exception as e:
        logger.warning(f"_apply_theme_and_layout_pass: error during pass: {e}")

# =============================================================================
# MAIN EXECUTION
# =============================================================================

def create_all_widgets(
    dry_run: bool = False,
    force_recreate: bool = False,
    validate_existing: bool = True
) -> Dict[str, Any]:
    """
    Create all widget blueprints in the correct order with robust handling.
    
    Handles:
    - Creating new widget blueprints that don't exist
    - Validating existing blueprints
    - Optionally recreating invalid blueprints
    - Detailed reporting of actions taken
    
    Args:
        dry_run: If True, only report what would be created without creating
        force_recreate: If True, recreate all widgets even if they exist
        validate_existing: If True, validate existing widgets and report issues
    
    Returns:
        Dictionary with creation results and statistics
    """
    results = {
        "created": [],
        "skipped": [],
        "updated": [],
        "recreated": [],
        "failed": [],
        "validation_issues": {},
        "total": len(CREATION_ORDER)
    }
    
    logger.info("=" * 60)
    logger.info("MF Widget Blueprint Creator - Starting")
    logger.info("=" * 60)
    
    if dry_run:
        logger.info("DRY RUN MODE - No assets will be modified")
        logger.info("-" * 60)
    
    if force_recreate:
        logger.warning("FORCE RECREATE MODE - All existing widgets will be recreated")
        logger.info("-" * 60)
    
    for name in CREATION_ORDER:
        definition = WIDGET_DEFINITIONS.get(name)
        if not definition:
            logger.error(f"No definition found for: {name}")
            results["failed"].append(name)
            continue
        
        asset_path = f"{definition['path']}/{name}"
        
        # Report widget info
        logger.info(f"\n{'='*40}")
        logger.info(f"Widget: {name}")
        logger.info(f"Parent: {definition['parent_class']}")
        logger.info(f"Path: {asset_path}")
        logger.info(f"Fill Screen: {definition.get('fill_screen', False)}")
        
        if not definition.get("fill_screen", False):
            size = definition.get("design_size", (400, 300))
            logger.info(f"Design Size: {size[0]} x {size[1]}")
        
        # Report bindings
        if name != EUW_NAME:
            bindings = get_widget_binding_info(definition)
            if bindings:
                logger.info("Bindings:")
                for b in bindings:
                    logger.info(b)
        
        # Check current state
        asset_exists = does_asset_exist(asset_path)
        
        if dry_run:
            # Dry run - just report what would happen
            if asset_exists:
                if force_recreate:
                    logger.info(f"[DRY RUN] Would recreate: {name}")
                    results["recreated"].append(name)
                elif validate_existing:
                    existing_bp = load_asset(asset_path)
                    if existing_bp:
                        action, val_result, issues = get_action_for_existing_widget(
                            existing_bp, definition, name, AssetAction.SKIP
                        )
                        if val_result != ValidationResult.VALID:
                            logger.warning(f"[DRY RUN] Would report issues for: {name}")
                            results["validation_issues"][name] = issues
                        else:
                            logger.info(f"[DRY RUN] Would skip (valid): {name}")
                    results["skipped"].append(name)
                else:
                    logger.info(f"[DRY RUN] Would skip (exists): {name}")
                    results["skipped"].append(name)
            else:
                logger.info(f"[DRY RUN] Would create: {name}")
                results["created"].append(name)
        else:
            # Actually process the widget
            if asset_exists:
                if force_recreate:
                    # Force recreate
                    action = AssetAction.RECREATE
                elif validate_existing:
                    # Validate and determine action
                    existing_bp = load_asset(asset_path)
                    if existing_bp:
                        action, val_result, issues = get_action_for_existing_widget(
                            existing_bp, definition, name, AssetAction.SKIP
                        )
                        if val_result != ValidationResult.VALID:
                            results["validation_issues"][name] = issues
                    else:
                        action = AssetAction.SKIP
                else:
                    action = AssetAction.SKIP
            else:
                action = AssetAction.SKIP  # Will create since doesn't exist
            
            # Create or update the widget
            wbp, was_created = create_widget_blueprint(name, definition, action)
            
            if wbp:
                # Save the asset
                save_asset(asset_path)
                
                if was_created:
                    if action == AssetAction.RECREATE:
                        results["recreated"].append(name)
                    else:
                        results["created"].append(name)
                else:
                    if action == AssetAction.UPDATE:
                        results["updated"].append(name)
                    else:
                        results["skipped"].append(name)
            else:
                if not asset_exists:
                    # Only count as failed if we were trying to create it
                    results["failed"].append(name)
                else:
                    # Asset exists but we couldn't process it
                    results["skipped"].append(name)
    
    return results

def generate_report(results: Dict[str, Any]) -> Dict[str, Any]:
    """Generate a comprehensive summary report of the creation process."""
    logger.info("\n" + "=" * 60)
    logger.info("MF Widget Blueprint Creator - REPORT")
    logger.info("=" * 60)
    
    # Summary statistics
    logger.info(f"Total Widgets Defined: {results['total']}")
    logger.info(f"Created: {len(results['created'])}")
    logger.info(f"Recreated: {len(results.get('recreated', []))}")
    logger.info(f"Updated: {len(results.get('updated', []))}")
    logger.info(f"Skipped: {len(results['skipped'])}")
    logger.info(f"Failed: {len(results['failed'])}")

    # EUW summary (if present)
    euw = results.get("euw")
    if isinstance(euw, dict):
        if euw.get("created"):
            logger.info(f"EUW: Created {euw.get('name')}")
        elif euw.get("exists") or euw.get("skipped"):
            logger.info(f"EUW: Present {euw.get('name')}")
        elif euw.get("failed"):
            logger.warning(f"EUW: Failed to create {euw.get('name')}")
    
    validation_issues = results.get('validation_issues', {})
    if validation_issues:
        logger.info(f"Validation Issues: {len(validation_issues)} widgets")
    
    logger.info("-" * 60)
    
    if results["created"]:
        logger.info("\n✓ Created Widgets:")
        for name in results["created"]:
            req_count = len(WIDGET_DEFINITIONS[name].get("required_widgets", []))
            custom_req = len([w for w in WIDGET_DEFINITIONS[name].get("custom_widgets", []) if w.get("required", False)])
            total_req = req_count + custom_req
            logger.info(f"  + {name} ({total_req} required bindings)")
    
    if results.get("recreated"):
        logger.info("\n↻ Recreated Widgets:")
        for name in results["recreated"]:
            logger.info(f"  ↻ {name}")
    
    if results.get("updated"):
        logger.info("\n⟳ Updated Widgets (need manual binding):")
        for name in results["updated"]:
            logger.warning(f"  ⟳ {name}")
    
    if results["skipped"]:
        logger.info("\n~ Skipped Widgets (already existed):")
        for name in results["skipped"]:
            logger.info(f"  ~ {name}")
    
    if results["failed"]:
        logger.error("\n✗ Failed Widgets:")
        for name in results["failed"]:
            logger.error(f"  ✗ {name}")
    
    if validation_issues:
        logger.warning("\n⚠ Widgets with Validation Issues:")
        for name, issues in validation_issues.items():
            logger.warning(f"  ⚠ {name}:")
            for issue in issues:
                logger.warning(f"      - {issue}")
    
    # Post-execution steps
    logger.info("\n" + "-" * 60)
    logger.info("POST-EXECUTION STEPS:")
    logger.info("1. Open each WBP in UMG Designer")
    logger.info("2. Add required child widgets with EXACT names (case-sensitive)")
    logger.info("3. Check 'Is Variable' checkbox for each widget that needs binding")
    logger.info("4. Compile each WBP (Ctrl+Shift+S)")
    logger.info("5. Test in PIE mode")
    
    if validation_issues:
        logger.warning("\nWARNING: Some widgets have validation issues!")
        logger.warning("Review the issues above and fix manually in UMG Designer.")
    
    logger.info("=" * 60)
    
    # Log summary
    log_summary = logger.get_summary()
    if log_summary["errors"] > 0:
        logger.warning(f"\nLog Summary: {log_summary['errors']} errors, {log_summary['warnings']} warnings")
    
    return results

def generate_binding_guide():
    """Generate a detailed binding guide for manual widget setup."""
    logger.info("\n" + "=" * 60)
    logger.info("WIDGET BINDING GUIDE")
    logger.info("=" * 60)
    logger.info("Add these widgets in UMG Designer with EXACT names:\n")
    
    for name in CREATION_ORDER:
        definition = WIDGET_DEFINITIONS.get(name)
        if not definition:
            continue
        
        logger.info(f"\n{'='*50}")
        logger.info(f"  {name}")
        logger.info(f"{'='*50}")
        
        required = definition.get("required_widgets", [])
        optional = definition.get("optional_widgets", [])
        custom = definition.get("custom_widgets", [])
        
        if required:
            logger.info("\n  REQUIRED Widgets (must add):")
            for w in required:
                text_info = f' text="{w.get("text", "")}"' if "text" in w else ""
                logger.info(f'    [{w["type"]}] Name: "{w["name"]}"{text_info}')
        
        if custom:
            logger.info("\n  CUSTOM WBP Widgets:")
            for w in custom:
                req_str = "REQUIRED" if w.get("required", False) else "Optional"
                logger.info(f'    [{w["type"]}] Name: "{w["name"]}" ({req_str})')
        
        if optional:
            logger.info("\n  Optional Widgets:")
            for w in optional:
                text_info = f' text="{w.get("text", "")}"' if "text" in w else ""
                logger.info(f'    [{w["type"]}] Name: "{w["name"]}"{text_info}')
    
    logger.info("\n" + "=" * 60)
    logger.info("Remember: Widget names are CASE-SENSITIVE!")
    logger.info("Enable 'Is Variable' checkbox for all bound widgets.")
    logger.info("=" * 60)

# =============================================================================
# ENTRY POINT
# =============================================================================

def main(
    dry_run: bool = False, 
    show_guide: bool = True,
    force_recreate: bool = False,
    validate_existing: bool = True,
    create_euw: bool = True,
    only_euw: bool = False
) -> Dict[str, Any]:
    """
    Main entry point for the script.
    
    Args:
        dry_run: If True, only show what would be created without modifying
        show_guide: If True, show binding guide after creation
        force_recreate: If True, recreate all widgets even if they exist
        validate_existing: If True, validate existing widgets and report issues
        create_euw: If True, create the Editor Utility Widget
        only_euw: If True, create ONLY the EUW and skip all widget blueprints
    
    Returns:
        Dictionary with creation results
    
    Usage Examples:
        # Dry run to see what would happen
        main(dry_run=True)
        
        # Create missing widgets, skip existing ones
        main(dry_run=False)
        
        # Force recreate all widgets
        main(dry_run=False, force_recreate=True)
        
        # Create without showing the binding guide
        main(dry_run=False, show_guide=False)
        
        # Create ONLY the Editor Utility Widget
        main(only_euw=True)
    """
    # ------------------------------------------------------------
    # MODE: ONLY CREATE EDITOR UTILITY WIDGET (skip everything else)
    # ------------------------------------------------------------
    if only_euw:
        logger.info("\n")
        logger.info("=" * 60)
        logger.info("=== ONLY_EUW MODE ENABLED ===")
        logger.info("Creating Editor Utility Widget and skipping all WBPs.")
        logger.info("=" * 60)
        logger.info("\n")
        
        euw_result = _run_create_euw_impl()
        
        logger.info("\n")
        logger.info("=== ONLY_EUW MODE COMPLETE ===")
        logger.info("\n")
        # Keep a stable return shape for EUW-only callers.
        return {"only_euw": True, "euw": euw_result}
    
    logger.info("\n")
    logger.info("*" * 60)
    logger.info("*  P_MiniFootball - Widget Blueprint Creator")
    logger.info("*  Based on UI_WIDGETS.md specifications")
    logger.info("*" * 60)
    logger.info("\n")
    
    # Log mode
    if dry_run:
        logger.info("Mode: DRY RUN (no changes will be made)")
    elif force_recreate:
        logger.warning("Mode: FORCE RECREATE (all widgets will be recreated)")
    else:
        logger.info("Mode: CREATE/VALIDATE (create missing, validate existing)")
    
    logger.info("\n")
    
    # Optionally ensure the EUW tool asset exists
    euw_result: Dict[str, Any] = {
        "name": EUW_NAME,
        "path": f"{EUW_DEFINITION['path']}/{EUW_NAME}",
        "exists": does_asset_exist(f"{EUW_DEFINITION['path']}/{EUW_NAME}"),
        "created": False,
        "updated": False,
        "skipped": False,
        "failed": False,
    }

    if create_euw:
        if dry_run:
            if euw_result["exists"]:
                logger.info(f"[DRY RUN] EUW exists: {EUW_NAME}")
                euw_result["skipped"] = True
            else:
                logger.info(f"[DRY RUN] Would create EUW: {EUW_NAME} at {EUW_DEFINITION['path']}")
        else:
            try:
                euw_bp, euw_created = create_editor_utility_widget_blueprint(
                    EUW_NAME,
                    EUW_DEFINITION,
                    action=AssetAction.SKIP,
                )
                if euw_bp:
                    save_asset(euw_result["path"])
                    euw_result["exists"] = True
                    if euw_created:
                        euw_result["created"] = True
                    else:
                        euw_result["skipped"] = True
                else:
                    euw_result["failed"] = True
            except Exception as e:
                euw_result["failed"] = True
                logger.error(f"EUW creation step failed: {e}")

    # Create widgets
    results = create_all_widgets(
        dry_run=dry_run,
        force_recreate=force_recreate,
        validate_existing=validate_existing
    )

    results["euw"] = euw_result
    
    # === Apply Theme & Layout Rules (BEFORE binding guide) ===
    if not dry_run:
        logger.info("\n")
        logger.info("-" * 60)
        logger.info("Applying Theme & Layout Rules...")
        logger.info("-" * 60)
        _apply_theme_and_layout_pass(results)
    
    # Generate report
    generate_report(results)
    
    # Show binding guide
    if show_guide:
        generate_binding_guide()
    
    # Final summary
    logger.info("\n")
    logger.info("*" * 60)
    logger.info("*  Script Execution Complete")
    logger.info("*" * 60)
    
    return results

# =============================================================================
# QUICK ACCESS FUNCTIONS
# =============================================================================

def run_dry():
    """Run in dry-run mode to see what would happen."""
    return main(dry_run=True, show_guide=False)

def run_create():
    """Create missing widgets, skip existing ones."""
    return main(dry_run=False, show_guide=True, create_euw=True)

# ============================================================
# FIX: Safe EUW creation implementation (prevents recursion)
# ============================================================

def _run_create_euw_impl() -> Dict[str, Any]:
    """Actual implementation for creating the EUW (shared by module + class)."""
    euw_path = f"{EUW_DEFINITION['path']}/{EUW_NAME}"
    euw_result: Dict[str, Any] = {
        "name": EUW_NAME,
        "path": euw_path,
        "exists": does_asset_exist(euw_path),
        "created": False,
        "skipped": False,
        "failed": False,
    }

    try:
        euw_bp, euw_created = create_editor_utility_widget_blueprint(
            EUW_NAME,
            EUW_DEFINITION,
            action=AssetAction.SKIP,
        )
        if euw_bp:
            save_asset(euw_path)
            euw_result["created"] = bool(euw_created)
            euw_result["skipped"] = not euw_created
            euw_result["exists"] = True
        else:
            euw_result["failed"] = True
    except Exception as e:
        euw_result["failed"] = True
        logger.error(f"_run_create_euw_impl failed: {e}")

    return {"euw": euw_result}


def run_create_euw() -> Dict[str, Any]:
    """Public module-safe wrapper for creating (or repairing) the EUW asset only."""
    return _run_create_euw_impl()

def run_validate():
    """Validate all existing widgets without creating or modifying assets."""
    issues = validate_bindings()
    
    if issues:
        logger.warning(f"\n{len(issues)} widgets have validation issues!")
        for name, widget_issues in issues.items():
            logger.warning(f"  ⚠ {name}:")
            for issue in widget_issues:
                logger.warning(f"      - {issue}")
    else:
        logger.info("All widgets validated successfully. No binding issues detected.")
    
    # Mirror the structure of other helpers and main() return values
    return {
        "validation_issues": issues,
        "total": len(CREATION_ORDER),
    }

def run_force_recreate():
    """Force recreate all widgets (USE WITH CAUTION)."""
    logger.warning("FORCE RECREATE: This will delete and recreate all widgets!")
    return main(dry_run=False, force_recreate=True, show_guide=True)

# =============================================================================
# EDITOR UTILITY WIDGET INTEGRATION
# =============================================================================

class MF_WBP_EditorUtility:
    """
    Helper class for Editor Utility Widget integration.
    
    Use this class from Blueprint's "Execute Python Script" node:
    
    Example Python commands for EUW buttons:
    - Preview: exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); run_dry()
    - Create:  exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); run_create()
    - Validate: exec(open(r"D:/Projects/UE/A_MiniFootball/Plugins/P_MiniFootball/Scripts/MF_WidgetBlueprintCreator.py").read()); run_validate()
    """

    # ------------------------------------------------------------------
    # Convenience wrappers so EUW buttons can call MF_WBP.run_* directly
    # ------------------------------------------------------------------
    @staticmethod
    def run_dry():
        """Preview changes without modifying assets (EUW friendly)."""
        return run_dry()

    @staticmethod
    def run_create():
        """Create only missing widgets (EUW friendly)."""
        return run_create()

    @staticmethod
    def run_validate():
        """Validate existing widgets (EUW friendly)."""
        return run_validate()

    @staticmethod
    def run_force_recreate():
        """Delete and recreate all widgets (EUW friendly)."""
        return run_force_recreate()

    @staticmethod
    def run_create_euw():
        """Create (or repair) the EUW tool asset (EUW friendly)."""
        return _run_create_euw_impl()
    
    @staticmethod
    def get_status_text() -> str:
        """Get current status of all widget blueprints for display in EUW, with summary counts."""
        existing, missing, issues = MF_WBP_EditorUtility.get_widget_count()
        status_lines = [
            "=== MF Widget Blueprint Status ===",
            f"Existing: {existing} | Missing: {missing} | Issues: {issues}",
            ""
        ]

        for name in CREATION_ORDER:
            definition = WIDGET_DEFINITIONS.get(name)
            if not definition:
                continue

            asset_path = f"{definition['path']}/{name}"
            exists = does_asset_exist(asset_path)

            if exists:
                widget_bp = load_asset(asset_path)
                if widget_bp:
                    result, _issues = validate_widget_blueprint(widget_bp, definition, name)
                    if result == ValidationResult.VALID:
                        status_lines.append(f"✓ {name}")
                    else:
                        status_lines.append(f"⚠ {name} - {result.value}")
                else:
                    status_lines.append(f"? {name} - Load failed")
            else:
                status_lines.append(f"✗ {name} - Not created")

        return "\n".join(status_lines)
    
    @staticmethod
    def get_widget_count() -> Tuple[int, int, int]:
        """
        Get count of widgets: (existing, missing, with_issues)
        
        Returns:
            Tuple of (existing_count, missing_count, issues_count)
        """
        existing = 0
        missing = 0
        issues = 0
        
        for name in CREATION_ORDER:
            definition = WIDGET_DEFINITIONS.get(name)
            if not definition:
                continue
            
            asset_path = f"{definition['path']}/{name}"
            if does_asset_exist(asset_path):
                existing += 1
                widget_bp = load_asset(asset_path)
                if widget_bp:
                    result, _ = validate_widget_blueprint(widget_bp, definition, name)
                    if result != ValidationResult.VALID:
                        issues += 1
            else:
                missing += 1
        
        return (existing, missing, issues)
    
    @staticmethod
    def create_single_widget(widget_name: str) -> bool:
        """
        Create a single widget by name.
        
        Args:
            widget_name: e.g., "WBP_MF_ActionButton"
        
        Returns:
            True if created successfully
        """
        definition = WIDGET_DEFINITIONS.get(widget_name)
        if not definition:
            logger.error(f"No definition found for: {widget_name}")
            return False
        
        asset_path = f"{definition['path']}/{widget_name}"
        
        if does_asset_exist(asset_path):
            widget_bp = load_asset(asset_path)
            action, _, _ = get_action_for_existing_widget(widget_bp, definition, widget_name, default_action=AssetAction.UPDATE)
            wbp, created = create_widget_blueprint(widget_name, definition, action)
            if wbp:
                save_asset(asset_path)
                return True
            return False

        wbp, created = create_widget_blueprint(widget_name, definition, AssetAction.SKIP)
        if wbp and created:
            save_asset(asset_path)
            logger.info(f"Created: {widget_name}")
            return True
        
        return False
    
    @staticmethod
    def get_creation_order() -> List[str]:
        """Get the list of widgets in creation order."""
        return CREATION_ORDER.copy()

    @staticmethod
    def get_python_commands() -> Dict[str, str]:
        """Expose Python commands for EUW buttons (mirrors get_euw_python_commands)."""
        return get_euw_python_commands()
    
    @staticmethod
    def get_widget_info(widget_name: str) -> Dict[str, Any]:
        """
        Get detailed info about a widget for display in EUW.
        
        Args:
            widget_name: e.g., "WBP_MF_ActionButton"
        
        Returns:
            Dict with widget info
        """
        definition = WIDGET_DEFINITIONS.get(widget_name, {})
        asset_path = f"{definition.get('path', PLUGIN_COMPONENTS_PATH)}/{widget_name}"
        
        spec = get_widget_spec_from_cpp(widget_name)
        info = {
            "name": widget_name,
            "exists": does_asset_exist(asset_path),
            "path": asset_path,
            "parent_class": definition.get("parent_class", "Unknown"),
            "fill_screen": definition.get("fill_screen", False),
            "required_widgets": definition.get("required_widgets", []),
            "optional_widgets": definition.get("optional_widgets", []),
            "custom_widgets": definition.get("custom_widgets", []),
            "valid": False,
            "issues": [],
            "spec": spec,
        }

        if info["exists"]:
            widget_bp = load_asset(asset_path)
            if widget_bp:
                result, issues = validate_widget_blueprint(widget_bp, definition, widget_name)
                info["valid"] = (result == ValidationResult.VALID)
                info["issues"] = issues

        # Missing bindings calculation from validation
        if info["issues"]:
            missing = []
            for issue in info["issues"]:
                if "Missing required" in issue:
                    missing.append(issue)
            info["missing_bindings"] = missing
        else:
            info["missing_bindings"] = []
        
        return info


# ============================================================
# Module-level wrappers (EUW-friendly MF_WBP.* API exposure)
# ============================================================

def get_status_text() -> str:
    """Expose status text for EUW (MF_WBP.get_status_text)."""
    return MF_WBP_EditorUtility.get_status_text()


def get_widget_count() -> Tuple[int, int, int]:
    """Expose widget count for EUW."""
    return MF_WBP_EditorUtility.get_widget_count()


def create_single_widget(widget_name: str) -> bool:
    """Expose single-widget creation at module level."""
    return MF_WBP_EditorUtility.create_single_widget(widget_name)


def get_widget_info(widget_name: str) -> Dict[str, Any]:
    """Expose widget info API at module level."""
    return MF_WBP_EditorUtility.get_widget_info(widget_name)


def get_euw_python_commands() -> Dict[str, str]:
    """Expose Python commands dictionary for EUW."""
    script_path = SCRIPT_PATH.replace("\\", "/")

    return {
        "create_euw": f'exec(open(r"{script_path}").read()); MF_WBP.run_create_euw()',
        "preview": f'exec(open(r"{script_path}").read()); MF_WBP.run_dry()',
        "create": f'exec(open(r"{script_path}").read()); MF_WBP.run_create()',
        "validate": f'exec(open(r"{script_path}").read()); MF_WBP.run_validate()',
        "force": f'exec(open(r"{script_path}").read()); MF_WBP.run_force_recreate()',
        "status": f'exec(open(r"{script_path}").read()); print(MF_WBP.get_status_text())',
        "setup_guide": f'exec(open(r"{script_path}").read()); print_euw_setup_guide()',
    }


def print_euw_setup_guide():
    """Print instructions for setting up an Editor Utility Widget from the markdown guide."""
    guide_path = SCRIPT_PATH.replace("MF_WidgetBlueprintCreator.py", "EDITOR_UTILITY_WIDGET_GUIDE.md")
    try:
        with open(guide_path, "r", encoding="utf-8") as f:
            content = f.read()
        print(content)
        logger.info("EUW Setup Guide printed to Output Log")
    except Exception as e:
        logger.error(f"Failed to read guide at {guide_path}: {e}")
        print("Could not load EDITOR_UTILITY_WIDGET_GUIDE.md. Please open it manually.")


# =============================================================================
# DIAGNOSTIC UTILITIES
# =============================================================================

def diagnose_widget_blueprint(asset_path: str) -> Dict[str, Any]:
    """
    Comprehensive diagnostic for a single Widget Blueprint.
    
    Prints and returns diagnostics:
      - Asset exists
      - GeneratedClass present
      - CDO present
      - Widget tree available
      - Compile errors (if available)
    
    Args:
        asset_path: Full asset path (e.g., "/P_MiniFootball/BP/Widget/Components/WBP_MF_ScorePopup")
    
    Returns:
        Dict with diagnostic results
    """
    result = {
        "asset_path": asset_path,
        "asset_loaded": False,
        "generated_class_present": False,
        "cdo_present": False,
        "widget_tree_found": False,
        "compile_errors": [],
        "widget_count": 0,
    }

    # EUW is treated as a manual shell; skip diagnostics that rely on widget trees.
    try:
        if asset_path and asset_path.split("/")[-1] == EUW_NAME:
            print("=" * 70)
            print(f"DIAGNOSTIC: {asset_path}")
            print("-" * 70)
            print("EUW detected: skipping diagnostics (manual shell asset).")
            try:
                bp = unreal.EditorAssetLibrary.load_asset(asset_path)
                result["asset_loaded"] = bp is not None
            except Exception:
                pass
            print("=" * 70)
            print()
            result["skipped_euw"] = True
            return result
    except Exception:
        pass
    
    print("=" * 70)
    print(f"DIAGNOSTIC: {asset_path}")
    print("-" * 70)

    try:
        bp = unreal.EditorAssetLibrary.load_asset(asset_path)
        result["asset_loaded"] = bp is not None
        print(f"Asset Loaded: {bp is not None}")
        if not bp:
            print("=" * 70)
            print()
            return result

        # Generated class
        gen_class = None
        try:
            gen_class = bp.get_editor_property("generated_class")
        except Exception:
            gen_class = None
        result["generated_class_present"] = gen_class is not None
        print(f"Generated Class Present: {gen_class is not None}")

        # CDO
        cdo = None
        try:
            if gen_class and hasattr(gen_class, "get_default_object"):
                cdo = gen_class.get_default_object()
        except Exception:
            pass
        result["cdo_present"] = cdo is not None
        print(f"CDO Present: {cdo is not None}")

        # Widget Tree
        try:
            wt = get_widget_tree(bp)
            result["widget_tree_found"] = wt is not None
            print(f"Widget Tree Found: {wt is not None}")
            
            if wt:
                # Count widgets in tree
                widgets = get_all_widgets_in_tree(bp)
                result["widget_count"] = len(widgets)
                print(f"Widget Count: {len(widgets)}")
        except Exception as e:
            print(f"Widget Tree Error: {e}")

        # Compile errors (best-effort)
        try:
            if hasattr(unreal, 'KismetEditorUtilities'):
                lib = unreal.KismetEditorUtilities()
                if hasattr(lib, 'get_blueprint_errors'):
                    msgs = lib.get_blueprint_errors(bp)
                    if msgs:
                        result["compile_errors"] = list(msgs)
                        print("Compile Errors:")
                        for m in msgs:
                            print(f"  - {m}")
                    else:
                        print("Compile Errors: None")
                else:
                    print("Compile Errors: Could not retrieve (API not available)")
            else:
                print("Compile Errors: Could not retrieve (API not available)")
        except Exception as e:
            print(f"Compile Errors: Could not retrieve ({e})")

    except Exception as e:
        print(f"ERROR: {e}")

    print("=" * 70)
    print()
    
    return result


def diagnose_all_mf_widgets() -> Dict[str, Dict[str, Any]]:
    """
    Run comprehensive diagnosis on all MF widget blueprints.
    
    Returns:
        Dict mapping widget name to diagnostic results
    """
    print("\n=== MF UI DIAGNOSTIC REPORT ===\n")
    
    all_results = {}
    
    for name in CREATION_ORDER:
        if name == EUW_NAME:
            continue
        definition = WIDGET_DEFINITIONS.get(name)
        if not definition:
            continue
        asset_path = f"{definition['path']}/{name}"
        result = diagnose_widget_blueprint(asset_path)
        all_results[name] = result
    
    # Summary
    print("\n=== DIAGNOSTIC SUMMARY ===")
    loaded_count = sum(1 for r in all_results.values() if r["asset_loaded"])
    gen_class_count = sum(1 for r in all_results.values() if r["generated_class_present"])
    widget_tree_count = sum(1 for r in all_results.values() if r["widget_tree_found"])
    with_errors_count = sum(1 for r in all_results.values() if r["compile_errors"])
    
    print(f"Total widgets: {len(all_results)}")
    print(f"Assets loaded: {loaded_count}")
    print(f"Generated class present: {gen_class_count}")
    print(f"Widget tree found: {widget_tree_count}")
    print(f"With compile errors: {with_errors_count}")
    
    if widget_tree_count < loaded_count:
        print(f"\n⚠ WARNING: {loaded_count - widget_tree_count} widgets are missing widget trees!")
        for name, r in all_results.items():
            if r["asset_loaded"] and not r["widget_tree_found"]:
                print(f"  - {name}")
    
    print("\n=== END OF DIAGNOSTIC REPORT ===\n")
    
    return all_results


def diagnose_missing_bindings(asset_path: str, cpp_class_path: str) -> Dict[str, Any]:
    """
    Check if all BindWidget / BindWidgetOptional UPROPERTY names
    exist as widgets inside the Blueprint.
    
    Note: This is a best-effort check. The metadata retrieval may not work
    on all engine versions.
    
    Args:
        asset_path: Full asset path to the WBP
        cpp_class_path: Class path like "/Script/P_MiniFootball.MF_ScorePopup"
    
    Returns:
        Dict with binding diagnostic results
    """
    result = {
        "asset_path": asset_path,
        "cpp_class_path": cpp_class_path,
        "bind_names": [],
        "widget_names": set(),
        "missing": [],
        "satisfied": False,
    }
    
    print(f"\n=== Binding Diagnostic: {asset_path} ===")

    try:
        if asset_path and asset_path.split("/")[-1] == EUW_NAME:
            print("EUW detected: skipping binding diagnostics (manual shell asset).")
            result["skipped_euw"] = True
            return result
    except Exception:
        pass

    bp = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not bp:
        print("Blueprint missing.")
        return result

    # Load the class
    cpp_class = unreal.load_class(None, cpp_class_path)
    if not cpp_class:
        print(f"Could not load class: {cpp_class_path}")
        return result

    # Extract UPROPERTY names marked as BindWidget or BindWidgetOptional
    # This is engine-version dependent and may not work on all versions
    bind_names = []
    try:
        if hasattr(unreal, 'get_all_class_properties'):
            for prop in unreal.get_all_class_properties(cpp_class):
                try:
                    meta = None
                    if hasattr(unreal, 'get_metadata'):
                        meta = unreal.get_metadata(prop, "")
                    if meta:
                        if "BindWidget" in str(meta) or "BindWidgetOptional" in str(meta):
                            prop_name = prop.get_name() if hasattr(prop, 'get_name') else str(prop)
                            bind_names.append(prop_name)
                except Exception:
                    continue
    except Exception as e:
        print(f"Could not extract BindWidget metadata: {e}")
        print("(This is normal - metadata extraction is engine-version dependent)")

    result["bind_names"] = bind_names
    print(f"Expected BindWidget fields: {bind_names}")

    # Get widget tree names
    widgets = get_all_widgets_in_tree(bp)
    widget_names = {w[0] for w in widgets}
    result["widget_names"] = widget_names

    missing = [b for b in bind_names if b not in widget_names]
    result["missing"] = missing

    if missing:
        print("❌ Missing widgets:")
        for m in missing:
            print(f"  - {m}")
        result["satisfied"] = False
    else:
        print("✅ All bindings satisfied!")
        result["satisfied"] = True
    
    return result


# Alias for easy import (module-level)
_current_module = sys.modules[__name__]
# If executed via exec(open(...).read()), make `import MF_WidgetBlueprintCreator` work afterwards.
sys.modules["MF_WidgetBlueprintCreator"] = _current_module
# Also register with alternate name for flexibility
sys.modules.setdefault("MF_WidgetBlueprintCreator", _current_module)
MF_WBP = _current_module

# === MODULE-LEVEL EXPORTS FOR exec() SAFETY ===
# NOTE: Do NOT alias run_* names to MF_WBP_EditorUtility methods.
# Doing so overwrites the module functions and can cause infinite recursion.

# Export diagnostic functions at module level
diagnose_widget = diagnose_widget_blueprint
diagnose_all = diagnose_all_mf_widgets
diagnose_bindings = diagnose_missing_bindings

# Module-level function for EUW-only mode
def run_only_euw():
    """Create ONLY the Editor Utility Widget, skipping all Widget Blueprints."""
    return main(
        dry_run=False,
        show_guide=False,
        force_recreate=False,
        validate_existing=False,
        only_euw=True,
    )

__all__ = [
    "run_create_euw", 
    "run_create", 
    "run_dry", 
    "run_validate", 
    "run_force_recreate",
    "run_only_euw",
    "MF_WBP_EditorUtility",
    "diagnose_widget_blueprint",
    "diagnose_all_mf_widgets", 
    "diagnose_missing_bindings",
    "get_widget_tree",
    "get_euw_widget_tree",
    "get_status_text",
    "get_widget_count",
    "create_single_widget",
    "get_widget_info",
    "main",
]

# CLI wrapper function to prevent auto-execution during exec()
def cli():
    """
    Command-line interface wrapper for the MF Widget Blueprint Creator.
    
    This function is called when the script is run directly (not via exec).
    It performs the full widget blueprint creation workflow.
    """
    main(
        dry_run=False, 
        show_guide=True, 
        force_recreate=False, 
        validate_existing=True,
        only_euw=False  # Set to True for EUW-only generation
    )

def _should_autorun_cli() -> bool:
    """Return True only when running this file as a script (not via exec())."""
    file_from_globals = globals().get("__file__")
    if isinstance(file_from_globals, str) and os.path.basename(file_from_globals).lower() == "mf_widgetblueprintcreator.py":
        return True

    try:
        argv0 = sys.argv[0] if sys.argv else ""
        if isinstance(argv0, str) and os.path.basename(argv0).lower() == "mf_widgetblueprintcreator.py":
            return True
    except Exception:
        pass

    return False


# Run when script is executed directly (not via exec)
if __name__ == "__main__" and _should_autorun_cli():
    # Default: Create missing widgets, validate existing ones
    # Change these parameters as needed:
    #   dry_run=True        - Preview only, no changes
    #   force_recreate=True - Recreate all widgets
    #   show_guide=False    - Skip the binding guide
    #   only_euw=True       - Create ONLY the EUW, skip all WBPs
    cli()
    
    # Uncomment to print Editor Utility Widget setup instructions:
    # print_euw_setup_guide()
    
    # Uncomment to register menu entry (one-time setup):
    # register_editor_menu_entry()

# =============================================================================
# MODULE-LEVEL EXPORTS FOR EASY IMPORT
# =============================================================================
# After running this script once, you can import it:
#   import MF_WidgetBlueprintCreator as MF_WBP
#   MF_WBP.run_dry()
#   MF_WBP.run_create()
#   MF_WBP.run_validate()
#   MF_WBP.diagnose_all_mf_widgets()
#   MF_WBP.diagnose_widget_blueprint("/P_MiniFootball/BP/Widget/Components/WBP_MF_ScorePopup")
# =============================================================================
