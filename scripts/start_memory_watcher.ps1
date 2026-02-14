param(
    [int]$DebounceSeconds = 4
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$memoryRoot = Join-Path $repoRoot ".memory"
$updateScript = Join-Path $PSScriptRoot "update_memory.ps1"

if (-not (Test-Path $updateScript)) {
    throw "Missing update script: $updateScript"
}

$watcher = New-Object System.IO.FileSystemWatcher
$watcher.Path = $repoRoot
$watcher.IncludeSubdirectories = $true
$watcher.Filter = "*"
$watcher.NotifyFilter = [IO.NotifyFilters]'FileName, LastWrite, DirectoryName, CreationTime'
$watcher.EnableRaisingEvents = $true

$script:pending = $false
$script:lastSignalUtc = [DateTime]::UtcNow

$isIgnored = {
    param([string]$fullPath)
    if ([string]::IsNullOrWhiteSpace($fullPath)) { return $true }
    $normalized = $fullPath.Replace('/', '\')
    return (
        $normalized.StartsWith((Join-Path $repoRoot ".git"), [StringComparison]::OrdinalIgnoreCase) -or
        $normalized.StartsWith((Join-Path $repoRoot "node_modules"), [StringComparison]::OrdinalIgnoreCase) -or
        $normalized.StartsWith((Join-Path $repoRoot "build"), [StringComparison]::OrdinalIgnoreCase) -or
        $normalized.StartsWith((Join-Path $memoryRoot ".state"), [StringComparison]::OrdinalIgnoreCase)
    )
}

$handler = {
    param($sender, $eventArgs)
    if (-not (& $isIgnored $eventArgs.FullPath)) {
        $script:pending = $true
        $script:lastSignalUtc = [DateTime]::UtcNow
    }
}

Register-ObjectEvent $watcher Changed -Action $handler | Out-Null
Register-ObjectEvent $watcher Created -Action $handler | Out-Null
Register-ObjectEvent $watcher Renamed -Action $handler | Out-Null
Register-ObjectEvent $watcher Deleted -Action $handler | Out-Null

Write-Host "Memory watcher running for: $repoRoot"
Write-Host "Debounce: $DebounceSeconds seconds"
Write-Host "Press Ctrl+C to stop."

try {
    while ($true) {
        Start-Sleep -Seconds 1
        if ($script:pending) {
            $elapsed = ([DateTime]::UtcNow - $script:lastSignalUtc).TotalSeconds
            if ($elapsed -ge $DebounceSeconds) {
                $script:pending = $false
                & $updateScript -Message "Auto watcher sync" -Quiet
                Write-Host "Auto memory sync completed at $((Get-Date).ToString("HH:mm:ss"))"
            }
        }
    }
} finally {
    Get-EventSubscriber | Where-Object { $_.SourceObject -eq $watcher } | Unregister-Event
    $watcher.Dispose()
}
