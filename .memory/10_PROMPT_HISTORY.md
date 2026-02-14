# Prompt History

## Build/Toolchain Debug
- Prompt type: Build failure troubleshooting
- Focus:
  - CMake/JUCE setup
  - x86/x64 linker conflicts
  - Windows install scripting
- Outcome:
  - Stable x64 script-driven build/install flow

## Architecture/Build System Refactor
- Prompt type: Convert to proper JUCE plugin CMake flow
- Focus:
  - `juce_add_plugin`
  - `createPluginFilter()`
  - output/install path alignment
- Outcome:
  - Valid VST3 bundle generated and installed

## Memory System Initialization
- Prompt type: Persistent context architecture
- Focus:
  - `.memory` scaffolding
  - anti-drift documentation
  - reusable resume command/rules
- Outcome:
  - Initial long-term project memory system created

## TODO
- [TODO] Append exact prompt excerpts for major future migrations.

## Auto Memory Sync - 2026-02-14T04:01:31Z
- Trigger: scripts/update_memory.ps1
- Message: initial auto-update installation
- Changed files: 239

## Auto Memory Sync - 2026-02-14T04:06:24Z
- Trigger: scripts/update_memory.ps1
- Message: Added functional JUCE editor controls for core parameters
- Changed files: 8

## Auto Memory Sync - 2026-02-14T04:18:33Z
- Trigger: scripts/update_memory.ps1
- Message: Integrated JUCE WebBrowserComponent UI path for React dist assets
- Changed files: 10

## Auto Memory Sync - 2026-02-14T04:40:23Z
- Trigger: scripts/update_memory.ps1
- Message: Applied BTZ-JUCE-Rewrite zip, rebuilt successfully; install blocked by file lock
- Changed files: 16

## Auto Memory Sync - 2026-02-14T05:05:38Z
- Trigger: scripts/update_memory.ps1
- Message: post-change sync
- Changed files: 159

## Auto Memory Sync - 2026-02-14T05:17:34Z
- Trigger: scripts/update_memory.ps1
- Message: post-change sync
- Changed files: 157
