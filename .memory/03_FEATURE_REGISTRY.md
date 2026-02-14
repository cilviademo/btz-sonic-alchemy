# Feature Registry

## Reliable Windows x64 Build Script
- Status: Complete
- Dependencies: VS 2022 x64 toolchain, CMake, Ninja
- Owner: User + AI assistant
- Risks: Hardcoded VS/JUCE paths may differ on another machine
- Notes: Implemented in `scripts/build_windows_with_vs_env.bat`

## One-Step Build + Install Script
- Status: Complete
- Dependencies: Build script + install script
- Owner: User + AI assistant
- Risks: Requires admin rights for install destination
- Notes: Implemented in `scripts/build_and_install_windows_vst3.bat`

## Windows VST3 Installer Robustness
- Status: Complete
- Dependencies: Built bundle under `BTZ/build/...`
- Owner: User + AI assistant
- Risks: Destination permissions, future output path changes
- Notes: Uses `robocopy` for directory bundle copy

## Persistent Memory System
- Status: Complete (initial scaffolding)
- Dependencies: `.memory/` folder + update discipline
- Owner: User + AI assistant
- Risks: Drift if logs not updated after major changes
- Notes: Initialized via this session

## Memory Auto-Update Engine
- Status: Complete
- Dependencies: PowerShell scripts in `scripts/`
- Owner: User + AI assistant
- Risks: Auto entries may include TODO placeholders needing review
- Notes: `install_memory_autoupdate.ps1`, `update_memory.ps1`, `start_memory_watcher.ps1`

## DSP Warning Cleanup (C4244)
- Status: Planned
- Dependencies: `TapeEmulator.h`, `GranularProcessor.h`
- Owner: TODO
- Risks: Minor type conversion warnings currently non-blocking
- Notes: Add explicit casts to silence warnings safely

## TODO
- [TODO] Add owner names/roles per feature.
- [TODO] Add acceptance criteria for each feature.

## Auto Sync Checkpoint (2026-02-13 22:01:31)
- Status: In Progress
- Dependencies: .memory update scripts
- Owner: User + AI assistant
- Risks: Auto summaries require human review for decision quality
- Notes: 239 file(s) changed since last sync.

## Auto Sync Checkpoint (2026-02-13 22:06:24)
- Status: In Progress
- Dependencies: .memory update scripts
- Owner: User + AI assistant
- Risks: Auto summaries require human review for decision quality
- Notes: 8 file(s) changed since last sync.

## Auto Sync Checkpoint (2026-02-13 22:18:33)
- Status: In Progress
- Dependencies: .memory update scripts
- Owner: User + AI assistant
- Risks: Auto summaries require human review for decision quality
- Notes: 10 file(s) changed since last sync.

## Auto Sync Checkpoint (2026-02-13 22:40:23)
- Status: In Progress
- Dependencies: .memory update scripts
- Owner: User + AI assistant
- Risks: Auto summaries require human review for decision quality
- Notes: 16 file(s) changed since last sync.

## Auto Sync Checkpoint (2026-02-13 23:05:38)
- Status: In Progress
- Dependencies: .memory update scripts
- Owner: User + AI assistant
- Risks: Auto summaries require human review for decision quality
- Notes: 159 file(s) changed since last sync.

## Auto Sync Checkpoint (2026-02-13 23:17:34)
- Status: In Progress
- Dependencies: .memory update scripts
- Owner: User + AI assistant
- Risks: Auto summaries require human review for decision quality
- Notes: 157 file(s) changed since last sync.
