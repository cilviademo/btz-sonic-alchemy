/*
  ==============================================================================

  PerformanceGuardrails.cpp

  ==============================================================================
*/

#include "PerformanceGuardrails.h"

// ============================================================================
// PerformanceMonitor Implementation
// ============================================================================

PerformanceMonitor::PerformanceMonitor() = default;

void PerformanceMonitor::prepare(double sr, int samplesPerBlock)
{
    sampleRate = sr;
    blockSize = samplesPerBlock;

    // Calculate time budget: block duration in microseconds
    timebudgetUs = (static_cast<float>(blockSize) / static_cast<float>(sampleRate)) * 1000000.0f;

    // Calculate smoothing coefficient (10% per block)
    cpuSmoothingCoeff = 0.1f;

    resetPeak();
}

void PerformanceMonitor::startBlock()
{
    blockStartTime = std::chrono::high_resolution_clock::now();
}

void PerformanceMonitor::endBlock()
{
    auto blockEndTime = std::chrono::high_resolution_clock::now();
    auto durationUs = std::chrono::duration_cast<std::chrono::microseconds>(
        blockEndTime - blockStartTime
    ).count();

    // Calculate CPU percentage
    float cpuPercent = (static_cast<float>(durationUs) / timebudgetUs) * 100.0f;

    // Update current CPU
    currentCPU.store(cpuPercent);

    // Update smoothed average
    float avg = averageCPU.load();
    avg += cpuSmoothingCoeff * (cpuPercent - avg);
    averageCPU.store(avg);

    // Update peak
    float peak = peakCPU.load();
    if (cpuPercent > peak)
        peakCPU.store(cpuPercent);

    // Detect sustained overload
    if (cpuPercent > 80.0f)
    {
        overloadCounter++;
        if (overloadCounter >= overloadThreshold)
            overloaded.store(true);
    }
    else
    {
        overloadCounter = 0;
        if (cpuPercent < 60.0f) // Recovery threshold
            overloaded.store(false);
    }
}

void PerformanceMonitor::resetPeak()
{
    peakCPU.store(0.0f);
    overloadCounter = 0;
    overloaded.store(false);
}

// ============================================================================
// QualityTierManager Implementation
// ============================================================================

QualityTierManager::QualityTierManager() = default;

bool QualityTierManager::updateFromCPU(const PerformanceMonitor& monitor)
{
    if (!autoAdjust)
        return false;

    float avgCPU = monitor.getAverageCPU();
    Tier desiredTier = currentTier;

    // Determine desired tier based on CPU load
    if (avgCPU > 70.0f && currentTier != Tier::Eco)
    {
        // High CPU - downgrade quality
        desiredTier = (currentTier == Tier::High) ? Tier::Normal : Tier::Eco;
    }
    else if (avgCPU < 40.0f && currentTier != targetTier)
    {
        // Low CPU - upgrade towards target
        if (currentTier == Tier::Eco && targetTier >= Tier::Normal)
            desiredTier = Tier::Normal;
        else if (currentTier == Tier::Normal && targetTier == Tier::High)
            desiredTier = Tier::High;
    }

    // Hysteresis: require sustained change before switching
    if (desiredTier != currentTier)
    {
        tierChangeCounter++;
        if (tierChangeCounter >= tierChangeDelay)
        {
            currentTier = desiredTier;
            tierChangeCounter = 0;
            return true; // Tier changed
        }
    }
    else
    {
        tierChangeCounter = 0;
    }

    return false;
}

// ============================================================================
// ProcessingBudget Implementation
// ============================================================================

ProcessingBudget::ProcessingBudget() = default;

void ProcessingBudget::setTotalBudgetUs(float budgetUs)
{
    totalBudgetUs = budgetUs;
    reset();
}

void ProcessingBudget::allocateBudget(const juce::String& moduleName, float percentage)
{
    ModuleBudget budget;
    budget.allocatedUs = totalBudgetUs * percentage;
    budget.usedUs = 0.0f;

    moduleBudgets.set(moduleName, budget);
}

bool ProcessingBudget::canProcess(const juce::String& moduleName, float estimatedUs) const
{
    // Check global budget
    if (estimatedUs > remainingUs)
        return false;

    // Check module-specific budget
    if (moduleBudgets.contains(moduleName))
    {
        const auto& budget = moduleBudgets[moduleName];
        if (estimatedUs > (budget.allocatedUs - budget.usedUs))
            return false;
    }

    return true;
}

void ProcessingBudget::consumeBudget(const juce::String& moduleName, float actualUs)
{
    remainingUs -= actualUs;

    if (moduleBudgets.contains(moduleName))
    {
        auto budget = moduleBudgets[moduleName];
        budget.usedUs += actualUs;
        moduleBudgets.set(moduleName, budget);
    }
}

void ProcessingBudget::reset()
{
    remainingUs = totalBudgetUs;

    // Reset all module budgets
    juce::HashMap<juce::String, ModuleBudget>::Iterator iter(moduleBudgets);
    while (iter.next())
    {
        ModuleBudget budget = iter.getValue();
        budget.usedUs = 0.0f;
        moduleBudgets.set(iter.getKey(), budget);
    }
}

// ============================================================================
// CompositePerformanceGuardrails Implementation
// ============================================================================

CompositePerformanceGuardrails::CompositePerformanceGuardrails() = default;

void CompositePerformanceGuardrails::prepare(double sampleRate, int samplesPerBlock)
{
    monitor.prepare(sampleRate, samplesPerBlock);
    budget.setTotalBudgetUs(monitor.getTimebudgetUs());

    // Allocate budget to modules (example allocation)
    budget.allocateBudget("SPARK", 0.25f);    // 25% - limiter/saturation
    budget.allocateBudget("SHINE", 0.15f);    // 15% - air EQ
    budget.allocateBudget("Transient", 0.20f); // 20% - transient shaping
    budget.allocateBudget("Saturation", 0.20f); // 20% - saturation
    budget.allocateBudget("Other", 0.20f);    // 20% - other processing
}

void CompositePerformanceGuardrails::startBlock()
{
    monitor.startBlock();
    budget.reset();
}

void CompositePerformanceGuardrails::endBlock()
{
    monitor.endBlock();

    // Update quality tier based on CPU load
    tierManager.updateFromCPU(monitor);
}

bool CompositePerformanceGuardrails::shouldSkipProcessing(const juce::String& moduleName, float estimatedUs)
{
    return !budget.canProcess(moduleName, estimatedUs);
}
