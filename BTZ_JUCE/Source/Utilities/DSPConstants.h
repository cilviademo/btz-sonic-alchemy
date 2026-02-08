/*
  DSPConstants.h
  BTZ - Centralized DSP constants for maintainability

  P2-3 FIX: Extract magic numbers to named constants
*/

#pragma once

namespace BTZConstants
{
    //=========================================================================
    // PARAMETER SMOOTHING
    //=========================================================================

    // Smoothing ramp times (in seconds)
    constexpr double parameterSmoothingTime = 0.02;      // 20ms for main parameters
    constexpr double gainSmoothingTime = 0.05;           // 50ms for gain changes (smoother)

    // Sub-block processing for improved smoothing at low buffer sizes
    constexpr int smoothingSubBlockSize = 16;             // Process in 16-sample chunks

    //=========================================================================
    // LATENCY
    //=========================================================================

    // SparkLimiter lookahead buffer size (must match SparkLimiter.h)
    constexpr int sparkLimiterLookahead = 64;            // Samples

    //=========================================================================
    // SILENCE DETECTION
    //=========================================================================

    // Silence threshold (-60dB)
    constexpr float silenceThreshold = 0.001f;            // ~-60dB
    constexpr int maxSilentBuffersBeforeSkip = 10;        // Skip processing after 10 silent buffers

    //=========================================================================
    // DSP VALIDATION
    //=========================================================================

    // DC offset threshold (audible as "thump" when bypassed)
    constexpr float dcOffsetThreshold = 0.01f;            // Maximum acceptable DC

    // Sample validity range
    constexpr float maxValidSample = 100.0f;              // Safety limit for runaway samples

    //=========================================================================
    // METERING
    //=========================================================================

    // LUFS accumulator sample count before update
    constexpr int lufsSampleCountThreshold = 100;         // Update LUFS every 100 samples

    // K-weighting offset for LUFS approximation (ITU-R BS.1770-4)
    constexpr float lufsKWeightingOffset = -23.0f;        // dB

    // Default metering values
    constexpr float defaultLUFS = -14.0f;                 // Broadcast standard
    constexpr float defaultPeak = -6.0f;                  // Headroom for mixing
    constexpr float minMeteringLevel = -60.0f;            // Floor for dB conversion

} // namespace BTZConstants
