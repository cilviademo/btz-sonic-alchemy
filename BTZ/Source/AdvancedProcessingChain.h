#pragma once
#include <JuceHeader.h>
#include "DeepFilterNet.h"

#ifdef BTZ_WITH_ML
#include <torch/torch.h>
#include "NeuralNetwork.h"
#include "TimbralTransfer.h"
#endif

class AdvancedProcessingChain {
public:
    AdvancedProcessingChain() = default;
    ~AdvancedProcessingChain() = default;

    void prepare(const juce::dsp::ProcessSpec& spec) {
        sampleRate = spec.sampleRate;
        blockSize = spec.maximumBlockSize;
        numChannels = spec.numChannels;

        // Initialize oversampling (8x for HQ mode)
        oversampling8x.initProcessing(blockSize);
        
        // ZDF Filters for high quality processing
        highpassFilter.prepare(spec);
        lowpassFilter.prepare(spec);
        
        // Advanced limiters with lookahead
        truePeakLimiter.prepare(spec);
        
        // Prepare AI modules if compiled with ML support
        #ifdef BTZ_WITH_ML
        if (neuralNetwork) {
            neuralNetwork->prepare(spec);
        }
        if (timbralTransfer) {
            timbralTransfer->prepare(spec);
        }
        #endif

        // Spectrum analyzer
        fftOrder = 12; // 4096 samples
        fft = std::make_unique<juce::dsp::FFT>(fftOrder);
        fftData.resize(2 * fft->getSize());
        spectrumData.resize(fft->getSize() / 2);
        
        // LUFS metering
        lufsProcessor.prepare(spec);
    }

    void processBlock(juce::dsp::ProcessContextReplacing<float>& context) {
        auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        
        // Store input for parallel processing
        inputBuffer.makeCopyOf(inputBlock, true);
        
        // Apply processing chain based on parameters
        if (oversamplingEnabled) {
            auto oversampledBlock = oversampling8x.processSamplesUp(inputBlock);
            processOversampledChain(oversampledBlock);
            oversampling8x.processSamplesDown(outputBlock);
        } else {
            processRegularChain(context);
        }
        
        // Parallel mix
        if (mixAmount < 1.0f) {
            for (size_t ch = 0; ch < outputBlock.getNumChannels(); ++ch) {
                auto* output = outputBlock.getChannelPointer(ch);
                auto* input = inputBuffer.getReadPointer(ch);
                
                juce::FloatVectorOperations::multiply(output, mixAmount, (int)outputBlock.getNumSamples());
                juce::FloatVectorOperations::addWithMultiply(output, input, 1.0f - mixAmount, (int)outputBlock.getNumSamples());
            }
        }
        
        // True peak limiting (always on for safety)
        truePeakLimiter.process(context);
        
        // Update meters
        updateMetering(outputBlock);
    }

    // Parameter setters
    void setPunchAmount(float amount) { punchAmount = amount; }
    void setWarmthAmount(float amount) { warmthAmount = amount; }
    void setBoomAmount(float amount) { boomAmount = amount; }
    void setMixAmount(float amount) { mixAmount = amount; }
    void setDriveAmount(float amount) { driveAmount = amount; }
    void setTextureEnabled(bool enabled) { textureEnabled = enabled; }
    void setOversamplingEnabled(bool enabled) { oversamplingEnabled = enabled; }
    void setLUFSTarget(float target) { lufsTarget = target; }
    
    #ifdef BTZ_WITH_ML
    void setAIEnhanceEnabled(bool enabled) { aiEnhanceEnabled = enabled; }
    void setTimbralTransferEnabled(bool enabled) { timbralTransferEnabled = enabled; }
    #endif

    // Meter getters
    float getInputLevel() const { return inputLevel.load(); }
    float getOutputLevel() const { return outputLevel.load(); }
    float getGainReduction() const { return gainReduction.load(); }
    float getLUFSIntegrated() const { return lufsIntegrated.load(); }
    float getTruePeak() const { return truePeak.load(); }
    const std::vector<float>& getSpectrumData() const { return spectrumData; }

private:
    double sampleRate = 44100.0;
    int blockSize = 512;
    int numChannels = 2;

    // Processing parameters
    float punchAmount = 0.5f;
    float warmthAmount = 0.5f;
    float boomAmount = 0.5f;
    float mixAmount = 0.8f;
    float driveAmount = 0.5f;
    bool textureEnabled = false;
    bool oversamplingEnabled = true;
    float lufsTarget = -8.0f;
    
    #ifdef BTZ_WITH_ML
    bool aiEnhanceEnabled = false;
    bool timbralTransferEnabled = false;
    #endif

    // Advanced processing modules
    juce::dsp::Oversampling<float> oversampling8x{2, 3, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR};
    juce::dsp::ProcessorChain<
        juce::dsp::IIR::Filter<float>, // Highpass
        juce::dsp::IIR::Filter<float>  // Lowpass  
    > filterChain;
    
    juce::dsp::IIR::Filter<float> highpassFilter;
    juce::dsp::IIR::Filter<float> lowpassFilter;
    juce::dsp::Limiter<float> truePeakLimiter;
    
    #ifdef BTZ_WITH_ML
    std::unique_ptr<NeuralNetwork> neuralNetwork;
    std::unique_ptr<TimbralTransfer> timbralTransfer;
    #endif

    // Metering and analysis
    std::atomic<float> inputLevel{0.0f};
    std::atomic<float> outputLevel{0.0f};
    std::atomic<float> gainReduction{0.0f};
    std::atomic<float> lufsIntegrated{-23.0f};
    std::atomic<float> truePeak{-6.0f};
    
    // FFT analysis
    int fftOrder;
    std::unique_ptr<juce::dsp::FFT> fft;
    std::vector<float> fftData;
    std::vector<float> spectrumData;
    
    // LUFS processing
    juce::dsp::ProcessorChain<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>
    > lufsProcessor;
    
    // Buffers
    juce::AudioBuffer<float> inputBuffer;

    void processOversampledChain(juce::dsp::AudioBlock<float>& block) {
        // High quality processing chain with ZDF filters
        // Implementation would go here...
    }
    
    void processRegularChain(juce::dsp::ProcessContextReplacing<float>& context) {
        // Regular processing chain
        // Implementation would go here...
    }
    
    void updateMetering(const juce::dsp::AudioBlock<float>& block) {
        // Update all meters with block data
        float maxInput = 0.0f, maxOutput = 0.0f;
        
        for (size_t ch = 0; ch < block.getNumChannels(); ++ch) {
            auto* channelData = block.getChannelPointer(ch);
            for (size_t i = 0; i < block.getNumSamples(); ++i) {
                maxInput = std::max(maxInput, std::abs(channelData[i]));
                maxOutput = std::max(maxOutput, std::abs(channelData[i]));
            }
        }
        
        inputLevel.store(maxInput);
        outputLevel.store(maxOutput);
        
        // Update spectrum analyzer
        updateSpectrumAnalysis(block);
    }
    
    void updateSpectrumAnalysis(const juce::dsp::AudioBlock<float>& block) {
        if (block.getNumChannels() == 0) return;
        
        // Copy mono mix to FFT buffer
        size_t fftSize = fft->getSize();
        size_t samplesToCopy = std::min(fftSize, block.getNumSamples());
        
        // Clear FFT buffer
        std::fill(fftData.begin(), fftData.end(), 0.0f);
        
        // Mix channels to mono and copy to FFT buffer
        for (size_t i = 0; i < samplesToCopy; ++i) {
            float sample = 0.0f;
            for (size_t ch = 0; ch < block.getNumChannels(); ++ch) {
                sample += block.getChannelPointer(ch)[i];
            }
            fftData[i] = sample / float(block.getNumChannels());
        }
        
        // Perform FFT
        fft->performFrequencyOnlyForwardTransform(fftData.data());
        
        // Convert to magnitude spectrum
        for (size_t i = 0; i < spectrumData.size(); ++i) {
            spectrumData[i] = std::sqrt(fftData[i * 2] * fftData[i * 2] + fftData[i * 2 + 1] * fftData[i * 2 + 1]);
        }
    }
};