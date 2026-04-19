# BTZ Sonic Alchemy — Host Compatibility Matrix

## Test Procedure

For each DAW, perform the following checks:

1. **Load** — Plugin loads without crash, UI appears correctly
2. **Bypass** — Host bypass toggles cleanly (no click)
3. **Automation** — Record and play back automation on all parameters
4. **State Recall** — Save project, close, reopen — all parameters restored
5. **Preset Save/Load** — Save and load VST3 presets
6. **CPU** — Monitor CPU usage during 60s playback at 4x OS
7. **Latency** — Verify latency compensation is correct (null test with bypass)

## Matrix

| DAW | Version | OS | VST3 | AU | Load | Bypass | Auto | State | Preset | CPU | Latency |
|-----|---------|-----|------|-----|------|--------|------|-------|--------|-----|---------|
| Ableton Live | 12.x | macOS | | | | | | | | | |
| Ableton Live | 12.x | Win | | | | | | | | | |
| Logic Pro | 11.x | macOS | N/A | | | | | | | | |
| Pro Tools | 2024.x | macOS | | N/A | | | | | | | |
| Pro Tools | 2024.x | Win | | N/A | | | | | | | |
| FL Studio | 24.x | Win | | N/A | | | | | | | |
| Cubase | 14.x | macOS | | | | | | | | | |
| Cubase | 14.x | Win | | | | | | | | | |
| Studio One | 7.x | macOS | | | | | | | | | |
| Studio One | 7.x | Win | | | | | | | | | |
| REAPER | 7.x | macOS | | | | | | | | | |
| REAPER | 7.x | Win | | | | | | | | | |
| Bitwig | 5.x | macOS | | | | | | | | | |
| Bitwig | 5.x | Win | | | | | | | | | |

## Grading

- **P** = Pass
- **F** = Fail (describe issue in notes)
- **N/A** = Not applicable (format not supported)
- **NT** = Not tested

## Notes

(Record any issues, workarounds, or observations here)
