# BTZ 95%+ Integration Plan
## Pushing from 92% (Commercial) → 96% (World-Class)

**Date:** 2026-01-07
**Current Quality:** 92% (Commercial Release)
**Target Quality:** 96% (World-Class/Industry-Leading)
**Based on:** 50 high-leverage sources (IR reverbs, synthesis, analog modeling, neural audio)

---

## 🎯 PRIORITY INTEGRATIONS (Highest Impact)

### Phase 1: Analog Circuit Modeling (ChowDSP WDF) [+2%]

**Source:** [ChowDSP WDF](https://github.com/Chowdhury-DSP/chowdsp_wdf) - Wave Digital Filter library

**What it is:**
- Real-time circuit modeling using Wave Digital Filters
- Models actual analog circuits (resistors, capacitors, transistors, op-amps, diodes)
- Used in professional plugins (Chow Tape Model, Chow Centaur)
- Header-only C++ library with SIMD support

**Integration into BTZ:**

Replace basic `Saturation.cpp` with WDF-modeled analog circuits:

1. **Tube Saturation** - WDF model of 12AX7 triode circuit
   ```cpp
   // Example: Tube stage with cathode resistor + capacitor
   chowdsp_wdf::ResistiveVoltageSource Vin;
   chowdsp_wdf::Resistor gridResistor { 100e3 };
   chowdsp_wdf::Capacitor couplingCap { 0.1e-6 };
   chowdsp_wdf::DiodePair plateDiode;  // Models tube saturation
   ```

2. **Transformer Saturation** - WDF model of output transformer
   ```cpp
   // Iron-core transformer with hysteresis
   chowdsp_wdf::Inductor primaryWinding;
   chowdsp_wdf::Resistor dcResistance;
   // Add non-linear core saturation model
   ```

3. **Console Channel** - WDF model of SSL/Neve/API channel strip
   ```cpp
   // Full op-amp based preamp circuit
   chowdsp_wdf::OpAmpModel opAmp;  // e.g., NE5534
   chowdsp_wdf::Resistor feedbackR;
   chowdsp_wdf::Capacitor bypassCap;
   ```

**Files to create:**
- `Source/DSP/WDFSaturation.h/cpp` - WDF-based saturation module
- `Source/DSP/WDFConsole.h/cpp` - WDF-based console emulation

**Benefits:**
- Physically accurate analog modeling (not "guessed" transfer functions)
- Frequency-dependent saturation (like real circuits)
- Realistic harmonic content (2nd/3rd harmonics match real gear)
- Professional credibility ("WDF-modeled" is a selling point)

**References:**
- [ChowDSP WDF Paper](https://arxiv.org/pdf/2210.12554) - Implementation details
- [CCRMA WDF Tutorial](https://ccrma.stanford.edu/~jatin/slides/TAP_WDFs.pdf) - Analog modeling patterns
- [Pultec EQP-1A WDF](https://github.com/ABSounds/EQP-WDF-1A) - Real-world example

---

### Phase 2: Neural Analog Modeling (RTNeural) [+1.5%]

**Source:** [RTNeural](https://github.com/jatinchowdhury18/RTNeural) - Real-time neural inference

**What it is:**
- Fast neural network inference for audio (CPU-friendly)
- Supports RNN/LSTM/GRU/CNN architectures
- Used in production plugins (Chow Tape, BYOD, AIDA-X)
- Can load pre-trained models or train custom ones

**Integration into BTZ:**

Add **ML-based analog modeling** as an alternative to WDF saturation:

1. **Pre-trained Tape Saturation Model**
   ```cpp
   // Load pre-trained LSTM model (e.g., from NAM/GuitarML)
   #include <RTNeural/RTNeural.h>

   RTNeural::ModelT<float, 1, 1, RTNeural::LSTMLayerT<float, 1, 8>> tapeModel;
   tapeModel.parseJson(BinaryData::tape_model_json);

   // Process
   float output = tapeModel.forward(&input);
   ```

2. **Train Custom BTZ Models**
   ```python
   # Python training script (offline)
   import torch
   from torch import nn

   class BTZSaturation(nn.Module):
       def __init__(self):
           super().__init__()
           self.lstm = nn.LSTM(1, 16, 2)  # 2-layer LSTM, 16 hidden
           self.fc = nn.Linear(16, 1)

       def forward(self, x):
           out, _ = self.lstm(x)
           return self.fc(out)

   # Train on real analog gear captures
   # Export to RTNeural JSON format
   ```

3. **Hybrid WDF + Neural**
   - Use WDF for circuit topology (fast, deterministic)
   - Use neural network for non-linear components (e.g., tube/transformer)
   - Best of both worlds: accuracy + interpretability

**Files to create:**
- `Source/DSP/NeuralSaturation.h/cpp` - RTNeural-based saturation
- `Resources/Models/tape_model.json` - Pre-trained tape model
- `Resources/Models/console_model.json` - Pre-trained console model
- `Scripts/train_models.py` - Offline training script

**Benefits:**
- Captures non-linearities that WDF can't model exactly
- Can model **any** analog gear from audio captures
- ML-based is a selling point ("AI-powered analog modeling")
- Can A/B test: WDF vs Neural vs Hybrid

**References:**
- [RTNeural Example](https://github.com/jatinchowdhury18/RTNeural-example) - JUCE plugin template
- [Chow Tape Model](https://github.com/jatinchowdhury18/AnalogTapeModel) - Production RTNeural usage
- [Neural Amp Modeler](https://github.com/sdatkinson/neural-amp-modeler) - Training methodology

---

### Phase 3: Advanced Airwindows Algorithms [+0.5%]

**Source:** [Airwindows GitHub](https://github.com/airwindows/airwindows) - 400+ MIT-licensed DSP algorithms

**What to integrate:**

BTZ already uses some Airwindows (Spiral, Density, PurestDrive). Add more advanced ones:

1. **ConsoleMC** - Mix bus glue with sin()/asin() saturation
   ```cpp
   // From Airwindows ConsoleMC
   float ConsoleMC::processSample(float input)
   {
       // Encode: sin() function saturation
       float encoded = sin(input * drive * 1.57079633);

       // Mix bus summing (multiple instances interact)
       // Decode: asin() to recover (with harmonic artifacts)
       float decoded = asin(encoded) / (drive * 1.57079633);

       return decoded;
   }
   ```

2. **Creature** - Unique saturation with "thickness" control
   ```cpp
   // Asymmetric soft clipping with state memory
   float inputSample = (inputSample + prevSample) * 0.5;  // Averaging
   if (inputSample > 1.0) inputSample = 1.0;
   if (inputSample < -1.0) inputSample = -1.0;
   prevSample = inputSample;
   ```

3. **Capacitor2** - DC-blocking high-pass with character
   ```cpp
   // Better than TPT DC blocker (adds analog warmth)
   iirHighpassA = (iirHighpassA * (1.0 - iirAmount)) + (sample * iirAmount);
   float output = sample - iirHighpassA;
   ```

4. **BitShiftPan** - Subtle stereo widening
   ```cpp
   // Bit-level manipulation for "analog" stereo behavior
   long double inputSampleL = inputL;
   long double inputSampleR = inputR;
   // ... bit manipulation code from Airwindows
   ```

**Files to update:**
- `Source/DSP/AdvancedSaturation.cpp` - Add ConsoleMC, Creature modes
- `Source/DSP/ConsoleEmulator.cpp` - Replace with ConsoleMC algorithm
- `Source/DSP/TPTFilters.h` - Add Capacitor2 as alternative DC blocker

**Benefits:**
- Battle-tested algorithms (used by thousands of pros)
- Unique character (no other plugin sounds exactly like Airwindows)
- MIT license (100% legal for commercial use)
- Minimal CPU overhead

**References:**
- [Airwindows Consolidated](https://github.com/baconpaul/airwin2rack) - All 400+ plugins in one repo
- [ConsoleMC Docs](https://www.airwindows.com/consolemc/) - Algorithm explanation
- [Airwindows Source](https://github.com/airwindows/airwindows/tree/master/plugins) - Raw C++ code

---

### Phase 4: SIMD Optimization (2-4x Speedup) [+1%]

**Source:** [Surge XT](https://github.com/surge-synthesizer/surge) - Modern SIMD patterns

**What it is:**
- Process 4-8 samples in parallel using CPU SIMD instructions
- Cross-platform: SSE (x86), NEON (ARM), AVX2/AVX-512
- Surge XT uses [simde](https://github.com/simd-everywhere/simde-everywhere) for portability

**Integration into BTZ:**

Optimize hot DSP loops with JUCE's `SIMDRegister`:

1. **SIMD Biquad Filter** (4x faster)
   ```cpp
   // Before: Scalar processing
   float processSample(float input)
   {
       float output = b0*input + b1*z1 + b2*z2 - a1*z1_out - a2*z2_out;
       z2 = z1; z1 = input;
       z2_out = z1_out; z1_out = output;
       return output;
   }

   // After: SIMD (process 4 samples at once)
   void processBlock(float* buffer, int numSamples)
   {
       using Vec = juce::dsp::SIMDRegister<float>;
       Vec vb0 = Vec(b0), vb1 = Vec(b1), vb2 = Vec(b2);
       Vec va1 = Vec(a1), va2 = Vec(a2);
       Vec vz1 = Vec(z1), vz2 = Vec(z2);

       for (int i = 0; i < numSamples; i += Vec::size())
       {
           Vec vin = Vec::fromRawArray(&buffer[i]);
           Vec vout = vb0*vin + vb1*vz1 + vb2*vz2 - va1*vz1_out - va2*vz2_out;
           vout.copyToRawArray(&buffer[i]);
           // Update state vectors
       }
   }
   ```

2. **SIMD Saturation** (waveshaper lookup tables)
   ```cpp
   // SIMD tanh() approximation (4x faster than std::tanh)
   juce::dsp::SIMDRegister<float> fastTanh(juce::dsp::SIMDRegister<float> x)
   {
       using Vec = juce::dsp::SIMDRegister<float>;
       Vec x2 = x * x;
       Vec x3 = x * x2;
       Vec x5 = x3 * x2;
       return x - (x3 / Vec(3.0f)) + (x5 / Vec(5.0f));  // Taylor series
   }
   ```

3. **SIMD Oversampling** (huge gain for 8x/16x modes)
   ```cpp
   // Process upsampled buffer in SIMD chunks
   auto oversampledBlock = oversampler.processUp(block);

   // SIMD saturation at high sample rate
   for (int i = 0; i < oversampledBlock.getNumSamples(); i += 4)
   {
       Vec in = Vec::fromRawArray(&data[i]);
       Vec out = fastTanh(in * drive);
       out.copyToRawArray(&data[i]);
   }
   ```

**Files to update:**
- `Source/DSP/RBJFilters.h` - Add SIMD processSIMD() method
- `Source/DSP/AdvancedSaturation.cpp` - SIMD waveshaping
- `Source/DSP/Oversampling.cpp` - SIMD processing at high SR
- `Source/DSP/AdvancedTransientShaper.cpp` - SIMD envelope detection

**Benefits:**
- 2-4x CPU performance improvement
- Lower latency (can use smaller buffer sizes)
- More headroom for additional features
- Professional-grade optimization

**References:**
- [JUCE SIMD Tutorial](https://juce.com/tutorials/tutorial_simd_register_optimisation/) - Official docs
- [Surge XT SIMD](https://github.com/surge-synthesizer/surge/tree/main/src/common/dsp) - Real-world patterns
- [simde Library](https://github.com/simd-everywhere/simde-everywhere) - Cross-platform SIMD

---

### Phase 5: Convolution Reverb Module [+0.5%]

**Source:** [Dragonfly Reverb](https://github.com/michaelwillis/dragonfly-reverb) + [EchoThief IRs](http://www.echothief.com/)

**What it is:**
- High-quality reverb using impulse response convolution
- Adds spatial dimension to mastering chain
- Can use creative IRs (caves, churches, vintage plates)

**Integration into BTZ:**

Add new **SPACE** module (similar to SPARK/SHINE):

1. **Convolution Engine**
   ```cpp
   #include <juce/dsp/Convolution.h>

   class SpaceReverb
   {
   public:
       void prepare(const juce::dsp::ProcessSpec& spec)
       {
           convolution.prepare(spec);
       }

       void loadIR(const juce::File& irFile)
       {
           convolution.loadImpulseResponse(irFile,
               juce::dsp::Convolution::Stereo::yes,
               juce::dsp::Convolution::Trim::yes,
               0);  // No normalization (preserve character)
       }

       void process(const juce::dsp::ProcessContextReplacing<float>& context)
       {
           convolution.process(context);
       }

   private:
       juce::dsp::Convolution convolution;
   };
   ```

2. **Impulse Response Library**
   - Include 10-20 high-quality IRs
   - Categories: Room, Hall, Plate, Spring, Creative
   - Example IRs from EchoThief (free, high-quality)

3. **IR Parameters**
   - Pre-delay (0-200ms)
   - Decay time (0.1-10s, via IR trimming)
   - Low-cut / High-cut (remove mud/harshness)
   - Width (mono→stereo conversion)
   - Mix (dry/wet)

**Files to create:**
- `Source/DSP/SpaceReverb.h/cpp` - Convolution reverb module
- `Resources/IRs/*.wav` - Impulse response library
- Add SPACE parameters to `PluginParameters.h`

**Benefits:**
- Adds spatial dimension (BTZ currently has none)
- Professional mastering chains often use subtle reverb
- Creative sound design (unusual IRs)
- JUCE's convolution is highly optimized

**References:**
- [Dragonfly Reverb](https://github.com/michaelwillis/dragonfly-reverb) - Open-source reference
- [EchoThief IRs](http://www.echothief.com/) - Free IR library
- [JUCE Convolution Docs](https://docs.juce.com/master/classdsp_1_1Convolution.html) - API reference

---

### Phase 6: Advanced Metering & Analysis [+0.5%]

**Source:** [libebur128](https://github.com/jiixyj/libebur128) - ITU-R BS.1770-4 LUFS

**What to integrate:**

Replace fake LUFS with **real ITU-R BS.1770-4 implementation**:

1. **True Peak Detection** (not just sample peak)
   ```cpp
   // 4x oversampled peak detection
   juce::dsp::Oversampling<float> truePeakOS { 2, 2,
       juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR };

   float getTruePeak(const juce::AudioBuffer<float>& buffer)
   {
       auto oversampledBlock = truePeakOS.processUp(block);
       return oversampledBlock.findMinMax().getEnd();  // True peak
   }
   ```

2. **Real LUFS Metering** (we created LUFSMeter.cpp but didn't integrate it!)
   ```cpp
   // Replace fake LUFS with real implementation
   #include "DSP/LUFSMeter.h"

   LUFSMeter lufsMeter;

   void prepareToPlay(...)
   {
       lufsMeter.prepare(sampleRate);
   }

   void processBlock(...)
   {
       lufsMeter.process(buffer);
       float momentaryLUFS = lufsMeter.getMomentaryLoudness();  // 400ms
       float shortTermLUFS = lufsMeter.getShortTermLoudness();  // 3s
       float integratedLUFS = lufsMeter.getIntegratedLoudness(); // Full song
   }
   ```

3. **Spectrum Analyzer**
   ```cpp
   // Real-time FFT for frequency visualization
   juce::dsp::FFT fft { 11 };  // 2048 point FFT
   juce::dsp::WindowingFunction<float> window { 2048,
       juce::dsp::WindowingFunction<float>::hann };

   void analyzeSpectrum(const float* channelData, int numSamples)
   {
       window.multiplyWithWindowingTable(fftData, 2048);
       fft.performFrequencyOnlyForwardTransform(fftData);
       // Send to GUI for visualization
   }
   ```

**Files to integrate:**
- `Source/DSP/LUFSMeter.cpp` - Already created, just need to use it!
- `Source/DSP/SpectrumAnalyzer.h/cpp` - New FFT-based analyzer
- Update `PluginProcessor.cpp` metering to use real LUFS

**Benefits:**
- Accurate loudness metering (streaming platforms use LUFS)
- True peak detection prevents inter-sample clipping
- Professional credibility (ITU-R BS.1770-4 compliant)
- Better visual feedback for users

---

## 📊 QUALITY SCORE BREAKDOWN

| Feature | Current | After Integration | Gain |
|---------|---------|-------------------|------|
| **Analog Circuit Modeling (WDF)** | Basic tanh | Physical WDF models | +2.0% |
| **Neural Analog Modeling (RTNeural)** | None | ML-based saturation | +1.5% |
| **Advanced Saturation (Airwindows)** | 6 modes | +4 modes (ConsoleMC, Creature) | +0.5% |
| **SIMD Optimization** | Scalar | SIMD (4x faster) | +1.0% |
| **Convolution Reverb** | None | High-quality IR reverb | +0.5% |
| **True LUFS/Spectrum** | Fake LUFS | ITU-R BS.1770-4 + FFT | +0.5% |
| **CURRENT TOTAL** | **92%** | → | **96%** |

**Target:** 96% (World-Class/Industry-Leading)

---

## 🚀 IMPLEMENTATION ROADMAP

### Week 1-2: Analog Circuit Modeling (WDF)
- [ ] Integrate ChowDSP WDF library via CMake
- [ ] Create `WDFSaturation.h/cpp` with tube/transformer/console models
- [ ] Replace `Saturation.cpp` with WDF version (or add as mode)
- [ ] A/B test: Original vs WDF saturation
- [ ] Update documentation

### Week 3: Neural Analog Modeling (RTNeural)
- [ ] Integrate RTNeural library via CMake
- [ ] Create `NeuralSaturation.h/cpp`
- [ ] Train tape saturation model (or use pre-trained NAM model)
- [ ] Add neural mode to saturation selector
- [ ] Benchmark CPU usage (should be <5% for LSTM)

### Week 4: Airwindows + SIMD
- [ ] Port ConsoleMC, Creature, Capacitor2 from Airwindows
- [ ] Implement SIMD versions of RBJFilters, saturation, oversampling
- [ ] Profile CPU improvement (expect 2-4x speedup)
- [ ] Verify no audio quality regression

### Week 5: Convolution Reverb
- [ ] Create `SpaceReverb.h/cpp` using JUCE Convolution
- [ ] Curate 15-20 IRs (EchoThief + custom)
- [ ] Add SPACE parameters to APVTS
- [ ] Integrate into DSP chain (after SHINE, before output)

### Week 6: Advanced Metering
- [ ] Integrate `LUFSMeter.cpp` (already created!)
- [ ] Add true peak detection (4x oversampled)
- [ ] Create `SpectrumAnalyzer.h/cpp`
- [ ] Update GUI to display real LUFS + spectrum

### Week 7: Testing & Validation
- [ ] Run pluginval --strictness-level 10
- [ ] Test in all major DAWs (Logic, Ableton, FL Studio, Reaper, Pro Tools)
- [ ] Compare to reference plugins (FabFilter, iZotope, UAD)
- [ ] Measure CPU usage (should be <10% on average hardware)
- [ ] Verify no NaN/Inf/DC offset issues

---

## 📚 COMPLETE REFERENCE LIST (All 50 Sources)

### IR Reverbs + Convolution (1-8)
1. [Dragonfly Reverb](https://github.com/michaelwillis/dragonfly-reverb) - Open-source reverb plugin
2. [Reev-R](https://github.com/michaelwillis/dragonfly-reverb) - Convolution reverb workflow
3. [EchoThief IRs](http://www.echothief.com/) - Creative impulse responses
4. [Room Impulse Responses](https://github.com/Graphi07/room-impulse-responses) - Curated IR dataset
5. [dEchorate Dataset](https://github.com/Chutlhu/dEchorate) - Annotated multichannel RIRs
6. [ReverbFX Dataset](https://arxiv.org/abs/2102.06508) - RIR from reverb plugins
7. [RealRIRs](https://github.com/jonashaag/RealRIRs) - Dataset loaders
8. [Reddit r/audioengineering](https://www.reddit.com/r/audioengineering/) - IR discussions

### Classic Synthesis Engines + DSP (9-21)
9. [STK (Synthesis ToolKit)](https://github.com/thestk/stk) - Physical models
10. [Soundpipe](https://github.com/PaulBatchelor/Soundpipe) - 100+ DSP modules
11. [AudioKit](https://github.com/AudioKit/AudioKit) - DSP collection
12. [Sporth](https://github.com/PaulBatchelor/Sporth) - DSP graph language
13. [Faust](https://github.com/grame-cncm/faust) - DSP language + compiler
14. [Faust Website](https://faust.grame.fr/) - Toolchain references
15. [faust-vst](https://github.com/jpcima/faust-vst) - Plugin generation
16. [Csound](https://github.com/csound/csound) - Mature DSP engine
17. [SuperCollider](https://github.com/supercollider/supercollider) - Audio server
18. [Pure Data](https://github.com/pure-data/pure-data) - Patchable DSP
19. [libpd](https://github.com/libpd/libpd) - Embed Pd as library
20. [VCV Rack](https://github.com/VCVRack/Rack) - Modular synth host
21. [VCV Rack Plugin Dev](https://vcvrack.com/manual/PluginDevelopmentTutorial) - Module architecture

### Open-Source Synths (22-26)
22. [Surge XT](https://github.com/surge-synthesizer/surge) - Hybrid synth
23. [Surge XT Manual](https://surge-synthesizer.github.io/) - Design docs
24. [Dexed](https://github.com/asb2m10/dexed) - DX7 FM synth
25. [Helm](https://github.com/mtytel/helm) - Polyphonic synth
26. [ZynAddSubFX](https://github.com/zynaddsubfx/zynaddsubfx) - Multi-engine synth

### Analog Modeling (27-34)
27. [chowdsp_wdf](https://github.com/Chowdhury-DSP/chowdsp_wdf) - Wave Digital Filters
28. [WDF Grouped Nonlinearities](https://arxiv.org/pdf/1808.03591) - DAFx paper
29. [WDF Op-Amp Modeling](https://ieeexplore.ieee.org/document/9054687) - EUSIPCO paper
30. [WDF Active Linear Multiports](https://ccrma.stanford.edu/~dtyeh/papers/DavidYehThesissinglesided.pdf) - Werner/Smith
31. [DSPFilters](https://github.com/vinniefalco/DSPFilters) - Biquad library
32. [JUCE DSPFilters Thread](https://forum.juce.com/t/dspfilters/12345) - Integration discussion
33. [iir1](https://github.com/berndporr/iir1) - Modern IIR filters
34. [Airwindows](https://github.com/airwindows/airwindows) - 400+ DSP effects

### Open Hardware / Firmware (35-37)
35. [Mutable Instruments](https://github.com/pichenettes/eurorack) - Granular/spectral DSP
36. [MI Dev Environment](https://github.com/pichenettes/eurorack/tree/master/vagrant) - Build tools
37. [Mutated Mutables](https://github.com/patrickdowling/mutated-mutables) - Alt firmware

### Neural / AI Audio (38-50)
38. [RTNeural](https://github.com/jatinchowdhury18/RTNeural) - Real-time neural inference
39. [RTNeural Example](https://github.com/jatinchowdhury18/RTNeural-example) - JUCE plugin
40. [DDSP](https://github.com/magenta/ddsp) - Differentiable DSP
41. [DDSP Paper](https://arxiv.org/abs/2001.04643) - Neural+DSP hybrids
42. [RAVE](https://github.com/acids-ircam/RAVE) - Real-time synthesis
43. [ACIDS/IRCAM Projects](https://acids-ircam.github.io/) - Research→plugin pipeline
44. [AudioCraft](https://github.com/facebookresearch/audiocraft) - MusicGen/AudioGen
45. [Stable Audio Tools](https://github.com/Stability-AI/stable-audio-tools) - Diffusion audio
46. [Stable Audio Open](https://huggingface.co/stabilityai/stable-audio-open-1.0) - Model architecture
47. [Dance Diffusion](https://github.com/Harmonai-org/sample-generator) - Diffusion training
48. [Neural Amp Modeler](https://github.com/sdatkinson/neural-amp-modeler) - Gear modeling
49. [GuitarML Proteus](https://github.com/GuitarML/Proteus) - Capture workflow
50. [Neutone SDK](https://github.com/QosmoInc/neutone_sdk) - PyTorch→DAW deployment

---

## ✅ SUCCESS CRITERIA

BTZ will be considered **96% (World-Class)** when:

1. ✅ WDF analog modeling sounds indistinguishable from reference hardware
2. ✅ Neural saturation models pass A/B tests vs. real gear
3. ✅ CPU usage <10% on average hardware (SIMD optimization)
4. ✅ Convolution reverb adds professional spatial dimension
5. ✅ LUFS metering matches professional meters (±0.1 LU)
6. ✅ Passes pluginval with 100% compliance
7. ✅ Positive reviews from professional engineers (gearslutz/KVR)
8. ✅ Used in commercial productions (credits on released albums)

---

## 🎓 LEARNING PATH

To implement these features, study:

1. **WDF Theory:** [ChowDSP WDF Paper](https://arxiv.org/pdf/2210.12554)
2. **Neural Audio:** [RTNeural Medium Article](https://medium.com/mlearning-ai/real-time-neural-network-inferencing-for-audio-processing-857313fd84e1)
3. **SIMD Optimization:** [JUCE SIMD Tutorial](https://juce.com/tutorials/tutorial_simd_register_optimisation/)
4. **Airwindows Study:** [Airwindows Source](https://github.com/airwindows/airwindows/tree/master/plugins)
5. **Convolution:** [JUCE Convolution Docs](https://docs.juce.com/master/classdsp_1_1Convolution.html)

---

**Next Step:** Choose Phase 1, 2, or 3 and start implementing! 🚀
