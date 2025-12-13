#include "MF_WidgetCreatorUtilityWidget.h"

static FString G_MF_EUW_WidgetCreatorSpec;

static FString BuildMFWidgetCreatorSpec()
{
  // JSON spec describing the recommended EUW_MF_WidgetCreator layout.
  // Legacy: this utility widget pre-dates P_MWCS.
  const TCHAR *Json = TEXT(R"JSON({
  "WidgetClass": "UMF_WidgetCreatorUtilityWidget",
  "BlueprintName": "EUW_MF_WidgetCreator",
  "ParentClass": "/Script/P_MiniFootballEditor.MF_WidgetCreatorUtilityWidget",
  "Category": "MF|Editor|UI",
  "Description": "(Legacy) Editor Utility Widget layout spec (MWCS provides its own tools UI)",

  "Hierarchy": {
    "Root": { "Type": "CanvasPanel", "Name": "RootPanel" },
    "Children": [
      {
        "Name": "MainContainer",
        "Type": "VerticalBox",
        "BindingType": "Required"
      },
      { "Name": "Title", "Type": "TextBlock", "BindingType": "Optional" },
      { "Name": "StatusBar", "Type": "HorizontalBox", "BindingType": "Optional" },
      { "Name": "ExistingCount", "Type": "TextBlock", "BindingType": "Optional" },
      { "Name": "MissingCount", "Type": "TextBlock", "BindingType": "Optional" },
      { "Name": "IssuesCount", "Type": "TextBlock", "BindingType": "Optional" },
      { "Name": "PreviewButton", "Type": "Button", "BindingType": "Required" },
      { "Name": "CreateButton", "Type": "Button", "BindingType": "Required" },
      { "Name": "ValidateButton", "Type": "Button", "BindingType": "Required" },
      { "Name": "ForceRecreateButton", "Type": "Button", "BindingType": "Required" },
      { "Name": "OutputLog", "Type": "MultiLineEditableTextBox", "BindingType": "Optional" }
    ]
  },

  "Bindings": {
    "Required": [
      "MainContainer",
      "PreviewButton",
      "CreateButton",
      "ValidateButton",
      "ForceRecreateButton"
    ],
    "Optional": [
      "Title",
      "StatusBar",
      "ExistingCount",
      "MissingCount",
      "IssuesCount",
      "OutputLog"
    ]
  }
})JSON");

  return FString(Json);
}

FString UMF_WidgetCreatorUtilityWidget::GetWidgetSpec()
{
  if (G_MF_EUW_WidgetCreatorSpec.IsEmpty())
  {
    G_MF_EUW_WidgetCreatorSpec = BuildMFWidgetCreatorSpec();
  }
  return G_MF_EUW_WidgetCreatorSpec;
}
