import React, { useState, useEffect, useCallback } from 'react';
import { ModernKnobWithSpectrum } from './ModernKnobWithSpectrum';
import { ToggleButton } from './ToggleButton';
import { CentralVisualizer } from './CentralVisualizer';
import { PresetBrowser } from './PresetBrowser';
import { AIAutomationPanel } from './AIAutomationPanel';
import { EnhancedClippingControls } from './EnhancedClippingControls';
import { ProcessingChainVisualizer } from './ProcessingChainVisualizer';
import { AdvancedMeterPanel } from './AdvancedMeterPanel';
import { BTZPluginState, EnhancedPreset } from './types';
import { cn } from '@/lib/utils';

const DEFAULT_PRESET: EnhancedPreset = {
  id: 'default',
  label: 'Default',
  state: {
    punch: 0,
    warmth: 0,
    boom: 0,
    mix: 1.0,
    drive: 0,
    texture: false,
    active: true,
    oversampling: false,
    clippingType: 'soft',
    clippingBlend: 0.5,
    clippingEnabled: false,
    lufsTarget: -14,
    aiEnhance: false,
    timbralTransfer: false,
    aiAutomation: true,
    gateThreshold: -40,
    transientAmount: 0,
    saturationAmount: 0,
    subHarmonics: false,
    consoleGlue: true,
    oversamplingRate: 4
  }
};

// MVP Specification Presets - Professional drum enhancement presets 💕
const PERFORMANCE_PRESETS: EnhancedPreset[] = [
  DEFAULT_PRESET,
  {
    id: 'mvp-loud-clean',
    label: 'MVP (-8 LUFS)', 
    state: { 
      punch: 0.7, warmth: 0.5, boom: 0.4, mix: 1.0, drive: 0.8, texture: false, 
      active: true, oversampling: true, clippingType: 'soft', clippingBlend: 0.6,
      clippingEnabled: true, aiAutomation: true, gateThreshold: -25, transientAmount: 0.6,
      saturationAmount: 0.4, subHarmonics: true, consoleGlue: true, lufsTarget: -8
    },
  },
  {
    id: 'streaming-safe',
    label: 'Streaming Safe (-14 LUFS)',
    state: { 
      punch: 0.5, warmth: 0.4, boom: 0.3, mix: 1.0, drive: 0.4, texture: false, 
      active: true, oversampling: false, clippingType: 'soft', clippingBlend: 0.3,
      clippingEnabled: false, aiAutomation: true, gateThreshold: -40, transientAmount: 0.4,
      saturationAmount: 0.2, subHarmonics: false, consoleGlue: true, lufsTarget: -14
    },
  },
  {
    id: 'punchy-kick',
    label: 'Punchy Kick',
    state: { 
      punch: 0.9, warmth: 0.3, boom: 0.8, mix: 0.9, drive: 0.5, texture: false, 
      active: true, oversampling: false, clippingType: 'hard', clippingBlend: 0.4,
      clippingEnabled: true, aiAutomation: true, gateThreshold: -30, transientAmount: 0.9,
      saturationAmount: 0.2, subHarmonics: true, consoleGlue: true, lufsTarget: -10
    },
  },
  {
    id: 'silky-snare',
    label: 'Silky Snare',
    state: { 
      punch: 0.8, warmth: 0.6, boom: 0.2, mix: 0.95, drive: 0.4, texture: true, 
      active: true, oversampling: true, clippingType: 'tube', clippingBlend: 0.5,
      clippingEnabled: false, aiAutomation: true, gateThreshold: -35, transientAmount: 0.7,
      saturationAmount: 0.5, subHarmonics: false, consoleGlue: true, lufsTarget: -12
    },
  },
  {
    id: 'room-glue',
    label: 'Room Glue',
    state: { 
      punch: 0.4, warmth: 0.7, boom: 0.5, mix: 0.8, drive: 0.6, texture: true, 
      active: true, oversampling: false, clippingType: 'tape', clippingBlend: 0.6,
      clippingEnabled: true, aiAutomation: true, gateThreshold: -45, transientAmount: 0.3,
      saturationAmount: 0.6, subHarmonics: true, consoleGlue: true, lufsTarget: -11
    },
  },
  {
    id: 'tape-warmth',
    label: 'Tape Warmth',
    state: { 
      punch: 0.6, warmth: 0.9, boom: 0.6, mix: 0.9, drive: 0.7, texture: true, 
      active: true, oversampling: false, clippingType: 'tape', clippingBlend: 0.8,
      clippingEnabled: true, aiAutomation: true, gateThreshold: -40, transientAmount: 0.5,
      saturationAmount: 0.8, subHarmonics: true, consoleGlue: true, lufsTarget: -9
    },
  },
  {
    id: 'boom-sculpt',
    label: 'Boom Sculpt',
    state: { 
      punch: 0.7, warmth: 0.4, boom: 0.9, mix: 0.85, drive: 0.8, texture: false, 
      active: true, oversampling: true, clippingType: 'soft', clippingBlend: 0.5,
      clippingEnabled: true, aiAutomation: true, gateThreshold: -25, transientAmount: 0.6,
      saturationAmount: 0.3, subHarmonics: true, consoleGlue: true, lufsTarget: -7
    },
  },
  {
    id: 'billboard-loud',
    label: 'Billboard Loud (-6 LUFS)',
    state: { 
      punch: 0.9, warmth: 0.6, boom: 0.5, mix: 1.0, drive: 0.95, texture: false, 
      active: true, oversampling: true, clippingType: 'soft', clippingBlend: 0.8,
      clippingEnabled: true, aiAutomation: true, gateThreshold: -18, transientAmount: 0.8,
      saturationAmount: 0.4, subHarmonics: true, consoleGlue: true, lufsTarget: -6
    },
  }
];

export const EnhancedBTZPlugin: React.FC = () => {
  const [state, setState] = useState<BTZPluginState>(DEFAULT_PRESET.state);
  const [viewMode, setViewMode] = useState<'primary' | 'advanced' | 'engineering'>('primary');
  const [meters, setMeters] = useState({
    inputLevel: 0,
    outputLevel: 0,
    spectrumData: new Float32Array(64),
    waveformData: new Float32Array(128),
    lufsIntegrated: -14.2,
    truePeak: -2.1,
    isProcessing: false,
    analysisData: {
      transientStrength: 0,
      lowEndEnergy: 0,
      loudnessScore: 0,
      richness: 0,
      spectralCentroid: 1000
    }
  });

  // Generate live audio visualization data with AI analysis
  useEffect(() => {
    const interval = setInterval(() => {
      if (!state.active) {
        setMeters(prev => ({ ...prev, isProcessing: false }));
        return;
      }

      const baseLevel = 0.2 + Math.random() * 0.5;
      let processedLevel = baseLevel * (1 + state.drive * 0.8) * state.mix;
      
      // AI Analysis simulation (DeepFilterNet + Neutone style)
      const transientStrength = Math.min(1, baseLevel * 2 * (state.punch + 0.3));
      const lowEndEnergy = Math.min(1, baseLevel * 1.5 * (state.boom + 0.2));  
      const richness = Math.min(1, (state.warmth + (state.texture ? 0.3 : 0)) * 1.2);
      const loudnessScore = Math.min(1, processedLevel * 1.1);
      const spectralCentroid = 1000 + richness * 3000; // Hz
      
      // AI Automation - adjust parameters based on analysis
      if (state.aiAutomation) {
        // Auto-adjust drive based on loudness target
        if (state.lufsTarget && state.lufsTarget < -10) {
          const targetGain = (-10 - state.lufsTarget) / 10; // More aggressive for loud targets
          processedLevel *= (1 + targetGain * 0.3);
        }
        
        // FL Studio style clipping simulation
        if (state.clippingEnabled) {
          const clippingAmount = (state.clippingBlend || 0.5) * 0.4;
          const threshold = 0.8;
          
          if (processedLevel > threshold) {
            const excess = processedLevel - threshold;
            let clippedExcess;
            
            switch (state.clippingType) {
              case 'soft':
                // FL Studio style soft limiting (tanh-based)
                clippedExcess = Math.tanh(excess * 3) / 3;
                break;
              case 'hard':
                clippedExcess = Math.min(excess, 0.15);
                break;
              case 'tube':
                clippedExcess = excess * Math.pow(Math.E, -excess * 2);
                break;
              case 'tape':
                clippedExcess = excess / (1 + excess * 1.5);
                break;
              case 'digital':
                clippedExcess = excess * 0.7;
                break;
              default:
                clippedExcess = Math.tanh(excess * 2) / 2;
            }
            
            const clipped = threshold + clippedExcess;
            processedLevel = processedLevel * (1 - clippingAmount) + clipped * clippingAmount;
          }
        }
      }
      
      // Generate realistic spectrum for each processing stage
      const spectrumData = new Float32Array(64);
      const waveformData = new Float32Array(128);
      
      for (let i = 0; i < 64; i++) {
        const freq = (i / 64) * 20000;
        let magnitude = Math.random() * 0.1;
        
        // Shape spectrum based on controls and AI analysis
        if (freq < 200) magnitude += state.boom * 0.6 + (lowEndEnergy * 0.2);
        if (freq > 80 && freq < 800) magnitude += state.punch * 0.5 + (transientStrength * 0.3);
        if (freq > 1000 && freq < 8000) magnitude += state.warmth * 0.4 + (richness * 0.2);
        if (freq > 8000) magnitude += state.texture ? 0.3 : 0.1;
        if (state.clippingEnabled) magnitude += 0.1; // Harmonic content from clipping
        
        spectrumData[i] = Math.max(0, Math.min(1, magnitude * (1 + state.drive * 0.3)));
      }
      
      // Generate waveform with more character
      for (let i = 0; i < 128; i++) {
        const t = (i / 128) * Math.PI * 4;
        let wave = Math.sin(t) * processedLevel * (0.8 + Math.random() * 0.4);
        
        // Add harmonic content from processing
        wave += Math.sin(t * 2) * state.warmth * 0.1 * processedLevel;
        wave += Math.sin(t * 3) * (state.texture ? 0.05 : 0) * processedLevel;
        
        waveformData[i] = wave;
      }

      setMeters({
        inputLevel: baseLevel,
        outputLevel: processedLevel,
        spectrumData,
        waveformData,
        lufsIntegrated: (state.lufsTarget || -14) + (Math.random() - 0.5) * 2,
        truePeak: Math.max(-1.0, (state.lufsTarget || -14) + 12 + (Math.random() - 0.5) * 3),
        isProcessing: processedLevel > 0.1,
        analysisData: {
          transientStrength,
          lowEndEnergy,
          loudnessScore,
          richness,
          spectralCentroid
        }
      });
    }, 60);

    return () => clearInterval(interval);
  }, [state]);

  const updateParameter = useCallback((key: keyof BTZPluginState, value: any) => {
    setState(prev => ({ ...prev, [key]: value }));
  }, []);

  const applyPreset = useCallback((preset: EnhancedPreset) => {
    setState(preset.state);
  }, []);

  return (
    <div className="w-full max-w-7xl mx-auto rounded-3xl border border-audio-primary/20 overflow-hidden"
         style={{ 
           background: 'var(--gradient-main)',
           boxShadow: 'var(--shadow-panel), 0 0 60px hsl(var(--audio-primary) / 0.1)'
         }}>
      
      {/* Header with view toggle */}
      <div className="relative border-b border-audio-primary/20 p-6 bg-gradient-to-r from-plugin-surface/50 to-plugin-panel/50">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-8">
            <div className="relative">
              <h1 className="text-4xl font-black text-transparent bg-gradient-to-r from-audio-primary to-audio-secondary bg-clip-text tracking-wider"
                  style={{ 
                    filter: 'drop-shadow(0 0 20px hsl(var(--audio-primary) / 0.5))',
                    fontFamily: 'var(--font-display)'
                  }}>
                BTZ
              </h1>
              <div className="absolute -top-2 -right-2 w-3 h-3 rounded-full bg-audio-primary animate-pulse"
                   style={{ boxShadow: '0 0 10px hsl(var(--audio-primary))' }}></div>
            </div>
            <div>
              <h2 className="text-lg font-bold text-foreground tracking-wide">BOX TONE ZONE</h2>
              <h3 className="text-xs text-foreground/70 tracking-[0.2em] uppercase">Enhanced Audio Processor</h3>
            </div>
          </div>
          
          <div className="flex items-center gap-4">
            {/* View Mode Toggle */}
            <div className="flex bg-plugin-surface rounded-full p-1 border border-audio-primary/20">
              <button
                onClick={() => setViewMode('primary')}
                className={cn(
                  "px-4 py-2 rounded-full text-xs font-medium transition-all duration-300",
                  viewMode === 'primary'
                    ? "bg-audio-primary text-background shadow-lg"
                    : "text-foreground/70 hover:text-foreground"
                )}
              >
                PERFORMANCE
              </button>
              <button
                onClick={() => setViewMode('advanced')}
                className={cn(
                  "px-4 py-2 rounded-full text-xs font-medium transition-all duration-300",
                  viewMode === 'advanced'
                    ? "bg-audio-secondary text-background shadow-lg"
                    : "text-foreground/70 hover:text-foreground"
                )}
              >
                SOUND DESIGN
              </button>
              <button
                onClick={() => setViewMode('engineering')}
                className={cn(
                  "px-4 py-2 rounded-full text-xs font-medium transition-all duration-300",
                  viewMode === 'engineering'
                    ? "bg-audio-tertiary text-background shadow-lg"
                    : "text-foreground/70 hover:text-foreground"
                )}
              >
                ENGINEERING
              </button>
            </div>
            
            {/* Power Button */}
            <ToggleButton 
              value={state.active} 
              onChange={(v) => updateParameter('active', v)} 
              label="POWER"
              className={cn(
                "px-8 py-3 rounded-full border-2 font-bold transition-all duration-300",
                state.active 
                  ? "bg-audio-primary border-audio-primary text-background shadow-[0_0_20px_hsl(var(--audio-primary))]" 
                  : "bg-plugin-raised/50 border-plugin-raised hover:bg-plugin-raised text-foreground/70"
              )}
            />
          </div>
        </div>
      </div>

      {/* Content Area */}
      <div className="p-8">
        {viewMode === 'primary' ? (
          // PRIMARY VIEW - Performance Mode
          <div className="space-y-12">
            {/* Central Visualizer - Smaller size to fit better */}
            <div className="flex justify-center mb-6">
              <CentralVisualizer 
                spectrumData={meters.spectrumData}
                waveformData={meters.waveformData}
                isProcessing={meters.isProcessing}
                level={meters.outputLevel}
              />
            </div>

            {/* Main Controls - 5 Knobs with live visualization */}
            <div className="grid grid-cols-5 gap-4 justify-items-center max-w-4xl mx-auto">
              <ModernKnobWithSpectrum 
                value={state.punch} 
                onChange={(v) => updateParameter('punch', v)} 
                label="PUNCH" 
                min={0} 
                max={1}
                size="sm"
                spectrumData={meters.spectrumData.slice(0, 16)}
                color="hsl(var(--audio-primary))"
              />
              <ModernKnobWithSpectrum 
                value={state.warmth} 
                onChange={(v) => updateParameter('warmth', v)} 
                label="WARMTH" 
                min={0} 
                max={1}
                size="sm"
                spectrumData={meters.spectrumData.slice(16, 32)}
                color="hsl(var(--audio-secondary))"
              />
              <ModernKnobWithSpectrum 
                value={state.boom} 
                onChange={(v) => updateParameter('boom', v)} 
                label="BOOM" 
                min={0} 
                max={1}
                size="md"
                spectrumData={meters.spectrumData.slice(0, 8)}
                color="hsl(var(--audio-tertiary))"
              />
              <ModernKnobWithSpectrum 
                value={state.mix} 
                onChange={(v) => updateParameter('mix', v)} 
                label="MIX" 
                min={0} 
                max={1}
                size="md"
                spectrumData={meters.spectrumData}
                color="hsl(var(--audio-success))"
              />
              <ModernKnobWithSpectrum 
                value={state.drive} 
                onChange={(v) => updateParameter('drive', v)} 
                label="DRIVE" 
                min={0} 
                max={1}
                size="md"
                spectrumData={meters.spectrumData.slice(32, 48)}
                color="hsl(var(--audio-warning))"
              />
            </div>

            {/* Texture & Clipping Controls */}
            <div className="flex justify-center items-center gap-8">
              <ToggleButton 
                value={state.texture} 
                onChange={(v) => updateParameter('texture', v)} 
                label="TEXTURE"
                className={cn(
                  "px-8 py-3 rounded-xl border-2 font-bold text-sm transition-all duration-300",
                  state.texture 
                    ? "bg-audio-tertiary border-audio-tertiary text-background shadow-[0_0_20px_hsl(var(--audio-tertiary))]" 
                    : "bg-plugin-raised/50 border-plugin-raised hover:bg-plugin-raised text-foreground/70"
                )}
              />
              
              <ToggleButton 
                value={!!state.clippingEnabled} 
                onChange={(v) => updateParameter('clippingEnabled', v)} 
                label="FL CLIPPER"
                className={cn(
                  "px-8 py-3 rounded-xl border-2 font-bold text-sm transition-all duration-300",
                  state.clippingEnabled 
                    ? "bg-audio-warning border-audio-warning text-background shadow-[0_0_20px_hsl(var(--audio-warning))]" 
                    : "bg-plugin-raised/50 border-plugin-raised hover:bg-plugin-raised text-foreground/70"
                )}
              />
            </div>

            {/* Presets */}
            <div className="flex justify-center">
              <PresetBrowser 
                presets={PERFORMANCE_PRESETS}
                onApplyPreset={applyPreset}
              />
            </div>

            {/* Output Metering - Hardware style display */}
            <div className="bg-plugin-surface rounded-2xl p-6 border border-foreground/10"
                 style={{
                   background: `linear-gradient(145deg, hsl(220, 15%, 8%), hsl(220, 12%, 12%))`,
                   boxShadow: `
                     inset 0 2px 8px rgba(0,0,0,0.8),
                     inset 0 -2px 4px rgba(255,255,255,0.05),
                     0 8px 32px rgba(0,0,0,0.6)
                   `
                 }}>
              <div className="flex items-center justify-between mb-4">
                <h3 className="text-lg font-bold text-foreground tracking-wide">OUTPUT</h3>
                <div className="flex gap-6 text-sm font-mono">
                  <span>LUFS: <span className="text-audio-primary font-bold">{meters.lufsIntegrated.toFixed(1)}</span></span>
                  <span>Peak: <span className="text-audio-secondary font-bold">{meters.truePeak > 0 ? '+' : ''}{meters.truePeak.toFixed(1)}dB</span></span>
                </div>
              </div>
              
              {/* Waveform Display - Output style with dark background and colorful wave */}
              <div className="h-24 rounded-lg overflow-hidden relative border border-foreground/20"
                   style={{
                     background: `linear-gradient(145deg, hsl(220, 20%, 4%), hsl(220, 15%, 8%))`,
                     boxShadow: `inset 0 4px 12px rgba(0,0,0,0.9)`
                   }}>
                <svg className="w-full h-full">
                  <defs>
                    <linearGradient id="outputWaveGrad" x1="0%" y1="0%" x2="100%" y2="0%">
                      <stop offset="0%" stopColor="#00d4ff" stopOpacity="0.9"/>
                      <stop offset="25%" stopColor="#8a2be2" stopOpacity="1"/>
                      <stop offset="50%" stopColor="#ff1493" stopOpacity="1"/>
                      <stop offset="75%" stopColor="#ff8c00" stopOpacity="1"/>
                      <stop offset="100%" stopColor="#ffff00" stopOpacity="0.9"/>
                    </linearGradient>
                  </defs>
                  <polyline
                    points={Array.from(meters.waveformData).map((value, i) => 
                      `${(i / meters.waveformData.length) * 100},${50 + value * 35}`
                    ).join(' ')}
                    fill="none"
                    stroke="url(#outputWaveGrad)"
                    strokeWidth="3"
                    style={{ 
                      filter: 'drop-shadow(0 0 8px #ff1493) drop-shadow(0 0 16px #8a2be2)',
                      strokeLinecap: 'round',
                      strokeLinejoin: 'round'
                    }}
                  />
                  
                  {/* Grid lines for professional look */}
                  <g stroke="rgba(255,255,255,0.05)" strokeWidth="1">
                    <line x1="0" y1="25" x2="100" y2="25" />
                    <line x1="0" y1="50" x2="100" y2="50" />
                    <line x1="0" y1="75" x2="100" y2="75" />
                    <line x1="25" y1="0" x2="25" y2="100" />
                    <line x1="50" y1="0" x2="50" y2="100" />
                    <line x1="75" y1="0" x2="75" y2="100" />
                  </g>
                </svg>
              </div>
            </div>
          </div>
        ) : viewMode === 'advanced' ? (
          // ADVANCED VIEW - Sound Design Mode  
          <div className="space-y-8">
            <div className="grid grid-cols-1 xl:grid-cols-2 gap-8">
              {/* AI Automation Panel */}
              <AIAutomationPanel 
                state={state}
                updateParameter={updateParameter}
                analysisData={meters.analysisData}
              />
              
              {/* Enhanced Clipping Controls */}
              <EnhancedClippingControls 
                state={state}
                updateParameter={updateParameter}
              />
            </div>
          </div>
        ) : (
          // ENGINEERING VIEW - Technical Analysis Mode
          <div className="space-y-8">
            {/* Processing Chain Visualizer */}
            <ProcessingChainVisualizer 
              state={state}
              analysisData={meters.analysisData}
            />

            {/* Advanced Metering Panel */}
            <AdvancedMeterPanel 
              state={state}
              meters={meters}
            />
          </div>
        )}
      </div>
    </div>
  );
};