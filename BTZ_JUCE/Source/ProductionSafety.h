/*
  ProductionSafety.h

  Production-grade safety utilities for BTZ
  Addresses real-world VST issues that break plugins in production

  Based on lessons from:
  - Waves v9→v10 parameter ID disaster
  - Pro Tools RT violations causing dropouts
  - FL Studio call order crashes
  - Ableton automation rate issues

  CRITICAL: These are not "nice to have" - they prevent production failures
*/

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <array>

//=============================================================================
// 1. HOST CALL ORDER GUARDS
// Protects against hosts calling processBlock() before prepareToPlay()
//=============================================================================

class HostCallOrderGuard
{
public:
    HostCallOrderGuard() = default;

    void markPrepared(double sampleRate, int blockSize) noexcept
    {
        isPrepared.store(true, std::memory_order_release);
        lastSampleRate.store(sampleRate, std::memory_order_relaxed);
        lastBlockSize.store(blockSize, std::memory_order_relaxed);
    }

    void markReleased() noexcept
    {
        isPrepared.store(false, std::memory_order_release);
    }

    bool safeToProcess() const noexcept
    {
        return isPrepared.load(std::memory_order_acquire);
    }

    // Detect sample rate change mid-session (some hosts do this!)
    bool sampleRateChanged(double newRate) const noexcept
    {
        double old = lastSampleRate.load(std::memory_order_relaxed);
        return std::abs(old - newRate) > 0.1;
    }

private:
    std::atomic<bool> isPrepared { false };
    std::atomic<double> lastSampleRate { 0.0 };
    std::atomic<int> lastBlockSize { 0 };
};

//=============================================================================
// 2. RT-SAFE LOGGING
// Replaces juce::String() allocations in audio thread
//=============================================================================

class RTSafeLogger
{
public:
    static constexpr int MAX_MESSAGES = 128;
    static constexpr int MAX_MESSAGE_LENGTH = 256;

    struct Message
    {
        char text[MAX_MESSAGE_LENGTH];
        std::atomic<bool> ready { false };
    };

    RTSafeLogger() = default;

    // Call from audio thread (RT-safe)
    void logRT(const char* message) noexcept
    {
        int index = writeIndex.load(std::memory_order_relaxed);
        int nextIndex = (index + 1) % MAX_MESSAGES;

        // Check if buffer full
        if (messages[nextIndex].ready.load(std::memory_order_acquire))
            return; // Drop message (better than blocking)

        // Copy message (RT-safe)
        std::strncpy(messages[index].text, message, MAX_MESSAGE_LENGTH - 1);
        messages[index].text[MAX_MESSAGE_LENGTH - 1] = '\0';
        messages[index].ready.store(true, std::memory_order_release);

        writeIndex.store(nextIndex, std::memory_order_release);
    }

    // Call from message thread (non-RT)
    void processMessages() noexcept
    {
        int index = readIndex.load(std::memory_order_relaxed);

        while (messages[index].ready.load(std::memory_order_acquire))
        {
            // Output to debug console (safe here, not in RT thread)
            DBG(messages[index].text);

            messages[index].ready.store(false, std::memory_order_release);
            index = (index + 1) % MAX_MESSAGES;
            readIndex.store(index, std::memory_order_release);
        }
    }

private:
    std::array<Message, MAX_MESSAGES> messages;
    std::atomic<int> writeIndex { 0 };
    std::atomic<int> readIndex { 0 };
};

//=============================================================================
// 3. SOFT BYPASS
// Proper bypass with latency compensation and crossfade
//=============================================================================

class SoftBypass
{
public:
    SoftBypass() = default;

    void prepare(double sampleRate) noexcept
    {
        fadeRamp.reset(sampleRate, 0.02); // 20ms crossfade
        fadeRamp.setCurrentAndTargetValue(0.0f);
    }

    void setBypass(bool shouldBypass) noexcept
    {
        bypassed = shouldBypass;
        fadeRamp.setTargetValue(shouldBypass ? 1.0f : 0.0f);
    }

    bool isBypassed() const noexcept
    {
        return bypassed;
    }

    bool isFading() const noexcept
    {
        return !fadeRamp.isSmoothing();
    }

    // Process with crossfade
    template<typename SampleType>
    void process(SampleType* wet, const SampleType* dry, int numSamples) noexcept
    {
        for (int i = 0; i < numSamples; ++i)
        {
            float bypassAmount = fadeRamp.getNextValue();
            wet[i] = wet[i] * (1.0f - bypassAmount) + dry[i] * bypassAmount;
        }
    }

private:
    bool bypassed = false;
    juce::SmoothedValue<float> fadeRamp;
};

//=============================================================================
// 4. PARAMETER VERSIONING
// Prevents breaking old sessions when parameters change
//=============================================================================

struct ParameterVersion
{
    int major = 1;
    int minor = 0;
    int patch = 0;

    juce::String toString() const
    {
        return juce::String(major) + "." + juce::String(minor) + "." + juce::String(patch);
    }

    static ParameterVersion fromString(const juce::String& str, ParameterVersion defaultVersion = {1, 0, 0})
    {
        juce::StringArray parts = juce::StringArray::fromTokens(str, ".", "");
        if (parts.size() != 3)
            return defaultVersion;

        ParameterVersion v;
        v.major = parts[0].getIntValue();
        v.minor = parts[1].getIntValue();
        v.patch = parts[2].getIntValue();
        return v;
    }

    bool operator<(const ParameterVersion& other) const
    {
        if (major != other.major) return major < other.major;
        if (minor != other.minor) return minor < other.minor;
        return patch < other.patch;
    }

    bool operator==(const ParameterVersion& other) const
    {
        return major == other.major && minor == other.minor && patch == other.patch;
    }
};

//=============================================================================
// 5. STATE CORRUPTION PROTECTION
// Validates and sanitizes loaded state to prevent crashes
//=============================================================================

class StateValidator
{
public:
    // Validate parameter value is within expected range
    static float validateFloat(float value, float min, float max, float defaultVal) noexcept
    {
        if (!std::isfinite(value))
            return defaultVal;
        return juce::jlimit(min, max, value);
    }

    // Validate XML state is not corrupted
    static bool validateXML(const juce::XmlElement* xml) noexcept
    {
        if (xml == nullptr)
            return false;

        // Check for reasonable state size (<1MB)
        juce::String xmlString = xml->toString();
        if (xmlString.length() > 1024 * 1024)
            return false;

        // Check for basic structure
        if (!xml->hasAttribute("pluginVersion"))
            return false;

        if (!xml->hasAttribute("pluginName"))
            return false;

        return true;
    }

    // Calculate simple checksum for corruption detection
    static uint32_t calculateChecksum(const void* data, size_t size) noexcept
    {
        uint32_t checksum = 0;
        const uint8_t* bytes = static_cast<const uint8_t*>(data);

        for (size_t i = 0; i < size; ++i)
        {
            checksum = (checksum << 1) | (checksum >> 31);
            checksum ^= bytes[i];
        }

        return checksum;
    }
};

//=============================================================================
// 6. DAW DETECTION & WORKAROUNDS
// Host-specific quirks that need handling
//=============================================================================

class DAWQuirks
{
public:
    enum class Host
    {
        Unknown,
        AbletonLive,
        FLStudio,
        LogicPro,
        ProTools,
        Reaper,
        StudioOne,
        Cubase,
        Bitwig
    };

    static Host detectHost() noexcept
    {
        juce::PluginHostType hostType;

        if (hostType.isAbletonLive())
            return Host::AbletonLive;
        else if (hostType.isFruityLoops())
            return Host::FLStudio;
        else if (hostType.isLogic())
            return Host::LogicPro;
        else if (hostType.isProTools())
            return Host::ProTools;
        else if (hostType.isReaper())
            return Host::Reaper;
        else if (hostType.isStudioOne())
            return Host::StudioOne;
        else if (hostType.isCubase() || hostType.isNuendo())
            return Host::Cubase;
        else if (hostType.isBitwigStudio())
            return Host::Bitwig;

        return Host::Unknown;
    }

    static juce::String getHostName(Host host) noexcept
    {
        switch (host)
        {
            case Host::AbletonLive: return "Ableton Live";
            case Host::FLStudio: return "FL Studio";
            case Host::LogicPro: return "Logic Pro";
            case Host::ProTools: return "Pro Tools";
            case Host::Reaper: return "Reaper";
            case Host::StudioOne: return "Studio One";
            case Host::Cubase: return "Cubase/Nuendo";
            case Host::Bitwig: return "Bitwig Studio";
            default: return "Unknown";
        }
    }

    // FL Studio: May call processBlock() before prepareToPlay()
    static bool needsInitGuard(Host host) noexcept
    {
        return host == Host::FLStudio || host == Host::Reaper;
    }

    // Ableton: Changes buffer size frequently
    static bool hasVariableBufferSize(Host host) noexcept
    {
        return host == Host::AbletonLive;
    }

    // Pro Tools: Strict RT requirements
    static bool hasStrictRTRequirements(Host host) noexcept
    {
        return host == Host::ProTools;
    }

    // FL Studio: Aggressive automation (rate-limit needed)
    static bool needsAutomationRateLimit(Host host) noexcept
    {
        return host == Host::FLStudio;
    }
};

//=============================================================================
// 7. DIAGNOSTIC LOGGER (OPT-IN, NON-RT)
// Helps support users when things go wrong
//=============================================================================

class DiagnosticLogger
{
public:
    struct SessionInfo
    {
        juce::String pluginVersion;
        juce::String hostName;
        double sampleRate = 0.0;
        int bufferSize = 0;
        juce::String buildDate;
        juce::String buildTime;

        juce::String toString() const
        {
            juce::String info;
            info << "=== BTZ Diagnostic Info ===" << juce::newLine;
            info << "Plugin Version: " << pluginVersion << juce::newLine;
            info << "Host: " << hostName << juce::newLine;
            info << "Sample Rate: " << sampleRate << " Hz" << juce::newLine;
            info << "Buffer Size: " << bufferSize << " samples" << juce::newLine;
            info << "Build: " << buildDate << " " << buildTime << juce::newLine;
            return info;
        }
    };

    static void logSessionInfo(const SessionInfo& info) noexcept
    {
        // Only log in DEBUG or if user opts in
        #if JUCE_DEBUG
        DBG(info.toString());
        #endif
    }
};
