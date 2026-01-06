<#
.SYNOPSIS
    Automated A_WCG Widget Generation and P_MWCS Validation Script
    
.DESCRIPTION
    This script automates the full widget generation and validation pipeline:
    1. Builds A_WCG tool (Release configuration)
    2. Runs A_WCG to convert HTML to widget specification
    3. Copies generated .h and .cpp files to P_MiniFootball Source
    4. Runs P_MWCS headless validation commandlet
    5. Displays any errors/warnings
    
.PARAMETER SourceHtml
    Path to the source HTML file to convert. Defaults to Knockout Framework.
    
.PARAMETER ClassName
    The class name for the generated widget. Defaults to "KnockoutFramework".
    
.PARAMETER SkipBuild
    Skip the A_WCG build step (useful if already built).
    
.PARAMETER ValidateOnly
    Only run MWCS validation, skip generation steps.

.PARAMETER Recreate
    Force recreate the Widget Blueprint using MWCS_CreateWidgets instead of just validating.

.EXAMPLE
    .\TestWidgetGeneration.ps1
    .\TestWidgetGeneration.ps1 -ClassName "MyWidget" -SourceHtml ".\path\to\source.html"
    .\TestWidgetGeneration.ps1 -ValidateOnly
    .\TestWidgetGeneration.ps1 -Recreate  # Force recreate existing WBP
#>

param(
    [string]$SourceHtml = "",
    [string]$ClassName = "KnockoutFramework",
    [switch]$SkipBuild,
    [switch]$ValidateOnly,
    [switch]$Recreate
)

$ErrorActionPreference = "Stop"

# ============================================================================
# Configuration
# ============================================================================

$ProjectRoot = "D:\Projects\UE\A_MiniFootball"
$UEEngineRoot = "D:\UE\UE_S"
$UProjectPath = "$ProjectRoot\A_MiniFootball.uproject"

# A_WCG paths
$AWCGRoot = "$ProjectRoot\Plugins\P_MWCS\A_WCG"
$AWCGExe = "$AWCGRoot\out\build\x64-release\bin\Release\awcg.exe"
$AWCGGeneratedDir = "$AWCGRoot\generated"

# Default source HTML
if ([string]::IsNullOrEmpty($SourceHtml)) {
    $OriginalDir = "$AWCGGeneratedDir\Original"
    $FoundFile = Get-ChildItem -Path $OriginalDir -File -ErrorAction SilentlyContinue | Select-Object -First 1
    
    if ($FoundFile) {
        $SourceHtml = $FoundFile.FullName
        Write-Host "[INFO] Auto-selected default source: $($FoundFile.Name)" -ForegroundColor White
    }
}

# P_MiniFootball paths
$PMiniFootballSource = "$ProjectRoot\Plugins\P_MiniFootball\Source\P_MiniFootball"
$UIOutputDir = "$PMiniFootballSource\Base\UI"

# UE commandlet paths
$UEEngineBuildBatch = "$UEEngineRoot\Engine\Build\BatchFiles\Build.bat"
$UEEditorCmd = "$UEEngineRoot\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

# ============================================================================
# Functions
# ============================================================================

function Write-Header {
    param([string]$Text)
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host " $Text" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
}

function Write-Success {
    param([string]$Text)
    Write-Host "[OK] $Text" -ForegroundColor Green
}

function Write-Error {
    param([string]$Text)
    Write-Host "[ERROR] $Text" -ForegroundColor Red
}

function Write-Warning {
    param([string]$Text)
    Write-Host "[WARN] $Text" -ForegroundColor Yellow
}

function Write-Info {
    param([string]$Text)
    Write-Host "[INFO] $Text" -ForegroundColor White
}

function Show-LogAnalysis {
    param([string]$LogPath)

    if (Test-Path $LogPath) {
        $Output = Get-Content $LogPath -Raw
        
        # Extract relevant lines (errors, warnings, MWCS messages)
        $RelevantLines = $Output -split "`n" | Where-Object {
            $_ -match "MWCS" -or
            $_ -match "Error" -or
            $_ -match "Warning" -or
            $_ -match "validation" -or
            $_ -match "Builder\." -or
            $_ -match "Created widget" -or
            $_ -match "Failed"
        }
        
        if ($RelevantLines.Count -gt 0) {
            Write-Host ""
            Write-Host "--- Relevant Log Lines ($([System.IO.Path]::GetFileName($LogPath))) ---" -ForegroundColor Yellow
            foreach ($line in $RelevantLines) {
                if ($line -match "Error") {
                    Write-Host $line -ForegroundColor Red
                }
                elseif ($line -match "Warning") {
                    Write-Host $line -ForegroundColor Yellow
                }
                else {
                    Write-Host $line -ForegroundColor White
                }
            }
            Write-Host "--- End Log ---" -ForegroundColor Yellow
            Write-Host ""
        }
    }
}


# ============================================================================
# Step 1: Build A_WCG
# ============================================================================

if (-not $ValidateOnly -and -not $SkipBuild) {
    Write-Header "Step 1: Building A_WCG"
    
    Push-Location $AWCGRoot
    try {
        & ".\DevTools\scripts\Build.ps1" -Configuration Release
        if ($LASTEXITCODE -ne 0) {
            Write-Error "A_WCG build failed!"
            Pop-Location
            exit 1
        }
        Write-Success "A_WCG built successfully"
    }
    finally {
        Pop-Location
    }
}
elseif ($SkipBuild) {
    Write-Info "Skipping A_WCG build (--SkipBuild)"
}

# ============================================================================
# Step 2: Generate Widget Files
# ============================================================================

if (-not $ValidateOnly) {
    Write-Header "Step 2: Generating Widget Files"
    
    if (-not (Test-Path $AWCGExe)) {
        Write-Error "A_WCG executable not found: $AWCGExe"
        Write-Info "Run without -SkipBuild to build first."
        exit 1
    }
    
    if (-not (Test-Path $SourceHtml)) {
        Write-Error "Source HTML not found: $SourceHtml"
        exit 1
    }
    
    Write-Info "Source: $SourceHtml"
    Write-Info "Class: $ClassName"
    
    & $AWCGExe -s $SourceHtml -o $AWCGGeneratedDir -c $ClassName -m "P_MiniFootball" -v
    
    if ($LASTEXITCODE -ne 0) {
        Write-Error "A_WCG generation failed!"
        exit 1
    }
    
    Write-Success "Widget files generated"
    
    # ========================================================================
    # Step 3: Copy Generated Files to P_MiniFootball
    # ========================================================================
    
    Write-Header "Step 3: Copying Files to P_MiniFootball"
    
    # Create UI output directory if it doesn't exist
    if (-not (Test-Path $UIOutputDir)) {
        New-Item -ItemType Directory -Path $UIOutputDir -Force | Out-Null
        Write-Info "Created directory: $UIOutputDir"
    }
    
    # Copy .h file
    $HeaderFile = "$AWCGGeneratedDir\$ClassName.h"
    if (Test-Path $HeaderFile) {
        Copy-Item -Path $HeaderFile -Destination $UIOutputDir -Force
        Write-Success "Copied: $ClassName.h"
    }
    else {
        Write-Warning "Header file not found: $HeaderFile"
    }
    
    # Copy .cpp file
    $CppFile = "$AWCGGeneratedDir\$ClassName.cpp"
    if (Test-Path $CppFile) {
        Copy-Item -Path $CppFile -Destination $UIOutputDir -Force
        Write-Success "Copied: $ClassName.cpp"
    }
    else {
        Write-Warning "CPP file not found: $CppFile"
    }
    
    # Copy .json file (for reference)
    $JsonFile = "$AWCGGeneratedDir\$ClassName.json"
    if (Test-Path $JsonFile) {
        Copy-Item -Path $JsonFile -Destination $UIOutputDir -Force
        Write-Success "Copied: $ClassName.json"
    }
    
    Write-Success "Files copied to: $UIOutputDir"
    Write-Success "Files copied to: $UIOutputDir"
}

# ============================================================================
# Step 4: Build P_MiniFootball Project
# ============================================================================

Write-Header "Step 4: Building P_MiniFootball Project"

if (-not $ValidateOnly) {
    if (-not (Test-Path $UEEngineBuildBatch)) {
        Write-Error "UE Build.bat not found: $UEEngineBuildBatch"
        exit 1
    }

    Write-Info "Building A_MiniFootballEditor Win64 Development..."

    $BuildArgs = @(
        "A_MiniFootballEditor",
        "Win64",
        "Development",
        "-Project=`"$UProjectPath`"",
        "-WaitMutex",
        "-FromMsBuild"
    )

    $BuildProcess = Start-Process -FilePath $UEEngineBuildBatch `
        -ArgumentList $BuildArgs `
        -NoNewWindow `
        -Wait `
        -PassThru

    if ($BuildProcess.ExitCode -eq 0) {
        Write-Success "Project built successfully"
    }
    else {
        Write-Error "Project build failed! Exit code: $($BuildProcess.ExitCode)"
        exit 1
    }
}
else {
    Write-Info "Skipping Project build (--ValidateOnly)"
}

# ============================================================================
# Step 5: Run MWCS Operations
# ============================================================================

Write-Header "Step 5: Running MWCS Operations"

if (-not (Test-Path $UEEditorCmd)) {
    Write-Error "UE Editor-Cmd not found: $UEEditorCmd"
    Write-Info "Please verify UE Engine path in script configuration."
    exit 1
}

if (-not (Test-Path $UProjectPath)) {
    Write-Error "UProject file not found: $UProjectPath"
    exit 1
}

$CommonArgs = @(
    "`"$UProjectPath`"",
    "-FailOnErrors",
    "-unattended",
    "-nop4",
    "-NullRHI",
    "-stdout",
    "-FullStdOutLogOutput"
)

# 5a. Recreate Widgets (Default if generating, or forced via -Recreate)
if ($Recreate -or -not $ValidateOnly) {
    Write-Info "Running: MWCS_CreateWidgets -Mode=ForceRecreate commandlet..."
    $CreateArgs = $CommonArgs + "-run=MWCS_CreateWidgets" + "-Mode=ForceRecreate"
    
    $CreateProcess = Start-Process -FilePath $UEEditorCmd `
        -ArgumentList $CreateArgs `
        -NoNewWindow `
        -Wait `
        -PassThru `
        -RedirectStandardOutput "$AWCGGeneratedDir\create_output.txt" `
        -RedirectStandardError "$AWCGGeneratedDir\create_errors.txt"
        
    Show-LogAnalysis -LogPath "$AWCGGeneratedDir\create_output.txt"
    
    if ($CreateProcess.ExitCode -ne 0) {
        Write-Error "Widget Creation failed! Exit code: $($CreateProcess.ExitCode)"
        exit $CreateProcess.ExitCode
    }
    Write-Success "Widget Recreated Successfully"
}

# 5b. Validate Widgets (Always)
Write-Info "Running: MWCS_ValidateWidgets commandlet..."
$ValidateArgs = $CommonArgs + "-run=MWCS_ValidateWidgets"

$ValidateProcess = Start-Process -FilePath $UEEditorCmd `
    -ArgumentList $ValidateArgs `
    -NoNewWindow `
    -Wait `
    -PassThru `
    -RedirectStandardOutput "$AWCGGeneratedDir\validation_output.txt" `
    -RedirectStandardError "$AWCGGeneratedDir\validation_errors.txt"

$ValidationExitCode = $ValidateProcess.ExitCode

# ============================================================================
# Step 6: Display Results
# ============================================================================

Write-Header "Step 6: Validation Results"

Show-LogAnalysis -LogPath "$AWCGGeneratedDir\validation_output.txt"

if (Test-Path "$AWCGGeneratedDir\validation_errors.txt") {
    $Errors = Get-Content "$AWCGGeneratedDir\validation_errors.txt" -Raw
    if (-not [string]::IsNullOrWhiteSpace($Errors)) {
        Write-Host ""
        Write-Host "--- Standard Error ---" -ForegroundColor Red
        Write-Host $Errors -ForegroundColor Red
        Write-Host "--- End ---" -ForegroundColor Red
    }
}

Write-Host ""
if ($ValidationExitCode -eq 0) {
    Write-Success "MWCS Validation PASSED!"
}
else {
    Write-Error "MWCS Validation FAILED! Exit code: $ValidationExitCode"
    Write-Info "Full output: $AWCGGeneratedDir\validation_output.txt"
    Write-Info "Reports: $ProjectRoot\Saved\MWCS\Reports\"
}

# Check if SpecsProcessed > 0
$Processed = Select-String -Path "$AWCGGeneratedDir\validation_output.txt" -Pattern "SpecsProcessed=(\d+)"
if ($Processed) {
    $Count = [int]$Processed.Matches.Groups[1].Value
    if ($Count -eq 0) {
        Write-Error "MWCS Validation processed 0 specs! Check DefaultEditor.ini or plugin configuration."
        exit 1
    }
} else {
     Write-Warning "Could not determine SpecsProcessed count."
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Pipeline Complete" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Generated files location: $UIOutputDir" -ForegroundColor White
Write-Host "MWCS Reports: $ProjectRoot\Saved\MWCS\Reports\" -ForegroundColor White
Write-Host ""

# ============================================================================
# Step 7: JSON Spec Analysis & Validation
# ============================================================================

Write-Header "Step 7: JSON Spec Verification"

$JsonPath = "$AWCGGeneratedDir\$ClassName.json"

function Test-JsonContent {
    param($Json, $NameRegex, $Description)
    
    $Count = ([regex]::Matches($Json, "$NameRegex")).Count
    if ($Count -gt 0) {
        Write-Host "  [OK] Found $Description ($Count matches)" -ForegroundColor Green
        return $true
    } else {
        Write-Host "  [FAIL] Missing $Description" -ForegroundColor Red
        return $false
    }
}

if (Test-Path $JsonPath) {
    $JsonContent = Get-Content $JsonPath -Raw
    
    # 7a. Structural Checks
    Write-Host "Checking Structure..." -ForegroundColor Yellow
    
    $Checks = @(
        @{ Name = '"Type": "TextBlock"'; Desc = "TextBlock widgets" },
        @{ Name = '"Type": "TransparentButton"'; Desc = "Button widgets" },
        @{ Name = '"Type": "ScrollBox"'; Desc = "Scrollable Container" },
        @{ Name = '"Slot":'; Desc = "Layout Slots" },
        @{ Name = '"AutoWrapText": true'; Desc = "Auto-wrapping enabled" }
    )
    
    $Failed = $false
    foreach ($Check in $Checks) {
        if (-not (Test-JsonContent -Json $JsonContent -NameRegex $Check.Name -Description $Check.Desc)) {
            $Failed = $true
        }
    }
    
    # 7b. Content Parity Checks (Specific to Knockout issues)
    Write-Host ""
    Write-Host "Checking Content Parity..." -ForegroundColor Yellow
    
    $ContentChecks = @(
        @{ Name = '"Text": "6\."'; Desc = "Ordered List Numbering (6.)" },
        @{ Name = '"Text": "Knockout Setup"'; Desc = "List Item Text" },
        @{ Name = '"Text": "Headlines"'; Desc = "Sub-list Item" },
        @{ Name = '"Text": "Updates"'; Desc = "Previously missing 'Updates' text" },
        @{ Name = '"Text": "Color Quick Setup"'; Desc = "Section Header" },
        @{ Name = '"Name": "A_16"'; Desc = "Transparent Button A_16" }
    )
    
    foreach ($Check in $ContentChecks) {
        # Escape regex special chars in the Name string if needed, but here simple strings
        # We need to escape the regex specifically for special chars like .
        $Regex = [regex]::Escape($Check.Name).Replace("\\", "") # Simple hack, cleaner is to use proper regex
        # Actually our check regex is strict string matching effectively
        
        if (-not (Test-JsonContent -Json $JsonContent -NameRegex $Check.Name -Description $Check.Desc)) {
            $Failed = $true
        }
    }

    Write-Host ""
    if ($Failed) {
        Write-Error "Validation FAILED: Significant structural or content mismatches found."
        # Don't exit with error yet, ensure we see the full picture
    } else {
        Write-Success "All Validation Checks PASSED!"
    }
} else {
    Write-Warning "JSON file not found: $JsonPath"
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Pipeline Complete" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Generated files location: $UIOutputDir" -ForegroundColor White
Write-Host "MWCS Reports: $ProjectRoot\Saved\MWCS\Reports\" -ForegroundColor White
Write-Host ""

exit $ValidationExitCode
