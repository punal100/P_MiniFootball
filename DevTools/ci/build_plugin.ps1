<#
.SYNOPSIS
    Builds the plugin headlessly using RunUAT.
.PARAMETER UePath
    Path to Unreal Engine root.
.PARAMETER PluginPath
    Path to the .uplugin file.
.PARAMETER OutPath
    Where to store the packaged plugin.
#>

param(
    [string]$UePath = $null,
    [string]$PluginPath = $null,
    [string]$OutPath = "Artifacts/PluginBuild"
)

$ErrorActionPreference = "Stop"

# 1. Discover Unreal
$LibDir = Join-Path (Join-Path $PSScriptRoot "..") "lib"
$FindUnreal = Join-Path $LibDir "Find-Unreal.ps1"
$UE = & $FindUnreal -UePath $UePath

# 2. Resolve Plugin Path
$ResolvedPlugin = ""
if ($PluginPath) {
    if (Test-Path $PluginPath) {
        $ResolvedPlugin = (Get-Item $PluginPath).FullName
    }
}
else {
    # Scan root for .uplugin
    $RepoRoot = (Get-Item (Join-Path (Join-Path $PSScriptRoot "..") "..")).FullName
    $Plugins = Get-ChildItem -Path $RepoRoot -Filter "*.uplugin"
    if ($Plugins.Count -eq 1) {
        $ResolvedPlugin = $Plugins[0].FullName
    }
    elseif ($Plugins.Count -gt 1) {
        Write-Error "Multiple .uplugin files found. Please specify --PluginPath."
    }
}

if (-not $ResolvedPlugin) {
    Write-Error "Could not find .uplugin file."
}

# 3. Prepare output
$AbsoluteOut = (New-Item -ItemType Directory -Force -Path $OutPath).FullName
$LogPath = (New-Item -ItemType Directory -Force -Path "Artifacts/Logs").FullName

Write-Host "Building plugin: $ResolvedPlugin"
Write-Host "Output path: $AbsoluteOut"

# 4. Build
$BuildArgs = @(
    "BuildPlugin",
    "-Plugin=`"$ResolvedPlugin`"",
    "-Package=`"$AbsoluteOut`"",
    "-Rocket"
)

Write-Host "Running: $($UE.RUN_UAT) $($BuildArgs -join ' ')"
& $UE.RUN_UAT @BuildArgs > (Join-Path $LogPath "build_plugin.log")

Write-Host "Build completed successfully."
