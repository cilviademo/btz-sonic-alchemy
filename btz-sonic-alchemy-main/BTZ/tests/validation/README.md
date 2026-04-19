# BTZ Sonic Alchemy — Compiled-Build Validation Scripts

These scripts require a compiled plugin binary. They cannot be run in a sandbox
environment without a C++ toolchain and JUCE installed.

## Prerequisites

```bash
# Build the plugin first
cd BTZ && mkdir build && cd build
cmake .. -DBTZ_BUILD_TESTS=ON
cmake --build . --config Release
```

## Test Matrix

| # | Test | Script | Pass Criteria |
|---|------|--------|---------------|
| 1 | ISP Overshoot | `01_isp_torture.py` | Max ISP ≤ +0.2 dBTP above ceiling |
| 2 | Null-Path | `02_null_path.py` | Output − Input < −120 dB over 10s pink noise |
| 3 | Bypass Click | `03_bypass_click.py` | Max sample-to-sample delta < 0.01 during bypass toggle |
| 4 | Silence-In-Silence-Out | `04_silence_test.py` | Output peak < −120 dB when input is silence |
| 5 | State Migration | `05_state_migration.py` | v4/v5/v6 presets load without crash, params in valid range |
| 6 | CPU Benchmark | `06_cpu_benchmark.py` | < 15% single-core at 4x OS, 96 kHz, 512 samples |
| 7 | pluginval | CMake target | `validate_vst3` passes at strictness 10 |
| 8 | auvaltool | CMake target | `validate_auvaltool` passes (macOS only) |
| 9 | Host Matrix | `09_host_matrix.md` | Manual checklist for DAW compatibility |

## Running

```bash
# After building, run Python validation scripts:
cd tests/validation
python3 01_isp_torture.py --plugin ../build/BTZ_artefacts/Release/VST3/BTZ.vst3

# Or use CMake validator targets:
cd build
PLUGINVAL_PATH=/usr/local/bin/pluginval cmake --build . --target validate_vst3
```
