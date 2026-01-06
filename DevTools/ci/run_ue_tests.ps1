<#
.SYNOPSIS
    Runs Unreal Engine automation tests headlessly.
.PARAMETER ProjectPath
    Path to the .uproject file.
.PARAMETER Filter
    Filter for tests to run (e.g. "P_MiniFootball").
#>

param(
    [string]$UePath = $null,
    [string]$ProjectPath = $null,
    [string]$Filter = "P_MiniFootball"
)

$ErrorActionPreference = "Stop"

# 1. Discover Unreal
$LibDir = Join-Path (Join-Path $PSScriptRoot "..") "lib"
$FindUnreal = Join-Path $LibDir "Find-Unreal.ps1"
$UE = & $FindUnreal -UePath $UePath

# 2. Resolve Project Path
$ResolvedProject = ""
if ($ProjectPath) {
    if (Test-Path $ProjectPath) {
        $ResolvedProject = (Get-Item $ProjectPath).FullName
    }
}
else {
    # Scan sibling or parent for .uproject
    $SearchDir = (Get-Item (Join-Path (Join-Path (Join-Path (Join-Path $PSScriptRoot "..") "..") "..") "..")).FullName
    $Projects = Get-ChildItem -Path $SearchDir -Filter "*.uproject"
    if ($Projects.Count -eq 1) {
        $ResolvedProject = $Projects[0].FullName
    }
}

if (-not $ResolvedProject) {
    Write-Error "Could not find .uproject file. Please specify --ProjectPath."
}

# 3. Prepare results
$ResultDir = (New-Item -ItemType Directory -Force -Path "Artifacts/TestResults").FullName
$LogPath = Join-Path $ResultDir "test_run.log"

Write-Host "Running tests for project: $ResolvedProject"
Write-Host "Filter: $Filter"

# 4. Run
$ExecCmds = "Automation RunTests $Filter; Quit"
$RunArgs = @(
    "`"$ResolvedProject`"",
    "-unattended",
    "-nop4",
    "-nosplash",
    "-NullRHI",
    "-ExecCmds=`"$ExecCmds`"",
    "-log",
    "-ReportOutputPath=`"$ResultDir`""
)

Write-Host "Running: $($UE.UNREAL_CMD) $($RunArgs -join ' ')"
& $UE.UNREAL_CMD @RunArgs > $LogPath

Write-Host "Tests execution finished. Check $ResultDir for results."
