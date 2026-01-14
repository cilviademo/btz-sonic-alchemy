/*
  ==============================================================================

  PerformanceGuardrails.h

  Phase 3.2: CPU Guardrails and Dynamic Quality Management
  - Real-time CPU load monitoring
  - Automatic quality degradation under load
  - Budget-based processing decisions
  - Predictable worst-case performance

  Features:
  - Per-block CPU% measurement
  - Dynamic quality tier switching (High -> Normal -> Eco)
  - Processing budget allocation
  - Overrun detection and recovery
  - Graceful degradation (no dropouts)

  Applications:
  - Prevent audio dropouts in CPU-constrained scenarios
  - Adapt quality to available resources
  - Maintain real-time guarantees
  - Provide consistent user experience across systems

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <chrono>

class PerformanceMonitor
{
public:
    PerformanceMonitor();
    ~PerformanceMonitor() = default;

    void prepare(double sampleRate, int samplesPerBlock);

    // Measure CPU usage for a processing block
    void startBlock();
    void endBlock();

    // Get metrics
    float getCurrentCPU() const { return currentCPU.load(); }   // Current block %
    float getAverageCPU() const { return averageCPU.load(); }   // Smoothed average
    float getPeakCPU() const { return peakCPU.load(); }         // Peak since reset
    bool isOverloaded() const { return overloaded.load(); }     // Sustained overload

    // Reset peak measurements
    void resetPeak();

    // Get time budget for current block (microseconds)
    float getTimebudgetUs() const { return timebudgetUs; }

private:
    double sampleRate = 48000.0;
    int blockSize = 512;
    float timebudgetUs = 10666.0f; // Time budget in microseconds

    // Timing
    std::chrono::high_resolution_clock::time_point blockStartTime;

    // Metrics
    std::atomic<float> currentCPU{0.0f};
    std::atomic<float> averageCPU{0.0f};
    std::atomic<float> peakCPU{0.0f};
    std::atomic<bool> overloaded{false};

    // Smoothing
    float cpuSmoothingCoeff = 0.1f;

    // Overload detection
    int overloadCounter = 0;
    static constexpr int overloadThreshold = 3; // 3 consecutive blocks > 80%

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerformanceMonitor)
};

/*
  ==============================================================================
  QualityTierManager - Dynamic quality tier switching
  ==============================================================================
*/

class QualityTierManager
{
public:
    enum class Tier
    {
        Eco,        // Minimum quality (1x OS, light processing)
        Normal,     // Balanced quality (2x OS, standard processing)
        High        // Maximum quality (4x OS, full processing)
    };

    QualityTierManager();

    void setTargetTier(Tier tier) { targetTier = tier; }
    Tier getTargetTier() const { return targetTier; }

    // Update based on CPU load (returns true if tier changed)
    bool updateFromCPU(const PerformanceMonitor& monitor);

    Tier getCurrentTier() const { return currentTier; }

    // Manual tier locking (disable auto-adjustment)
    void setAutoAdjust(bool enabled) { autoAdjust = enabled; }
    bool isAutoAdjust() const { return autoAdjust; }

private:
    Tier targetTier = Tier::Normal;
    Tier currentTier = Tier::Normal;
    bool autoAdjust = true;

    // Hysteresis for tier switching (prevent oscillation)
    int tierChangeCounter = 0;
    static constexpr int tierChangeDelay = 5; // 5 blocks before switching

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QualityTierManager)
};

/*
  ==============================================================================
  ProcessingBudget - Time budget allocation for modules
  ==============================================================================
*/

class ProcessingBudget
{
public:
    ProcessingBudget();

    void setTotalBudgetUs(float budgetUs);

    // Allocate budget percentages to modules
    // Example: allocateBudget("SPARK", 0.20) = 20% of total budget
    void allocateBudget(const juce::String& moduleName, float percentage);

    // Check if module can run (has budget remaining)
    bool canProcess(const juce::String& moduleName, float estimatedUs) const;

    // Consume budget (call after processing)
    void consumeBudget(const juce::String& moduleName, float actualUs);

    // Reset per-block
    void reset();

    // Get remaining budget
    float getRemainingUs() const { return remainingUs; }

private:
    float totalBudgetUs = 10000.0f;
    float remainingUs = 10000.0f;

    struct ModuleBudget
    {
        float allocatedUs = 0.0f;
        float usedUs = 0.0f;
    };

    juce::HashMap<juce::String, ModuleBudget> moduleBudgets;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProcessingBudget)
};

/*
  ==============================================================================
  CompositePerformanceGuardrails - All performance features
  ==============================================================================
*/

class CompositePerformanceGuardrails
{
public:
    CompositePerformanceGuardrails();

    void prepare(double sampleRate, int samplesPerBlock);

    // Call at start/end of processBlock
    void startBlock();
    void endBlock();

    // Get current state
    const PerformanceMonitor& getMonitor() const { return monitor; }
    QualityTierManager::Tier getCurrentTier() const { return tierManager.getCurrentTier(); }
    bool shouldSkipProcessing(const juce::String& moduleName, float estimatedUs);

    // Configuration
    void setAutoQualityAdjust(bool enabled) { tierManager.setAutoAdjust(enabled); }
    void setTargetTier(QualityTierManager::Tier tier) { tierManager.setTargetTier(tier); }

private:
    PerformanceMonitor monitor;
    QualityTierManager tierManager;
    ProcessingBudget budget;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompositePerformanceGuardrails)
};
