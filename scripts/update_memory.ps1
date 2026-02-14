param(
    [string]$Message = "",
    [switch]$Quiet
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$memoryRoot = Join-Path $repoRoot ".memory"
$stateDir = Join-Path $memoryRoot ".state"
$sessionsDir = Join-Path $memoryRoot "sessions"

New-Item -ItemType Directory -Force -Path $stateDir | Out-Null
New-Item -ItemType Directory -Force -Path $sessionsDir | Out-Null

$lastRunFile = Join-Path $stateDir "last_run_utc.txt"
$nowUtc = (Get-Date).ToUniversalTime()
$nowIso = $nowUtc.ToString("yyyy-MM-ddTHH:mm:ssZ")
$today = (Get-Date).ToString("yyyy_MM_dd")
$sessionFile = Join-Path $sessionsDir ("session_{0}.md" -f $today)

if (Test-Path $lastRunFile) {
    $lastRunUtc = [DateTime]::Parse((Get-Content $lastRunFile -Raw).Trim()).ToUniversalTime()
} else {
    $lastRunUtc = [DateTime]::UtcNow.AddDays(-7)
}

$excludePattern = "\\(\.git|node_modules|build|\.memory\\\.state)(\\|$)"

$changed = Get-ChildItem -Path $repoRoot -Recurse -File -Force |
    Where-Object {
        $_.LastWriteTimeUtc -gt $lastRunUtc -and
        $_.FullName -notmatch $excludePattern
    } |
    Sort-Object LastWriteTimeUtc

$relativeChanged = @()
foreach ($item in $changed) {
    $relativeChanged += $item.FullName.Substring($repoRoot.Length).TrimStart('\', '/').Replace('\', '/')
}

if (-not (Test-Path $sessionFile)) {
    @"
# Session $((Get-Date).ToString("yyyy-MM-dd"))

## Context given
- [TODO] Fill context summary.

## Work completed
- [TODO] Fill completed work summary.

## Files modified
- [TODO] Filled automatically below on updates.

## Open questions
- [TODO] List open questions.

## Next steps
- [TODO] List next actionable steps.
"@ | Set-Content -Path $sessionFile
}

$block = @()
$block += ""
$block += "### Auto Update $nowIso"
if ($Message) { $block += "- Note: $Message" }
$block += "- Changed files detected: $($relativeChanged.Count)"
if ($relativeChanged.Count -gt 0) {
    $limit = [Math]::Min(50, $relativeChanged.Count)
    for ($i = 0; $i -lt $limit; $i++) {
        $block += "  - $($relativeChanged[$i])"
    }
    if ($relativeChanged.Count -gt $limit) {
        $block += "  - ... and $($relativeChanged.Count - $limit) more"
    }
}
Add-Content -Path $sessionFile -Value ($block -join [Environment]::NewLine)

$decisionsFile = Join-Path $memoryRoot "02_DECISIONS_LOG.md"
if ((Test-Path $decisionsFile) -and $relativeChanged.Count -gt 0) {
    @"

## $((Get-Date).ToString("yyyy-MM-dd")) [Auto Update]
Decision:
- [TODO] Confirm whether architectural/process decision is required for this change set.

Reason:
- Auto-detected file changes since last memory sync.

Tradeoffs:
- [TODO] Add tradeoff notes if applicable.

Impact:
- $($relativeChanged.Count) file(s) changed since last sync.

Reversal Strategy:
- Revert changes and update session/feature logs accordingly.
"@ | Add-Content -Path $decisionsFile
}

$featuresFile = Join-Path $memoryRoot "03_FEATURE_REGISTRY.md"
if ((Test-Path $featuresFile) -and $relativeChanged.Count -gt 0) {
    @"

## Auto Sync Checkpoint ($((Get-Date).ToString("yyyy-MM-dd HH:mm:ss")))
- Status: In Progress
- Dependencies: `.memory` update scripts
- Owner: User + AI assistant
- Risks: Auto summaries require human review for decision quality
- Notes: $($relativeChanged.Count) file(s) changed since last sync.
"@ | Add-Content -Path $featuresFile
}

$promptHistory = Join-Path $memoryRoot "10_PROMPT_HISTORY.md"
if (Test-Path $promptHistory) {
    $messageValue = "N/A"
    if (-not [string]::IsNullOrWhiteSpace($Message)) {
        $messageValue = $Message
    }
    @"

## Auto Memory Sync - $nowIso
- Trigger: `scripts/update_memory.ps1`
- Message: $messageValue
- Changed files: $($relativeChanged.Count)
"@ | Add-Content -Path $promptHistory
}

$indexFile = Join-Path $memoryRoot "memory_index.md"
if (Test-Path $indexFile) {
    $indexRaw = Get-Content -Path $indexFile -Raw
    if ($indexRaw -match "Last Auto Update:") {
        $indexRaw = [Regex]::Replace($indexRaw, "Last Auto Update:\s*.*", "Last Auto Update: $nowIso")
    } else {
        $indexRaw += [Environment]::NewLine + [Environment]::NewLine + "Last Auto Update: $nowIso"
    }
    Set-Content -Path $indexFile -Value $indexRaw
}

$nowIso | Set-Content -Path $lastRunFile

if (-not $Quiet) {
    Write-Host "Memory updated. Changed files: $($relativeChanged.Count)"
    Write-Host "Session file: $sessionFile"
}
