#!/bin/bash

#==============================================================================
# BTZ Comprehensive Benchmark Suite Runner
#
# Purpose: Execute all performance benchmarks and generate combined report
#
# Benchmarks:
#   1. CPU Benchmark - Multi-instance processing load
#   2. Load Time Benchmark - Plugin initialization time
#   3. Automation Spike Benchmark - Parameter change CPU spikes
#
# Usage:
#   ./run_all_benchmarks.sh [--output-dir PATH]
#==============================================================================

set -e

# Default configuration
OUTPUT_DIR="artifacts/perf"
TIMESTAMP=$(date +"%Y-%m-%d_%H-%M-%S")
BENCHMARK_DIR="tools/benchmark/build"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --output-dir)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        --help)
            echo "BTZ Comprehensive Benchmark Suite Runner"
            echo ""
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --output-dir PATH   Output directory for results (default: artifacts/perf)"
            echo "  --help              Show this help"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

echo -e "${BLUE}========================================"
echo "BTZ Comprehensive Benchmark Suite"
echo "========================================${NC}"
echo "Timestamp: $TIMESTAMP"
echo "Output Directory: $OUTPUT_DIR"
echo ""

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Build benchmarks if needed
if [ ! -f "$BENCHMARK_DIR/cpu_benchmark" ] || \
   [ ! -f "$BENCHMARK_DIR/load_time_benchmark" ] || \
   [ ! -f "$BENCHMARK_DIR/automation_spike_benchmark" ]; then
    echo -e "${YELLOW}Building benchmark executables...${NC}"

    cd tools/benchmark
    rm -rf build
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release --parallel 4
    cd ../..

    echo -e "${GREEN}✓ Benchmarks built${NC}"
    echo ""
fi

# Check all executables exist
for bench in cpu_benchmark load_time_benchmark automation_spike_benchmark; do
    if [ ! -f "$BENCHMARK_DIR/$bench" ]; then
        echo -e "${RED}ERROR: $bench not found${NC}"
        exit 1
    fi
done

# Results tracking
CPU_STATUS="PENDING"
LOAD_STATUS="PENDING"
AUTOMATION_STATUS="PENDING"

CPU_RESULT=""
LOAD_RESULT=""
AUTOMATION_RESULT=""

#==============================================================================
# Benchmark 1: CPU Performance
#==============================================================================
echo -e "${BLUE}[1/3] Running CPU Benchmark...${NC}"
CPU_OUTPUT="$OUTPUT_DIR/cpu_benchmark_$TIMESTAMP.json"

if ./"$BENCHMARK_DIR/cpu_benchmark" \
    --instances 10 \
    --sample-rate 48000 \
    --buffer-size 128 \
    --buffers 5000 \
    --output "$CPU_OUTPUT"; then

    CPU_STATUS="PASS"
    CPU_RESULT=$(grep -o '"avg_cpu_percent": [0-9.]*' "$CPU_OUTPUT" | grep -o '[0-9.]*$')
    echo -e "${GREEN}✅ CPU Benchmark PASS${NC} (${CPU_RESULT}% avg)"
else
    CPU_STATUS="FAIL"
    echo -e "${RED}❌ CPU Benchmark FAIL${NC}"
fi
echo ""

#==============================================================================
# Benchmark 2: Load Time
#==============================================================================
echo -e "${BLUE}[2/3] Running Load Time Benchmark...${NC}"
LOAD_OUTPUT="$OUTPUT_DIR/load_time_benchmark_$TIMESTAMP.json"

if ./"$BENCHMARK_DIR/load_time_benchmark" \
    --iterations 50 \
    --sample-rate 48000 \
    --buffer-size 512 \
    --output "$LOAD_OUTPUT"; then

    LOAD_STATUS="PASS"
    LOAD_RESULT=$(grep -o '"avg_ms": [0-9.]*' "$LOAD_OUTPUT" | head -1 | grep -o '[0-9.]*$')
    echo -e "${GREEN}✅ Load Time Benchmark PASS${NC} (${LOAD_RESULT}ms avg)"
else
    LOAD_STATUS="FAIL"
    echo -e "${RED}❌ Load Time Benchmark FAIL${NC}"
fi
echo ""

#==============================================================================
# Benchmark 3: Automation Spikes
#==============================================================================
echo -e "${BLUE}[3/3] Running Automation Spike Benchmark...${NC}"
AUTOMATION_OUTPUT="$OUTPUT_DIR/automation_spike_benchmark_$TIMESTAMP.json"

if ./"$BENCHMARK_DIR/automation_spike_benchmark" \
    --sample-rate 48000 \
    --buffer-size 128 \
    --output "$AUTOMATION_OUTPUT"; then

    AUTOMATION_STATUS="PASS"
    AUTOMATION_RESULT=$(grep -o '"spike_ratio": [0-9.]*' "$AUTOMATION_OUTPUT" | grep -o '[0-9.]*$')
    echo -e "${GREEN}✅ Automation Spike Benchmark PASS${NC} (${AUTOMATION_RESULT}x spike ratio)"
else
    AUTOMATION_STATUS="FAIL"
    echo -e "${RED}❌ Automation Spike Benchmark FAIL${NC}"
fi
echo ""

#==============================================================================
# Generate Combined Report
#==============================================================================
REPORT_FILE="$OUTPUT_DIR/benchmark_report_$TIMESTAMP.md"

cat > "$REPORT_FILE" <<EOF
# BTZ Performance Benchmark Report

**Date**: $(date +"%Y-%m-%d %H:%M:%S")
**Commit**: $(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
**Branch**: $(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown")

---

## Summary

| Benchmark | Status | Key Metric | Target | Result |
|-----------|--------|------------|--------|--------|
| CPU Performance | $CPU_STATUS | Avg CPU | < 60% | ${CPU_RESULT}% |
| Load Time | $LOAD_STATUS | Avg Load | < 200ms | ${LOAD_RESULT}ms |
| Automation Spikes | $AUTOMATION_STATUS | Spike Ratio | < 2.0x | ${AUTOMATION_RESULT}x |

---

## Detailed Results

### 1. CPU Performance Benchmark

**Configuration**:
- Instances: 10
- Sample Rate: 48 kHz
- Buffer Size: 128 samples
- Buffers Processed: 5000

**Results**:
- Average CPU: ${CPU_RESULT}%
- Status: **$CPU_STATUS**

**Ship Gate #4**: 10 instances @ 48kHz/128 < 60% CPU

---

### 2. Load Time Benchmark

**Configuration**:
- Iterations: 50
- Sample Rate: 48 kHz
- Buffer Size: 512 samples

**Results**:
- Average Load Time: ${LOAD_RESULT}ms
- Status: **$LOAD_STATUS**

**Performance Targets**:
- Typical load: < 200ms
- Worst-case load: < 500ms

---

### 3. Automation Spike Benchmark

**Configuration**:
- Sample Rate: 48 kHz
- Buffer Size: 128 samples
- Parameters Automated: 8 (punch, warmth, boom, drive, mix, gains, shine)

**Results**:
- Spike Ratio: ${AUTOMATION_RESULT}x
- Status: **$AUTOMATION_STATUS**

**Performance Target**: CPU spikes < 2x baseline during automation

---

## Artifacts

- CPU Benchmark: \`cpu_benchmark_$TIMESTAMP.json\`
- Load Time Benchmark: \`load_time_benchmark_$TIMESTAMP.json\`
- Automation Spike Benchmark: \`automation_spike_benchmark_$TIMESTAMP.json\`

---

## Overall Assessment

EOF

# Calculate overall status
OVERALL_STATUS="PASS"
if [ "$CPU_STATUS" != "PASS" ] || [ "$LOAD_STATUS" != "PASS" ] || [ "$AUTOMATION_STATUS" != "PASS" ]; then
    OVERALL_STATUS="FAIL"
fi

if [ "$OVERALL_STATUS" = "PASS" ]; then
    echo "**Overall Status**: ✅ **ALL BENCHMARKS PASS**" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    echo "BTZ meets all performance targets for ship readiness." >> "$REPORT_FILE"
else
    echo "**Overall Status**: ❌ **SOME BENCHMARKS FAILED**" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    echo "Performance optimization required before ship." >> "$REPORT_FILE"
fi

echo "---" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"
echo "**Generated by**: BTZ Benchmark Suite v1.0.0" >> "$REPORT_FILE"

#==============================================================================
# Summary Output
#==============================================================================
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Benchmark Suite Complete${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo "Results:"
echo "  CPU Benchmark: $CPU_STATUS ($CPU_RESULT%)"
echo "  Load Time Benchmark: $LOAD_STATUS (${LOAD_RESULT}ms)"
echo "  Automation Spike Benchmark: $AUTOMATION_STATUS (${AUTOMATION_RESULT}x)"
echo ""
echo "Report: $REPORT_FILE"
echo ""

if [ "$OVERALL_STATUS" = "PASS" ]; then
    echo -e "${GREEN}✅ OVERALL STATUS: ALL BENCHMARKS PASS${NC}"
    exit 0
else
    echo -e "${RED}❌ OVERALL STATUS: SOME BENCHMARKS FAILED${NC}"
    exit 1
fi
