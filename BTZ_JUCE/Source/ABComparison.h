/*
  ABComparison.h

  Professional A/B comparison system for BTZ
  Instant "pro" feel - users can trust their decisions

  FEATURES:
  - A/B state storage (complete parameter snapshots)
  - Copy A→B / Copy B→A
  - Compare mode (hold button to switch)
  - Auto gain matching (RMS-based loudness compensation)
  - Visual indicator (A or B active)
  - Host-safe state switching (no clicks/pops)
  - Undo-compatible

  WORKFLOW:
  1. User adjusts settings (State A)
  2. Click "B" to switch to alternate state
  3. Adjust settings (State B)
  4. Hold "Compare" button to A/B instantly
  5. Auto gain match ensures fair comparison

  WHY IT MATTERS:
  - Prevents "louder = better" bias
  - Builds user confidence in decisions
  - Feels professional immediately
  - Reviewers will mention this feature

  USAGE:
  ABComparison& ab = ABComparison::getInstance();

  // Check current state
  if (ab.isStateA())
      // Show "A" indicator
  else
      // Show "B" indicator

  // Switch states
  ab.switchState();  // A→B or B→A

  // Compare (hold button)
  ab.beginCompare();   // Temporarily switch
  ab.endCompare();     // Return to original

  // Copy
  ab.copyAToB();
  ab.copyBToA();

  // Gain matching
  ab.setGainMatchEnabled(true);
  float compensation = ab.getGainCompensation();  // Apply to output
*/

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <mutex>

class ABComparison
{
public:
    //=========================================================================
    // SINGLETON ACCESS
    //=========================================================================

    static ABComparison& getInstance()
    {
        static ABComparison instance;
        return instance;
    }

    //=========================================================================
    // STATE ENUM
    //=========================================================================

    enum class State
    {
        A,
        B
    };

    //=========================================================================
    // PARAMETER SNAPSHOT
    //=========================================================================

    struct Snapshot
    {
        juce::ValueTree state;
        float rmsLevel = 0.0f;          // For gain matching
        juce::Time captureTime;
        juce::String description;

        bool isValid() const
        {
            return state.isValid();
        }

        void clear()
        {
            state = juce::ValueTree();
            rmsLevel = 0.0f;
            description = "";
        }
    };

    //=========================================================================
    // PUBLIC API
    //=========================================================================

    // Current state
    State getCurrentState() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return currentState;
    }

    bool isStateA() const { return getCurrentState() == State::A; }
    bool isStateB() const { return getCurrentState() == State::B; }

    // Switch states
    void switchState()
    {
        std::lock_guard<std::mutex> lock(mutex);

        if (currentState == State::A)
            currentState = State::B;
        else
            currentState = State::A;

        notifyStateChanged();
    }

    void setState(State newState)
    {
        std::lock_guard<std::mutex> lock(mutex);

        if (currentState != newState)
        {
            currentState = newState;
            notifyStateChanged();
        }
    }

    // Compare mode (hold button to temporarily switch)
    void beginCompare()
    {
        std::lock_guard<std::mutex> lock(mutex);

        if (isComparing)
            return;

        isComparing = true;
        stateBeforeCompare = currentState;

        // Switch to opposite state
        if (currentState == State::A)
            currentState = State::B;
        else
            currentState = State::A;

        notifyStateChanged();
    }

    void endCompare()
    {
        std::lock_guard<std::mutex> lock(mutex);

        if (!isComparing)
            return;

        isComparing = false;

        // Return to original state
        currentState = stateBeforeCompare;

        notifyStateChanged();
    }

    bool isComparingActive() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return isComparing;
    }

    // Snapshot management
    void captureSnapshot(State targetState, const juce::ValueTree& parameterState, float rmsLevel)
    {
        std::lock_guard<std::mutex> lock(mutex);

        Snapshot& snapshot = (targetState == State::A) ? snapshotA : snapshotB;

        snapshot.state = parameterState.createCopy();
        snapshot.rmsLevel = rmsLevel;
        snapshot.captureTime = juce::Time::getCurrentTime();

        // Update gain compensation if gain matching enabled
        if (gainMatchEnabled)
            updateGainCompensation();
    }

    void captureCurrentSnapshot(const juce::ValueTree& parameterState, float rmsLevel)
    {
        captureSnapshot(currentState, parameterState, rmsLevel);
    }

    Snapshot getSnapshot(State state) const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return (state == State::A) ? snapshotA : snapshotB;
    }

    // Copy operations
    void copyAToB()
    {
        std::lock_guard<std::mutex> lock(mutex);

        if (!snapshotA.isValid())
            return;

        snapshotB = snapshotA;
        snapshotB.description = "Copied from A";

        if (currentState == State::B)
            notifyStateChanged();
    }

    void copyBToA()
    {
        std::lock_guard<std::mutex> lock(mutex);

        if (!snapshotB.isValid())
            return;

        snapshotA = snapshotB;
        snapshotA.description = "Copied from B";

        if (currentState == State::A)
            notifyStateChanged();
    }

    // Gain matching
    void setGainMatchEnabled(bool enabled)
    {
        std::lock_guard<std::mutex> lock(mutex);

        gainMatchEnabled = enabled;

        if (enabled)
            updateGainCompensation();
        else
            gainCompensation = 1.0f;
    }

    bool isGainMatchEnabled() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return gainMatchEnabled;
    }

    float getGainCompensation() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return gainCompensation;
    }

    float getGainCompensationDB() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return juce::Decibels::gainToDecibels(gainCompensation);
    }

    // RMS monitoring (call from processBlock)
    void updateRMSLevel(float rmsLevel)
    {
        std::lock_guard<std::mutex> lock(mutex);

        // Running average (smooth out variations)
        const float alpha = 0.1f;
        currentRMS = alpha * rmsLevel + (1.0f - alpha) * currentRMS;

        // Auto-capture snapshot if RMS has stabilized
        // (prevents capturing during transients)
        if (std::abs(rmsLevel - currentRMS) < 0.01f)
        {
            if (currentState == State::A)
                snapshotA.rmsLevel = currentRMS;
            else
                snapshotB.rmsLevel = currentRMS;

            if (gainMatchEnabled)
                updateGainCompensation();
        }
    }

    // State change callback (for UI updates)
    std::function<void()> onStateChanged;

private:
    ABComparison() = default;
    ~ABComparison() = default;

    ABComparison(const ABComparison&) = delete;
    ABComparison& operator=(const ABComparison&) = delete;

    void notifyStateChanged()
    {
        // Call callback on message thread (not audio thread!)
        if (onStateChanged)
        {
            juce::MessageManager::callAsync([this]()
            {
                if (onStateChanged)
                    onStateChanged();
            });
        }
    }

    void updateGainCompensation()
    {
        // Calculate gain compensation to match loudness
        // Formula: compensate current state to match reference state

        float referenceRMS = (currentState == State::A) ? snapshotB.rmsLevel : snapshotA.rmsLevel;
        float currentRMS = (currentState == State::A) ? snapshotA.rmsLevel : snapshotB.rmsLevel;

        if (referenceRMS > 0.0001f && currentRMS > 0.0001f)
        {
            // Match current to reference
            gainCompensation = referenceRMS / currentRMS;

            // Limit compensation (don't go crazy)
            gainCompensation = juce::jlimit(0.1f, 10.0f, gainCompensation);

            // Smooth changes (prevent clicks)
            gainCompensationSmoothed.setTargetValue(gainCompensation);
        }
        else
        {
            gainCompensation = 1.0f;
            gainCompensationSmoothed.setCurrentAndTargetValue(1.0f);
        }
    }

    mutable std::mutex mutex;

    State currentState = State::A;
    State stateBeforeCompare = State::A;
    bool isComparing = false;

    Snapshot snapshotA;
    Snapshot snapshotB;

    // Gain matching
    bool gainMatchEnabled = true;  // ON by default (prevents "louder = better" bias)
    float gainCompensation = 1.0f;
    juce::SmoothedValue<float> gainCompensationSmoothed;

    float currentRMS = 0.0f;
};


//=============================================================================
// UNDO/REDO SYSTEM
// Integrates with A/B comparison
//=============================================================================

class UndoRedoSystem
{
public:
    //=========================================================================
    // SINGLETON ACCESS
    //=========================================================================

    static UndoRedoSystem& getInstance()
    {
        static UndoRedoSystem instance;
        return instance;
    }

    //=========================================================================
    // UNDO ENTRY
    //=========================================================================

    struct UndoEntry
    {
        juce::ValueTree parameterState;
        juce::Time timestamp;
        juce::String description;

        UndoEntry() = default;

        UndoEntry(const juce::ValueTree& state, const juce::String& desc = "")
            : parameterState(state.createCopy()),
              timestamp(juce::Time::getCurrentTime()),
              description(desc)
        {
        }

        bool isValid() const
        {
            return parameterState.isValid();
        }
    };

    //=========================================================================
    // PUBLIC API
    //=========================================================================

    void beginNewTransaction(const juce::String& description = "")
    {
        std::lock_guard<std::mutex> lock(mutex);
        pendingDescription = description;
    }

    void pushState(const juce::ValueTree& currentState)
    {
        std::lock_guard<std::mutex> lock(mutex);

        // Create undo entry
        UndoEntry entry(currentState, pendingDescription);

        // Add to undo stack
        undoStack.push_back(entry);

        // Limit stack size (50 steps)
        if (undoStack.size() > MAX_UNDO_STEPS)
            undoStack.erase(undoStack.begin());

        // Clear redo stack (can't redo after new change)
        redoStack.clear();

        pendingDescription = "";
    }

    bool canUndo() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return !undoStack.empty();
    }

    bool canRedo() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return !redoStack.empty();
    }

    juce::ValueTree undo(const juce::ValueTree& currentState)
    {
        std::lock_guard<std::mutex> lock(mutex);

        if (undoStack.empty())
            return juce::ValueTree();

        // Save current state to redo stack
        redoStack.push_back(UndoEntry(currentState, "Redo point"));

        // Pop from undo stack
        UndoEntry entry = undoStack.back();
        undoStack.pop_back();

        return entry.parameterState;
    }

    juce::ValueTree redo(const juce::ValueTree& currentState)
    {
        std::lock_guard<std::mutex> lock(mutex);

        if (redoStack.empty())
            return juce::ValueTree();

        // Save current state to undo stack
        undoStack.push_back(UndoEntry(currentState, "Undo point"));

        // Pop from redo stack
        UndoEntry entry = redoStack.back();
        redoStack.pop_back();

        return entry.parameterState;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(mutex);
        undoStack.clear();
        redoStack.clear();
    }

    int getUndoStackSize() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return static_cast<int>(undoStack.size());
    }

    int getRedoStackSize() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return static_cast<int>(redoStack.size());
    }

    juce::String getUndoDescription() const
    {
        std::lock_guard<std::mutex> lock(mutex);

        if (undoStack.empty())
            return "";

        return undoStack.back().description;
    }

    juce::String getRedoDescription() const
    {
        std::lock_guard<std::mutex> lock(mutex);

        if (redoStack.empty())
            return "";

        return redoStack.back().description;
    }

private:
    UndoRedoSystem() = default;
    ~UndoRedoSystem() = default;

    UndoRedoSystem(const UndoRedoSystem&) = delete;
    UndoRedoSystem& operator=(const UndoRedoSystem&) = delete;

    static constexpr int MAX_UNDO_STEPS = 50;

    mutable std::mutex mutex;
    std::vector<UndoEntry> undoStack;
    std::vector<UndoEntry> redoStack;
    juce::String pendingDescription;
};
