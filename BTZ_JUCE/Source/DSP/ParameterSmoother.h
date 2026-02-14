/*
  ==============================================================================

  ParameterSmoother.h

  Lock-free parameter smoothing to prevent zipper noise

  Features:
  - One-pole lowpass smoothing
  - Configurable ramp time (default: 20ms)
  - Per-sample and per-block processing
  - Zero-allocation RT-safe operation

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <cmath>

template<typename FloatType>
class ParameterSmoother
{
public:
    ParameterSmoother() = default;

    /** Prepares the smoother with the given sample rate and ramp time.
        @param sampleRate The sample rate to use
        @param rampTimeSeconds The time in seconds to reach the target value
    */
    void prepare(double sampleRate, FloatType rampTimeSeconds = static_cast<FloatType>(0.02))
    {
        this->sampleRate = sampleRate;
        setRampTime(rampTimeSeconds);
        reset();
    }

    /** Sets the ramp time in seconds.
        @param rampTimeSeconds The time in seconds to reach ~99% of target value
    */
    void setRampTime(FloatType rampTimeSeconds)
    {
        jassert(sampleRate > 0.0);

        if (rampTimeSeconds <= static_cast<FloatType>(0))
            rampTimeSeconds = static_cast<FloatType>(0.001); // Minimum 1ms

        // One-pole filter coefficient for exponential smoothing
        // Formula: coeff = exp(-1 / (sampleRate * rampTime))
        // This gives ~63% of target in rampTime seconds
        coefficient = std::exp(static_cast<FloatType>(-1.0) /
                               (static_cast<FloatType>(sampleRate) * rampTimeSeconds));
    }

    /** Sets the target value to smooth towards. */
    void setTarget(FloatType newTarget) noexcept
    {
        targetValue = newTarget;
    }

    /** Resets the smoother to the current target value immediately. */
    void reset() noexcept
    {
        currentValue = targetValue;
    }

    /** Resets the smoother to a specific value. */
    void reset(FloatType value) noexcept
    {
        currentValue = value;
        targetValue = value;
    }

    /** Gets the next smoothed sample.
        @return The next smoothed value
    */
    FloatType getNext() noexcept
    {
        currentValue = currentValue * coefficient + targetValue * (static_cast<FloatType>(1) - coefficient);
        return currentValue;
    }

    /** Gets the current smoothed value without advancing. */
    FloatType getCurrentValue() const noexcept
    {
        return currentValue;
    }

    /** Gets the target value. */
    FloatType getTargetValue() const noexcept
    {
        return targetValue;
    }

    /** Checks if the smoother is currently smoothing (not yet reached target). */
    bool isSmoothing() const noexcept
    {
        return std::abs(currentValue - targetValue) > static_cast<FloatType>(0.0001);
    }

    /** Processes a block of samples, applying the same smoothed value to all.
        @param buffer The buffer to fill with smoothed values
        @param numSamples The number of samples to process
    */
    void processBlock(FloatType* buffer, int numSamples) noexcept
    {
        for (int i = 0; i < numSamples; ++i)
            buffer[i] = getNext();
    }

    /** Skips ahead by the given number of samples without generating output. */
    void skip(int numSamples) noexcept
    {
        for (int i = 0; i < numSamples; ++i)
            getNext();
    }

private:
    FloatType currentValue = static_cast<FloatType>(0);
    FloatType targetValue = static_cast<FloatType>(0);
    FloatType coefficient = static_cast<FloatType>(0);
    double sampleRate = 48000.0;
};

// Common type aliases
using ParameterSmootherFloat = ParameterSmoother<float>;
using ParameterSmootherDouble = ParameterSmoother<double>;
