# ============================================================================
# VERIFY_ACTIONNAMEPARITY.PS1
# Ensures ActionNames in JSON profiles match code usage
# ============================================================================

param(
    [string]$ProfilePath = "Saved/InputProfiles/Default.json",
    [string]$SourcePath = "Source"
)

Write-Host "`n=== ActionName Parity Verification ===" -ForegroundColor Cyan

if (-not (Test-Path $ProfilePath)) {
    Write-Host "X Profile not found: $ProfilePath" -ForegroundColor Red
    Write-Host "  Skipping ActionName parity check (no profile to verify)" -ForegroundColor Yellow
    exit 0
}

$json = Get-Content $ProfilePath -Raw | ConvertFrom-Json
$code = Get-ChildItem $SourcePath -Recurse -Include "*.cpp", "*.h" | Get-Content -Raw

$missing = @()

foreach ($binding in $json.ActionBindings) {
    $actionName = $binding.InputActionName
    if ($code -notmatch [regex]::Escape($actionName)) {
        $missing += $actionName
    }
}

if ($missing.Count -gt 0) {
    Write-Host "`nX ActionNames missing in code:" -ForegroundColor Red
    foreach ($name in $missing) {
        Write-Host "   - $name" -ForegroundColor Red
    }
    exit 1
}

Write-Host "OK ActionName parity verified - all JSON actions found in code" -ForegroundColor Green
exit 0
