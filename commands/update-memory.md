# /update-memory

Run a manual memory sync after significant code changes.

## Command
- Windows PowerShell:
  - `powershell -ExecutionPolicy Bypass -File .\scripts\update_memory.ps1 -Message "manual sync"`

## What it updates
- `.memory/sessions/session_YYYY_MM_DD.md`
- `.memory/02_DECISIONS_LOG.md` (auto checkpoint entry)
- `.memory/03_FEATURE_REGISTRY.md` (auto checkpoint entry)
- `.memory/10_PROMPT_HISTORY.md`
- `.memory/memory_index.md` (`Last Auto Update`)

## Notes
- Auto-generated entries include TODO markers for human review.
- This command reduces context drift; it does not replace architectural judgment.
