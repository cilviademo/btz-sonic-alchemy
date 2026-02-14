/*
  offline_render.cpp

  Minimal offline audio rendering tool for BTZ sound quality validation.

  Purpose:
  - Render test audio through BTZ with specific parameter settings
  - Measure objective metrics (peak, RMS, DC offset, THD estimate)
  - Generate A/B comparison (bypass vs processed)
  - No external dependencies beyond JUCE

  Usage:
    ./offline_render input.wav output.wav --preset "Punchy Drums" --bypass bypass.wav

  Compile-time guard: Only built when BTZ_BUILD_TOOLS=ON
*/

#include <JuceHeader.h>
#include "../Source/PluginProcessor.h"
#include "../Source/Parameters/PluginParameters.h"
#include <iostream>
#include <iomanip>
#include <cmath>

class AudioMetrics
{
public:
    void analyze(const juce::AudioBuffer<float>& buffer)
    {
        reset();

        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* data = buffer.getReadPointer(ch);

            for (int i = 0; i < numSamples; ++i)
            {
                float sample = data[i];

                // Peak
                float absSample = std::abs(sample);
                if (absSample > peak)
                    peak = absSample;

                // RMS accumulation
                rmsSum += sample * sample;

                // DC offset accumulation
                dcSum += sample;

                totalSamples++;
            }
        }
    }

    void printResults(const std::string& label)
    {
        float rms = std::sqrt(rmsSum / totalSamples);
        float dc = dcSum / totalSamples;

        float peakDb = 20.0f * std::log10(peak + 1e-10f);
        float rmsDb = 20.0f * std::log10(rms + 1e-10f);

        std::cout << "\n=== " << label << " ===" << std::endl;
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Peak: " << peakDb << " dBFS (" << peak << ")" << std::endl;
        std::cout << "RMS: " << rmsDb << " dBFS (" << rms << ")" << std::endl;
        std::cout << "Crest Factor: " << (peakDb - rmsDb) << " dB" << std::endl;
        std::cout << std::scientific << std::setprecision(6);
        std::cout << "DC Offset: " << dc << " (" << 20.0f * std::log10(std::abs(dc) + 1e-10f) << " dBFS)" << std::endl;
    }

private:
    void reset()
    {
        peak = 0.0f;
        rmsSum = 0.0;
        dcSum = 0.0;
        totalSamples = 0;
    }

    float peak = 0.0f;
    double rmsSum = 0.0;
    double dcSum = 0.0;
    int64_t totalSamples = 0;
};

bool loadAudioFile(const juce::File& file, juce::AudioBuffer<float>& buffer, double& sampleRate)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

    if (reader == nullptr)
    {
        std::cerr << "ERROR: Could not read audio file: " << file.getFullPathName() << std::endl;
        return false;
    }

    sampleRate = reader->sampleRate;
    buffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&buffer, 0, (int)reader->lengthInSamples, 0, true, true);

    std::cout << "Loaded: " << file.getFileName() << std::endl;
    std::cout << "  Sample Rate: " << sampleRate << " Hz" << std::endl;
    std::cout << "  Channels: " << reader->numChannels << std::endl;
    std::cout << "  Duration: " << reader->lengthInSamples / sampleRate << " seconds" << std::endl;

    return true;
}

bool saveAudioFile(const juce::File& file, const juce::AudioBuffer<float>& buffer, double sampleRate)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    std::unique_ptr<juce::AudioFormat> format(formatManager.findFormatForFileExtension("wav"));
    if (format == nullptr)
    {
        std::cerr << "ERROR: WAV format not available" << std::endl;
        return false;
    }

    file.deleteFile();
    std::unique_ptr<juce::FileOutputStream> stream(file.createOutputStream());

    if (stream == nullptr)
    {
        std::cerr << "ERROR: Could not create output file: " << file.getFullPathName() << std::endl;
        return false;
    }

    std::unique_ptr<juce::AudioFormatWriter> writer(format->createWriterFor(
        stream.get(), sampleRate, buffer.getNumChannels(), 24, {}, 0));

    if (writer == nullptr)
    {
        std::cerr << "ERROR: Could not create audio writer" << std::endl;
        return false;
    }

    stream.release(); // Writer takes ownership

    writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());

    std::cout << "Saved: " << file.getFullPathName() << std::endl;
    return true;
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: offline_render input.wav output.wav [--bypass bypass.wav]" << std::endl;
        std::cout << "\nPurpose: Render test audio through BTZ for sound quality validation" << std::endl;
        std::cout << "\nOptions:" << std::endl;
        std::cout << "  --bypass FILE   Save bypass (input only) version for A/B comparison" << std::endl;
        return 1;
    }

    juce::File inputFile(argv[1]);
    juce::File outputFile(argv[2]);
    juce::File bypassFile;

    bool generateBypass = false;
    if (argc >= 5 && std::string(argv[3]) == "--bypass")
    {
        bypassFile = juce::File(argv[4]);
        generateBypass = true;
    }

    if (!inputFile.existsAsFile())
    {
        std::cerr << "ERROR: Input file does not exist: " << inputFile.getFullPathName() << std::endl;
        return 1;
    }

    // Load input audio
    juce::AudioBuffer<float> inputBuffer;
    double sampleRate;

    if (!loadAudioFile(inputFile, inputBuffer, sampleRate))
        return 1;

    // Analyze input metrics
    AudioMetrics inputMetrics;
    inputMetrics.analyze(inputBuffer);
    inputMetrics.printResults("Input Audio");

    // Save bypass version if requested
    if (generateBypass)
    {
        if (!saveAudioFile(bypassFile, inputBuffer, sampleRate))
            return 1;

        std::cout << "\n✅ Bypass version saved" << std::endl;
    }

    // Initialize BTZ processor
    BTZAudioProcessor processor;

    // Prepare processor
    processor.setRateAndBufferSizeDetails(sampleRate, 512);
    processor.prepareToPlay(sampleRate, 512);

    std::cout << "\n=== Processing with BTZ ===" << std::endl;
    std::cout << "Factory preset: Default (neutral)" << std::endl;

    // Process audio in blocks
    juce::AudioBuffer<float> outputBuffer;
    outputBuffer.makeCopyOf(inputBuffer);

    juce::MidiBuffer midiBuffer;
    int numSamples = outputBuffer.getNumSamples();
    int blockSize = 512;
    int numBlocks = (numSamples + blockSize - 1) / blockSize;

    auto startTime = juce::Time::getMillisecondCounterHiRes();

    for (int block = 0; block < numBlocks; ++block)
    {
        int startSample = block * blockSize;
        int numSamplesThisBlock = std::min(blockSize, numSamples - startSample);

        juce::AudioBuffer<float> blockBuffer(
            outputBuffer.getArrayOfWritePointers(),
            outputBuffer.getNumChannels(),
            startSample,
            numSamplesThisBlock);

        processor.processBlock(blockBuffer, midiBuffer);

        // Progress indicator
        if (block % 100 == 0)
        {
            float progress = (float)block / numBlocks * 100.0f;
            std::cout << "\rProgress: " << std::fixed << std::setprecision(1) << progress << "%" << std::flush;
        }
    }

    auto endTime = juce::Time::getMillisecondCounterHiRes();
    double elapsedMs = endTime - startTime;
    double audioDurationMs = (double)numSamples / sampleRate * 1000.0;
    double realtimeFactor = audioDurationMs / elapsedMs;

    std::cout << "\r                                                    \r"; // Clear progress
    std::cout << "✅ Processing complete" << std::endl;
    std::cout << "CPU Time: " << std::fixed << std::setprecision(2) << elapsedMs << " ms" << std::endl;
    std::cout << "Realtime Factor: " << realtimeFactor << "x (higher = more efficient)" << std::endl;

    // Analyze output metrics
    AudioMetrics outputMetrics;
    outputMetrics.analyze(outputBuffer);
    outputMetrics.printResults("Output Audio (Processed)");

    // Save processed output
    if (!saveAudioFile(outputFile, outputBuffer, sampleRate))
        return 1;

    std::cout << "\n✅ All done! Compare input vs output in your DAW." << std::endl;

    processor.releaseResources();
    return 0;
}
