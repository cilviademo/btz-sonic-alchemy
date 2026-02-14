#pragma once

#ifdef BTZ_WITH_ML
#include <JuceHeader.h>
#include <torch/torch.h>
#include <vector>

class NeuralNetwork {
public:
    NeuralNetwork(const std::string& modelPath) {
        try {
            model = torch::jit::load(modelPath);
            model.eval();
            isLoaded = true;
        } catch (const std::exception& e) {
            juce::Logger::writeToLog("Failed to load neural network model: " + juce::String(e.what()));
            isLoaded = false;
        }
    }
    
    ~NeuralNetwork() = default;

    void prepare(const juce::dsp::ProcessSpec& spec) {
        sampleRate = spec.sampleRate;
        blockSize = spec.maximumBlockSize;
        numChannels = spec.numChannels;
        
        // Prepare input/output tensors
        inputBuffer.resize(blockSize * numChannels);
        outputBuffer.resize(blockSize * numChannels);
    }

    bool process(juce::dsp::AudioBlock<float>& block) {
        if (!isLoaded || !isEnabled) return false;
        
        try {
            // Convert JUCE AudioBlock to PyTorch tensor
            copyAudioBlockToBuffer(block);
            
            auto inputTensor = torch::from_blob(
                inputBuffer.data(), 
                {1, static_cast<long>(numChannels), static_cast<long>(blockSize)}, 
                torch::kFloat
            );
            
            // Inference
            std::vector<torch::jit::IValue> inputs;
            inputs.push_back(inputTensor);
            
            torch::NoGradGuard no_grad;
            auto output = model.forward(inputs).toTensor();
            
            // Convert back to JUCE AudioBlock
            copyTensorToAudioBlock(output, block);
            
            return true;
        } catch (const std::exception& e) {
            juce::Logger::writeToLog("Neural network inference error: " + juce::String(e.what()));
            return false;
        }
    }

    void setEnabled(bool enabled) { isEnabled = enabled; }
    bool getEnabled() const { return isEnabled && isLoaded; }
    bool isModelLoaded() const { return isLoaded; }

    // Analysis methods for BTZ features
    float analyzeTransientStrength(const juce::dsp::AudioBlock<float>& block) {
        if (!isLoaded) return 0.5f; // Default fallback
        
        // Simplified transient analysis using model features
        // In production, this would use a specialized analysis model
        return 0.3f + (rand() % 40) / 100.0f; // Simulated for now
    }
    
    float analyzeLowEndEnergy(const juce::dsp::AudioBlock<float>& block) {
        if (!isLoaded) return 0.5f;
        return 0.4f + (rand() % 30) / 100.0f; // Simulated
    }

private:
    torch::jit::script::Module model;
    bool isLoaded = false;
    bool isEnabled = false;
    
    double sampleRate = 44100.0;
    size_t blockSize = 512;
    size_t numChannels = 2;
    
    std::vector<float> inputBuffer;
    std::vector<float> outputBuffer;
    
    void copyAudioBlockToBuffer(const juce::dsp::AudioBlock<float>& block) {
        size_t sampleIndex = 0;
        for (size_t sample = 0; sample < block.getNumSamples(); ++sample) {
            for (size_t channel = 0; channel < block.getNumChannels(); ++channel) {
                inputBuffer[sampleIndex++] = block.getSample(channel, sample);
            }
        }
    }
    
    void copyTensorToAudioBlock(const torch::Tensor& tensor, juce::dsp::AudioBlock<float>& block) {
        auto accessor = tensor.accessor<float, 3>();
        
        for (size_t sample = 0; sample < block.getNumSamples(); ++sample) {
            for (size_t channel = 0; channel < block.getNumChannels(); ++channel) {
                block.setSample(channel, sample, accessor[0][channel][sample]);
            }
        }
    }
};

#else
// Fallback implementation when ML is disabled
class NeuralNetwork {
public:
    NeuralNetwork(const std::string&) {}
    void prepare(const juce::dsp::ProcessSpec&) {}
    bool process(juce::dsp::AudioBlock<float>&) { return false; }
    void setEnabled(bool) {}
    bool getEnabled() const { return false; }
    bool isModelLoaded() const { return false; }
    float analyzeTransientStrength(const juce::dsp::AudioBlock<float>&) { return 0.5f; }
    float analyzeLowEndEnergy(const juce::dsp::AudioBlock<float>&) { return 0.5f; }
};
#endif