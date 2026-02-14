# Memory Index

## What Each File Stores
- `00_PROJECT_OVERVIEW.md`: mission, scope, constraints, non-goals.
- `01_ARCHITECTURE.md`: stack, data flow, folder structure, dependencies.
- `02_DECISIONS_LOG.md`: dated architectural/process decisions.
- `03_FEATURE_REGISTRY.md`: feature status/dependencies/risks.
- `04_KNOWN_BUGS.md`: active and resolved issues.
- `05_ROADMAP.md`: short/mid/long-term plan.
- `06_ENVIRONMENT_SETUP.md`: build/install prerequisites and commands.
- `07_API_SCHEMA.md`: plugin interface contract and parameter IDs.
- `08_UI_SYSTEM.md`: UI architecture notes.
- `09_DEPLOYMENT.md`: artifact/install/deploy checklist.
- `10_PROMPT_HISTORY.md`: major prompt categories and outcomes.
- `sessions/`: dated working session logs.
- `versions/`: milestone snapshots.

## How to Reference
- Use targeted references in new chats:
  - `@.memory/00_PROJECT_OVERVIEW.md`
  - `@.memory/01_ARCHITECTURE.md`
  - `@.memory/02_DECISIONS_LOG.md`
  - `@.memory/sessions/session_YYYY_MM_DD.md`

## Update Protocol
1. Before coding, read:
   - `01_ARCHITECTURE.md`
   - `02_DECISIONS_LOG.md`
2. After significant change, update:
   - `02_DECISIONS_LOG.md`
   - `03_FEATURE_REGISTRY.md`
   - relevant technical files (architecture/setup/deployment)
   - new `sessions/session_YYYY_MM_DD.md`
3. On milestone:
   - create/update `versions/vX.X.md`

## Auto-Update Commands
- Install automation:
  - `powershell -ExecutionPolicy Bypass -File .\scripts\install_memory_autoupdate.ps1`
- Manual trigger:
  - `powershell -ExecutionPolicy Bypass -File .\scripts\update_memory.ps1 -Message "manual sync"`
- Live watcher:
  - `powershell -ExecutionPolicy Bypass -File .\scripts\start_memory_watcher.ps1`

## Anti-Drift Checklist
- Confirm proposed change against architecture and decisions.
- Flag conflicts explicitly before implementing.
- Do not silently redesign core architecture.

Last Auto Update: 2026-02-14T05:17:34Z






