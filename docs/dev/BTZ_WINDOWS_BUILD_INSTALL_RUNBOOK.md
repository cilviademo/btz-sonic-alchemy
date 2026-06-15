# BTZ — Windows Build → Install → Reaper Runbook

> **Purpose:** Repeatable path from `git pull` to a working VST3 in Reaper on Marc's Windows box.
> **Verified baseline:** `main` @ `31f8029` (BTZ Sonic Alchemy v1.0.4 squash)
> **Last validated:** 2026-06-15 — 86/86 unit tests, VST3 installed, Reaper-ready

> **Field origin:** captured by Cursor during the actual v1.0.4 install on Marc's
> workstation. This runbook reflects **what actually happened**, including the
> failure modes that had to be worked around. Two repo fixes followed from it
> and ship in the same commit: `scripts/install_vst3_windows.bat` no longer
> trips on `(BTZ)` parentheses, and `scripts/build_windows_with_vs_env.bat`
> now finds VS 2026 (18) in addition to VS 2022.

---

## One-page happy path

```cmd
cd C:\Users\marcm\OneDrive\Desktop\BTZ\btz-sonic-alchemy
git fetch origin
git checkout main
git pull --ff-only

:: One-time: JUCE junction (skip if C:\JUCE-8.0.6 already exists)
mklink /J "C:\JUCE-8.0.6" "C:\Users\marcm\OneDrive\Desktop\DESKTOP (2026)\JUCE-8.0.6"

set JUCE_DIR=C:\JUCE-8.0.6

:: Build + test (preferred on this machine — see § VS version below)
pwsh scripts\build_windows.ps1 -Config Release -Tests

:: Install (close Reaper first — see § Install pitfalls)
scripts\install_vst3_windows.bat

:: Reaper: Options → Preferences → Plug-ins → VST → Re-scan
```

---

## What lives where

| Artifact | Path |
|---|---|
| **Repo root** | `C:\Users\marcm\OneDrive\Desktop\BTZ\btz-sonic-alchemy` |
| **CMake project** | `btz-sonic-alchemy-main\BTZ\` |
| **Built VST3 bundle** (folder, not a single file) | `btz-sonic-alchemy-main\BTZ\build\BTZ_artefacts\Release\VST3\Box Tone Zone (BTZ).vst3` |
| **Plugin DLL inside bundle** | `...\Box Tone Zone (BTZ).vst3\Contents\x86_64-win\Box Tone Zone (BTZ).vst3` (~4.2 MB) |
| **Installed system VST3** | `C:\Program Files\Common Files\VST3\Box Tone Zone (BTZ).vst3` |
| **Standalone exe** | `btz-sonic-alchemy-main\BTZ\build\BTZ_artefacts\Release\Standalone\Box Tone Zone (BTZ).exe` |

On Windows, `.vst3` is always a **directory bundle**. Reaper scans the bundle folder under `Common Files\VST3\`.

---

## Prerequisites checklist

- [ ] Git repo cloned / synced to `main`
- [ ] **JUCE 8.0.6** reachable via junction `C:\JUCE-8.0.6` (or set `JUCE_DIR` to your copy)
- [ ] **Visual Studio** with Desktop C++ workload and x64 toolchain
- [ ] **CMake** on PATH (`C:\Program Files\CMake\bin\cmake.exe`)
- [ ] **Ninja** on PATH (optional but used by `build_windows_with_vs_env.bat`)
- [ ] **Reaper** installed for DAW smoke test

---

## Step 1 — Sync source

```cmd
git fetch origin
git checkout main
git pull --ff-only
git log -1 --oneline
```

Expected at v1.0.4 baseline:
`31f8029 BTZ Sonic Alchemy v1.0.4 — production baseline (squash of 54 commits)`

Optional tag (if not on remote yet):

```cmd
git tag v1.0.4 31f8029
git push origin v1.0.4
```

---

## Step 2 — JUCE junction (one-time)

The repo lives under OneDrive; JUCE should **not** be inside the synced tree. Use a junction:

```cmd
mklink /J "C:\JUCE-8.0.6" "C:\Users\marcm\OneDrive\Desktop\DESKTOP (2026)\JUCE-8.0.6"
set JUCE_DIR=C:\JUCE-8.0.6
```

Verify:

```cmd
dir C:\JUCE-8.0.6\modules\juce_core
```

---

## Step 3 — Build

### Option A — Recommended on Marc's machine (VS 2026 / VS 18)

This machine has **VS 2026 (18) Community** at:

```
C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat
```

`scripts\build_windows_with_vs_env.bat` **now finds VS 18 automatically** (as of v1.0.4) — but the PowerShell script is the preferred path:

```powershell
$env:JUCE_DIR = "C:\JUCE-8.0.6"
pwsh scripts\build_windows.ps1 -Config Release -Tests
```

**Verified result:** configure OK, full Release build OK, **86/86 tests passed**, VST3 bundle + `moduleinfo.json` generated.

### Option B — Manual Ninja build (same outcome as A)

```cmd
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
set JUCE_DIR=C:\JUCE-8.0.6
set BTZ_DIR=%CD%\btz-sonic-alchemy-main\BTZ

if exist "%BTZ_DIR%\build" rd /s /q "%BTZ_DIR%\build"

cmake -S "%BTZ_DIR%" -B "%BTZ_DIR%\build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DBTZ_BUILD_TESTS=ON
cmake --build "%BTZ_DIR%\build" -j 8
ctest --test-dir "%BTZ_DIR%\build" --output-on-failure
```

### Option C — `build_windows_with_vs_env.bat` (VS 18 or VS 2022)

```cmd
set JUCE_DIR=C:\JUCE-8.0.6
scripts\build_windows_with_vs_env.bat
```

> The batch now prefers VS 2026 (18) Community / Professional / BuildTools, then
> falls back to VS 2022 — patched in v1.0.4 from this runbook's field findings.

### Build success criteria

- [ ] No CMake/configure errors
- [ ] `Box Tone Zone (BTZ).vst3` bundle exists under `BTZ_artefacts\Release\VST3\`
- [ ] Bundle contains `Contents\x86_64-win\Box Tone Zone (BTZ).vst3` (~4 MB)
- [ ] Bundle contains `Contents\Resources\moduleinfo.json`
- [ ] (If `-Tests`) ctest reports **86/86 passed**

Warnings about deprecated `juce::Font` constructors are expected and non-fatal.

---

## Step 4 — Install VST3 to system folder

### Before you install

1. **Close Reaper** (and any other DAW). If the old plugin is loaded, the DLL in `Program Files\Common Files\VST3\` is locked → robocopy **ERROR 32**.
2. Run install from an **elevated** cmd if the destination folder is not writable.

### Normal install

```cmd
scripts\install_vst3_windows.bat
```

Copies build output → `C:\Program Files\Common Files\VST3\Box Tone Zone (BTZ).vst3`

### Install pitfalls we hit (and fixes)

| Symptom | Cause | Fix |
|---|---|---|
| `.vst3 was unexpected at this time.` | Batch `if exist "%VST3_SRC%\."` breaks when path contains `(BTZ)` — cmd treats parens as block syntax | **Fixed in repo (v1.0.4):** uses `if not exist "%VST3_SRC%\"` instead |
| `ERROR 32 … file is being used by another process` on copy to Program Files | **Reaper was running** with the old plugin loaded | Close Reaper, delete old bundle, reinstall |
| Robocopy hangs retrying forever | Default robocopy retry count is huge | **Fixed in repo (v1.0.4):** install script uses `/R:2 /W:2`. Close locking app first. |
| OneDrive on repo path | Source under `OneDrive\Desktop\…` can occasionally lock files during sync | Stage to `%TEMP%` first, then copy to Program Files (see below) |

### Reliable install when something is flaky (staging pattern)

```powershell
$src  = "$PWD\btz-sonic-alchemy-main\BTZ\build\BTZ_artefacts\Release\VST3\Box Tone Zone (BTZ).vst3"
$stage = "$env:TEMP\BTZ_vst3_stage"
$dest = "C:\Program Files\Common Files\VST3\Box Tone Zone (BTZ).vst3"

# Close Reaper first!
Stop-Process -Name reaper -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 2

if (Test-Path $stage) { Remove-Item -Recurse -Force $stage }
robocopy $src $stage /E /R:2 /W:2

if (Test-Path $dest) { Remove-Item -Recurse -Force $dest }
robocopy $stage $dest /E /R:2 /W:2
```

Verify:

```powershell
Get-ChildItem -LiteralPath 'C:\Program Files\Common Files\VST3\Box Tone Zone (BTZ).vst3' -Recurse -File |
  Format-Table Name, Length, LastWriteTime
```

Expect ~4.2 MB DLL dated from today's build.

---

## Step 5 — Reaper validation

1. Open Reaper
2. `Options → Preferences → Plug-ins → VST → Re-scan`
3. Insert **Box Tone Zone (BTZ)** by **BTZ Audio**

### Quick sanity checklist

| Check | Expected |
|---|---|
| Plugin loads | No crash |
| Default patch | Audible warm glue (not silent/transparent) |
| **Mix** = 0.0 | Output ≈ dry input |
| **Mix** = 0.5 | 50/50 wet/dry |
| **Drive** ~0.7 + sine | Saturation audible; auto-gain compensates level |
| **Bypass** | Click-free fade to dry |
| **Master** sweep | Audible level change |
| Target LUFS **−14** + **LOCK** | Footer shows lock state |
| Footer **TRUE PEAK / LUFS** | Real dBTP + K-weighted LUFS |
| Saturation menu | 5 analog + 2 WDF; 4 Neural greyed out |
| Multiband | Only "Full Range" selectable |

---

## Troubleshooting quick reference

### Build fails: "VS 2022 vcvars64.bat not found"

→ Pre-v1.0.4 script. Either pull v1.0.4 (the batch now finds VS 18) or use
  `pwsh scripts\build_windows.ps1 -Config Release -Tests`, or call VS 18
  `vcvars64.bat` manually (§ Step 3).

### Build fails: JUCE not found

→ Set `JUCE_DIR=C:\JUCE-8.0.6` and confirm junction exists.

### Install fails: ERROR 32

→ Close Reaper / DAW. If still locked, reboot or use staging pattern (§ Step 4).

### Reaper shows old plugin behaviour after install

→ Full rescan; if needed remove `%APPDATA%\REAPER\reaper-vstplugins64.ini` entry and rescan (last resort).

### Linux CI note (not Windows)

Headless Linux `juce_vst3_helper` can fail on `moduleinfo.json` when path contains parentheses (`dash` parsing). **Does not affect Windows or macOS.** The `.so` still links.

---

## Rollback

```cmd
git checkout ee185f9
:: only if remote must revert:
git push origin main --force-with-lease
```

Unsquashed 54-commit history preserved on `audit/v1.0-baseline` and `claude/review-btz-compilation-aN5UD`.

---

## Session log — what had to be done for v1.0.4 on this box

1. Confirmed `main` @ `31f8029` matched `origin/main`
2. Confirmed `C:\JUCE-8.0.6` junction present
3. **`build_windows_with_vs_env.bat` failed** — no VS 2022; VS 2026 (18) present instead → fixed in v1.0.4
4. Built with VS 18 + Ninja + `-DBTZ_BUILD_TESTS=ON` → **86/86 tests pass**
5. VST3 bundle produced at `btz-sonic-alchemy-main\BTZ\build\BTZ_artefacts\Release\VST3\`
6. **`install_vst3_windows.bat` failed** — parentheses in bundle name broke batch `if exist` → fixed in v1.0.4
7. **Install robocopy failed with ERROR 32** — Reaper had old plugin loaded; closed Reaper, removed old bundle, staged via `%TEMP%`, copied fresh bundle
8. Verified installed DLL ~4.2 MB with today's timestamp

---

## Script follow-ups still recommended (not in v1.0.4)

- [ ] Add Reaper pre-check to `install_vst3_windows.bat` ("close Reaper if running")
- [ ] Optional: `%TEMP%` staging in install script when source is under OneDrive
- [ ] Update `docs/Build.md` and `docs/dev/BTZ_LOCAL_DEV_SETUP.md` to cross-link this runbook as the canonical Windows path
