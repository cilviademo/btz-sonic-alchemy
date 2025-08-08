import React, { useState, useEffect, useCallback, useReducer, useMemo } from 'react';
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
import { PresetsSelect, PresetOption } from './PresetsSelect';
import { PRO_STYLE_PRESETS } from './proStyles';
import { Slider } from '@/components/ui/slider';
import { makeBTZReducer } from '@/store/btzReducer';
import { morphParams } from '@/utils/morph';
import { useAnalyser } from '@/hooks/useAnalyser';
import { useAudioEngine } from '@/hooks/useAudioEngine';
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

function clampState(s: Partial<BTZPluginState>): Partial<BTZPluginState> {
  const clamp01 = (v?: number) => (v == null ? v : Math.max(0, Math.min(1, v)));
  return {
    ...s,
    punch: clamp01(s.punch),
    warmth: clamp01(s.warmth),
    boom: clamp01(s.boom),
    mix: clamp01(s.mix),
    drive: clamp01(s.drive),
    clippingBlend: clamp01(s.clippingBlend),
  };
}

export const EnhancedBTZPlugin: React.FC = () => {
  const [state, dispatch] = React.useReducer(makeBTZReducer(DEFAULT_PRESET.state), DEFAULT_PRESET.state);
  const [viewMode, setViewMode] = useState<'primary' | 'advanced' | 'engineering'>('primary');
  useEffect(() => {
    if (typeof window === 'undefined') return;
    const v = localStorage.getItem('btz:view');
    if (v === 'primary' || v === 'advanced' || v === 'engineering') {
      setViewMode(v as any);
    }
  }, []);
  useEffect(() => {
    if (typeof window === 'undefined') return;
    localStorage.setItem('btz:view', viewMode);
  }, [viewMode]);

  // Audio engine + analyser wiring
  const { running: audioRunning, start: startAudio, stop: stopAudio, update: updateAudio, analyserOut } = require('@/hooks/useAudioEngine').useAudioEngine?.() ?? {} as any;
  const analyserData = require('@/hooks/useAnalyser').useAnalyser?.(analyserOut, 60) ?? { spectrum: new Float32Array(64), waveform: new Float32Array(128), levelIn: 0, levelOut: 0 };

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

  const specPunch = useMemo(() => meters.spectrumData.subarray(0, 16), [meters.spectrumData]);
  const specWarmth = useMemo(() => meters.spectrumData.subarray(16, 32), [meters.spectrumData]);
  const specBoom = useMemo(() => meters.spectrumData.subarray(0, 8), [meters.spectrumData]);
  const specMix = useMemo(() => meters.spectrumData, [meters.spectrumData]);
  const specDrive = useMemo(() => meters.spectrumData.subarray(32, 48), [meters.spectrumData]);

  // Map analyser to meters if audio is running
  useEffect(() => {
    if (!analyserOut || !analyserData) return;
    setMeters(prev => ({
      ...prev,
      spectrumData: analyserData.spectrum,
      waveformData: analyserData.waveform,
      inputLevel: analyserData.levelIn,
      outputLevel: analyserData.levelOut,
      isProcessing: (state.active ?? true) && !!audioRunning,
    }));
  }, [analyserOut, analyserData?.spectrum, analyserData?.waveform, analyserData?.levelIn, analyserData?.levelOut, audioRunning, state.active]);

  // Generate live audio visualization when no audio engine is active (fallback sim)
  useEffect(() => {
    if (typeof window === 'undefined') return;
    if (audioRunning) return; // real analyser drives meters
    let raf = 0;
    let last = 0;

    const tick = (t: number) => {
      raf = requestAnimationFrame(tick);
      if (t - last < 1000 / 60) return;

      setMeters(prev => {
        if (!state.active) {
          return { ...prev, isProcessing: false };
        }

        const baseLevel = 0.2 + Math.random() * 0.5;
        let processedLevel = baseLevel * (1 + state.drive * 0.8) * state.mix;

        // Generate spectrum
        const spectrumData = new Float32Array(64);
        for (let i = 0; i < 64; i++) {
          const freq = (i / 64) * 20000;
          let magnitude = Math.random() * 0.1;
          if (freq < 200) magnitude += state.boom * 0.6;
          if (freq > 80 && freq < 800) magnitude += state.punch * 0.5;
          if (freq > 1000 && freq < 8000) magnitude += state.warmth * 0.4;
          if (freq > 8000) magnitude += state.texture ? 0.3 : 0.1;
          if (state.clippingEnabled) magnitude += 0.1;
          spectrumData[i] = Math.max(0, Math.min(1, magnitude * (1 + state.drive * 0.3)));
        }

        const waveformData = new Float32Array(128);
        for (let i = 0; i < 128; i++) {
          const tt = (i / 128) * Math.PI * 4;
          let wave = Math.sin(tt) * processedLevel * (0.8 + Math.random() * 0.4);
          waveformData[i] = wave * 0.5 + 0.5; // normalize to 0..1
        }

        const lufsIntegrated = prev.lufsIntegrated * 0.92 + (-14 + processedLevel * 6);
        const truePeak = prev.truePeak * 0.8 + Math.min(0, -1 + processedLevel * 2);

        return {
          inputLevel: baseLevel,
          outputLevel: processedLevel,
          spectrumData,
          waveformData,
          lufsIntegrated,
          truePeak,
          isProcessing: processedLevel > 0.1,
          analysisData: prev.analysisData,
        };
      });

      last = t;
    };

    raf = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(raf);
  }, [state, audioRunning]);

  const updateParameter = useCallback((key: keyof BTZPluginState, value: any) => {
    dispatch({ type: 'set', key, value });
  }, [dispatch]);

  const applyPreset = useCallback((preset: EnhancedPreset) => {
    morphParams(state, preset.state, 200, (patch) => dispatch({ type: 'batch', patch }), () => {});
  }, [state, dispatch]);

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
            {/* Audio Engine Toggle */}
            <button
              onClick={() => (audioRunning ? stopAudio?.() : startAudio?.())}
              className={cn(
                "px-4 py-2 rounded-full text-xs font-bold border transition-all duration-300",
                audioRunning
                  ? "bg-audio-success text-background border-audio-success"
                  : "bg-plugin-raised/50 border-plugin-raised hover:bg-plugin-raised text-foreground/80"
              )}
              aria-pressed={!!audioRunning}
            >
              {audioRunning ? 'AUDIO ON' : 'ENABLE AUDIO'}
            </button>
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
            <div className="grid grid-cols-3 sm:grid-cols-5 gap-3 sm:gap-4 md:gap-6 gap-y-6 justify-items-center max-w-md sm:max-w-4xl mx-auto">
              <ModernKnobWithSpectrum 
                value={state.punch} 
                onChange={(v) => updateParameter('punch', v)} 
                label="PUNCH" 
                min={0} 
                max={1}
                size="sm"
                spectrumData={specPunch}
                color="hsl(var(--audio-primary))"
              />
              <ModernKnobWithSpectrum 
                value={state.warmth} 
                onChange={(v) => updateParameter('warmth', v)} 
                label="WARMTH" 
                min={0} 
                max={1}
                size="sm"
                spectrumData={specWarmth}
                color="hsl(var(--audio-secondary))"
              />
              <ModernKnobWithSpectrum 
                value={state.boom} 
                onChange={(v) => updateParameter('boom', v)} 
                label="BOOM" 
                min={0} 
                max={1}
                size="sm"
                spectrumData={specBoom}
                color="hsl(var(--audio-tertiary))"
              />
              <ModernKnobWithSpectrum 
                value={state.mix} 
                onChange={(v) => updateParameter('mix', v)} 
                label="MIX" 
                min={0} 
                max={1}
                size="sm"
                spectrumData={specMix}
                color="hsl(var(--audio-success))"
              />
              <ModernKnobWithSpectrum 
                value={state.drive} 
                onChange={(v) => updateParameter('drive', v)} 
                label="DRIVE" 
                min={0} 
                max={1}
                size="sm"
                spectrumData={specDrive}
                color="hsl(var(--audio-warning))"
              />
            </div>

            {/* Texture & Clipping Controls */}
            <div className="flex flex-wrap justify-center items-center gap-6 sm:gap-8">
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

            {/* Compact Blend control for primary view */}
            <div className="w-full flex justify-center">
              <div className="flex items-center gap-3 bg-plugin-surface/60 border border-foreground/10 rounded-xl px-4 py-3 mt-2">
                <span className="text-[10px] sm:text-xs font-semibold tracking-wide text-foreground/80">CLIP BLEND</span>
                <div className="w-40 sm:w-56">
                  <Slider
                    value={[Math.round(((state.clippingBlend ?? 0.5) * 100))]}
                    onValueChange={(val) => updateParameter('clippingBlend', ((val?.[0] ?? 50) / 100))}
                    max={100}
                    step={1}
                    disabled={!state.clippingEnabled}
                  />
                </div>
                <span className="text-[10px] sm:text-xs font-mono w-10 text-right">{Math.round((state.clippingBlend ?? 0.5) * 100)}%</span>
              </div>
            </div>

            {/* Presets */}
            <div className="flex flex-col items-center gap-4">
              <PresetBrowser 
                presets={PERFORMANCE_PRESETS}
                onApplyPreset={applyPreset}
              />

              {/* Pro Styles - Top 25 Influences */}
              <div className="w-full max-w-xs">
                <PresetsSelect 
                  presets={PRO_STYLE_PRESETS as unknown as { id: string; label: string; state: Partial<BTZPluginState>; }[]}
                  onApply={(preset) => {
                    dispatch({ type: 'batch', patch: clampState(preset.state) });
                  }}
                />
              </div>
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