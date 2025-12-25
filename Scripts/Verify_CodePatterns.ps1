# ============================================================================
# VERIFY_CODEPATTERNS.PS1
# Validates that forbidden patterns are removed and correct patterns exist
# ============================================================================

param(
    [string]$SourceRoot = "Source/P_MiniFootball"
)

Write-Host "`n=== Code Pattern Verification ===" -ForegroundColor Cyan

$errors = @()

# --- FORBIDDEN PATTERNS ---

Write-Host "`nChecking for forbidden patterns..." -ForegroundColor Yellow

# Forbidden: SwitchToNextCharacter method definition or call (not comments)
$matches = Get-ChildItem -Path $SourceRoot -Recurse -Include "*.cpp", "*.h" |
    Select-String -Pattern "void SwitchToNextCharacter|SwitchToNextCharacter\(\)"

if ($matches.Count -gt 0) {
    Write-Host "  X SwitchToNextCharacter still exists" -ForegroundColor Red
    $errors += "SwitchToNextCharacter method not deleted"
} else {
    Write-Host "  OK SwitchToNextCharacter removed" -ForegroundColor Green
}

# Forbidden: Cached UInputAction pointers in MF_InputHandler
$inputHandlerFiles = Get-ChildItem -Path $SourceRoot -Recurse -Include "MF_InputHandler.h" -ErrorAction SilentlyContinue
if ($inputHandlerFiles) {
    $matches = $inputHandlerFiles | Select-String -Pattern "UPROPERTY.*UInputAction"
    if ($matches.Count -gt 0) {
        Write-Host "  X Cached UInputAction pointers in MF_InputHandler" -ForegroundColor Red
        $errors += "MF_InputHandler still caches UInputAction pointers"
    } else {
        Write-Host "  OK No cached UInputAction pointers" -ForegroundColor Green
    }
}

# Forbidden: SpawnActor<ABall> or SpawnActor<AMF_Ball> in PlayerController (GameMode can legitimately spawn)
$matches = Get-ChildItem -Path $SourceRoot -Recurse -Include "MF_PlayerController.cpp" |
    Select-String -Pattern "SpawnActor<ABall>|SpawnActor<AMF_Ball>"

if ($matches.Count -gt 0) {
    Write-Host "  X Forbidden ball spawn pattern in PlayerController" -ForegroundColor Red
    $errors += "SpawnActor<Ball> in PlayerController - should use GameState->GetMatchBall()"
} else {
    Write-Host "  OK No forbidden ball spawn patterns in PlayerController" -ForegroundColor Green
}

if ($errors.Count -eq 0) {
    Write-Host "  OK All forbidden patterns verified removed" -ForegroundColor Green
}

# --- REQUIRED PATTERNS ---

Write-Host "`nChecking for required patterns..." -ForegroundColor Yellow

# Required: SwitchToNearestToBall exists
$matches = Get-ChildItem -Path "$SourceRoot/Base/Player/MF_PlayerController.*" -ErrorAction SilentlyContinue |
    Select-String -Pattern "SwitchToNearestToBall"

if ($matches.Count -eq 0) {
    Write-Host "  X SwitchToNearestToBall not found" -ForegroundColor Red
    $errors += "SwitchToNearestToBall method not implemented"
} else {
    Write-Host "  OK SwitchToNearestToBall found" -ForegroundColor Green
}

# Required: OnRequestTeamChange delegate
$matches = Get-ChildItem -Path "$SourceRoot/Base/UI/MF_PauseMenu.h" -ErrorAction SilentlyContinue |
    Select-String -Pattern "OnRequestTeamChange"

if ($matches.Count -eq 0) {
    Write-Host "  X OnRequestTeamChange delegate not found" -ForegroundColor Red
    $errors += "FMF_RequestTeamChange delegate not declared"
} else {
    Write-Host "  OK OnRequestTeamChange delegate found" -ForegroundColor Green
}

# Required: GameState->GetMatchBall usage
$matches = Get-ChildItem -Path $SourceRoot -Recurse -Include "*.cpp" |
    Select-String -Pattern "GetMatchBall"

if ($matches.Count -eq 0) {
    Write-Host "  X GetMatchBall not found - ball resolution not using GameState" -ForegroundColor Red
    $errors += "GetMatchBall not found - should resolve ball via GameState"
} else {
    Write-Host "  OK GetMatchBall found" -ForegroundColor Green
}

# --- SUMMARY ---

if ($errors.Count -gt 0) {
    Write-Host "`nX Pattern Verification FAILED:" -ForegroundColor Red
    foreach ($err in $errors) {
        Write-Host "   - $err" -ForegroundColor Red
    }
    exit 1
} else {
    Write-Host "`nOK Pattern Verification PASSED" -ForegroundColor Green
    exit 0
}
