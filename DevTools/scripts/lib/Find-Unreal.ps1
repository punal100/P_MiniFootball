<#
.SYNOPSIS
    Discovers Unreal Engine installation and validates prerequisites.
.DESCRIPTION
    Resolves UE_ROOT and checks for required binaries (RunUAT, UnrealEditor-Cmd).
    Outputs environment variables and paths.
#>

param(
    [string]$UePath = $null
)

$ErrorActionPreference = "Stop"

function Log($Msg) {
    Write-Host "[Find-Unreal] $Msg"
}

# 1. Resolve UE_ROOT
$ResolvedUeRoot = ""

if ($UePath -and (Test-Path $UePath)) {
    $ResolvedUeRoot = $UePath
    Log "Using provided path: $ResolvedUeRoot"
} elseif ($env:UE_ROOT -and (Test-Path $env:UE_ROOT)) {
    $ResolvedUeRoot = $env:UE_ROOT
    Log "Using environment variable UE_ROOT: $ResolvedUeRoot"
} else {
    # Try common paths if not provided
    $SearchPaths = @(
        "C:\Program Files\Epic Games\UE_5.5",
        "D:\UE\UE_5.5",
        "E:\UE\UE_5.5"
    )
    
    foreach ($Path in $SearchPaths) {
        if (Test-Path $Path) {
            $ResolvedUeRoot = $Path
            Log "Discovered UE at: $ResolvedUeRoot"
            break
        }
    }
}

if (-not $ResolvedUeRoot) {
    Write-Error "Could not resolve Unreal Engine root directory. Please set UE_ROOT environment variable."
}

# 2. Validate prerequisites
$Prerequisites = @(
    "Engine\Build\BatchFiles\RunUAT.bat",
    "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
)

foreach ($RelPath in $Prerequisites) {
    $FullPath = Join-Path $ResolvedUeRoot $RelPath
    if (-not (Test-Path $FullPath)) {
        Write-Error "Missing required binary: $FullPath"
    }
}

# 3. Output results
$Output = @{
    UE_ROOT = $ResolvedUeRoot
    RUN_UAT = Join-Path $ResolvedUeRoot "Engine\Build\BatchFiles\RunUAT.bat"
    UNREAL_CMD = Join-Path $ResolvedUeRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
}

Log "Validation successful."
return $Output
