param(
    [switch]$EnableGitHook = $true
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$updateScript = Join-Path $repoRoot "scripts\update_memory.ps1"

if (-not (Test-Path $updateScript)) {
    throw "Missing update script: $updateScript"
}

if ($EnableGitHook) {
    $gitDir = Join-Path $repoRoot ".git"
    if (Test-Path $gitDir) {
        $hooksDir = Join-Path $gitDir "hooks"
        New-Item -ItemType Directory -Force -Path $hooksDir | Out-Null

        $hookPath = Join-Path $hooksDir "post-commit"
        $hookScript = @"
#!/bin/sh
powershell -NoProfile -ExecutionPolicy Bypass -File "$updateScript" -Message "post-commit auto-sync" -Quiet
"@
        Set-Content -Path $hookPath -Value $hookScript -NoNewline
        Write-Host "Installed git post-commit hook: $hookPath"
    } else {
        Write-Host "No .git folder found; skipped git hook setup."
    }
}

& $updateScript -Message "initial auto-update installation"

Write-Host ""
Write-Host "Auto-update installed."
Write-Host "Optional live watcher:"
Write-Host "  powershell -ExecutionPolicy Bypass -File `"$repoRoot\scripts\start_memory_watcher.ps1`""
