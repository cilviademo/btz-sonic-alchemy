#!/bin/bash

#==============================================================================
# BTZ CPU Benchmark Runner
#
# Purpose: Ship Gate #4 - CPU performance validation
#
# Requirements: 10 instances @ 48kHz/128 samples < 60% CPU
#
# Usage:
#   ./run_benchmark.sh [--instances N] [--output PATH]
#==============================================================================

set -e

# Default configuration
INSTANCES=10
SAMPLE_RATE=48000
BUFFER_SIZE=128
BUFFERS=10000
OUTPUT_DIR="artifacts/benchmark"
OUTPUT_FILE="benchmark_results.json"

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --instances)
            INSTANCES="$2"
            shift 2
            ;;
        --sample-rate)
            SAMPLE_RATE="$2"
            shift 2
            ;;
        --buffer-size)
            BUFFER_SIZE="$2"
            shift 2
            ;;
        --buffers)
            BUFFERS="$2"
            shift 2
            ;;
        --output)
            OUTPUT_FILE="$2"
            shift 2
            ;;
        --help)
            echo "BTZ CPU Benchmark Runner"
            echo ""
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --instances N       Number of plugin instances (default: 10)"
            echo "  --sample-rate SR    Sample rate in Hz (default: 48000)"
            echo "  --buffer-size N     Buffer size in samples (default: 128)"
            echo "  --buffers N         Number of buffers to process (default: 10000)"
            echo "  --output PATH       Output JSON path (default: benchmark_results.json)"
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

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "========================================"
echo "BTZ CPU Benchmark Runner"
echo "========================================"

# Find or build benchmark executable
BENCHMARK_EXE=""
if [ -f "tools/benchmark/build/cpu_benchmark" ]; then
    BENCHMARK_EXE="tools/benchmark/build/cpu_benchmark"
    echo "✓ Found existing benchmark executable"
elif [ -f "tools/benchmark/cpu_benchmark" ]; then
    BENCHMARK_EXE="tools/benchmark/cpu_benchmark"
    echo "✓ Found existing benchmark executable"
else
    echo "Building benchmark executable..."

    # Check if JUCE is available
    if [ ! -d "BTZ_JUCE/JUCE" ]; then
        echo "ERROR: JUCE not found. Please run BTZ_JUCE/scripts/setup_juce.sh first."
        exit 1
    fi

    cd tools/benchmark
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release --parallel 4
    cd ../..

    BENCHMARK_EXE="tools/benchmark/build/cpu_benchmark"

    if [ ! -f "$BENCHMARK_EXE" ]; then
        echo "ERROR: Failed to build benchmark executable"
        exit 1
    fi

    echo "✓ Built benchmark executable"
fi

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Run benchmark
echo ""
echo "Running benchmark with:"
echo "  Instances: $INSTANCES"
echo "  Sample Rate: $SAMPLE_RATE Hz"
echo "  Buffer Size: $BUFFER_SIZE samples"
echo "  Buffers: $BUFFERS"
echo ""

OUTPUT_PATH="$OUTPUT_DIR/$OUTPUT_FILE"

if ./"$BENCHMARK_EXE" \
    --instances "$INSTANCES" \
    --sample-rate "$SAMPLE_RATE" \
    --buffer-size "$BUFFER_SIZE" \
    --buffers "$BUFFERS" \
    --output "$OUTPUT_PATH"; then

    echo ""
    echo -e "${GREEN}✅ Benchmark completed successfully${NC}"
    echo ""

    # Parse results
    if [ -f "$OUTPUT_PATH" ]; then
        AVG_CPU=$(grep -o '"avg_cpu_percent": [0-9.]*' "$OUTPUT_PATH" | grep -o '[0-9.]*$')
        STATUS=$(grep -o '"status": "[^"]*"' "$OUTPUT_PATH" | cut -d'"' -f4)

        echo "Results saved to: $OUTPUT_PATH"
        echo ""
        echo "Average CPU: ${AVG_CPU}%"

        if [ "$STATUS" = "PASS" ]; then
            echo -e "Ship Gate #4: ${GREEN}✅ PASS${NC}"
        else
            echo -e "Ship Gate #4: ${RED}❌ FAIL${NC}"
        fi
    fi

    exit 0
else
    echo ""
    echo -e "${RED}❌ Benchmark failed${NC}"
    exit 1
fi
