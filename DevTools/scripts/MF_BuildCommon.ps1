# Common build helpers for P_MiniFootball DevTools scripts.
# Dot-source this file:
#   . "$PSScriptRoot\MF_BuildCommon.ps1"

function Get-UEBuildBatPath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$UEEngineRoot
    )

    return Join-Path $UEEngineRoot "Engine\Build\BatchFiles\Build.bat"
}

function Invoke-UEProjectBuild {
    param(
        [Parameter(Mandatory = $true)]
        [string]$UEEngineRoot,

        [Parameter(Mandatory = $true)]
        [string]$UProjectPath,

        [string]$Target = "A_MiniFootballEditor",
        [ValidateSet("Win64", "Linux", "LinuxArm64", "Android")]
        [string]$Platform = "Win64",
        [string]$Configuration = "Development",
        [switch]$FromMsBuild
    )

    $buildBat = Get-UEBuildBatPath -UEEngineRoot $UEEngineRoot

    if (-not (Test-Path $buildBat)) {
        throw "UE Build.bat not found: $buildBat"
    }

    if (-not (Test-Path $UProjectPath)) {
        throw "UProject file not found: $UProjectPath"
    }

    $buildArgs = @(
        $Target,
        $Platform,
        $Configuration,
        "-Project=`"$UProjectPath`"",
        "-WaitMutex"
    )

    if ($FromMsBuild) {
        $buildArgs += "-FromMsBuild"
    }

    $process = Start-Process -FilePath $buildBat `
        -ArgumentList $buildArgs `
        -NoNewWindow `
        -Wait `
        -PassThru

    return $process.ExitCode
}
