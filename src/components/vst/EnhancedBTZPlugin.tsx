import React, { useState, useEffect, useCallback, useReducer, useMemo } from 'react';
import { ToggleButton } from './ToggleButton';
import { PresetBrowser } from './PresetBrowser';
import { AIAutomationPanel } from './AIAutomationPanel';
import { EnhancedClippingControls } from './EnhancedClippingControls';
import { ProcessingChainVisualizer } from './ProcessingChainVisualizer';
import { AdvancedMeterPanel } from './AdvancedMeterPanel';
import { BTZPluginState, EnhancedPreset } from './types';
import { cn } from '@/lib/utils';
import { PresetsSelect } from './PresetsSelect';
import { PRO_STYLE_PRESETS } from './proStyles';
import { Slider } from '@/components/ui/slider';
import { makeBTZReducer } from '@/store/btzReducer';
import { morphParams } from '@/utils/morph';
import { useAnalyser } from '@/hooks/useAnalyser';
import { useAudioEngine } from '@/hooks/useAudioEngine';
import { ThermalKnob } from '@/components/ThermalKnob';
import { SectionCard } from '@/components/SectionCard';
import { ClipTypeRadio } from '@/components/ClipTypeRadio';
import { IRConvolverPanel } from '@/components/IRConvolverPanel';
import { CentralVisualizerCanvas } from '@/components/CentralVisualizerCanvas';
import { OutputScope } from '@/components/OutputScope';
import { StickyControls } from '@/components/StickyControls';
import { useHotkeys } from '@/hooks/useHotkeys';
import { usePresetLibrary } from '@/hooks/usePresetLibrary';
import { asClipType } from '@/utils/params';
import { useIRConvolver } from '@/hooks/useIRConvolver';

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
  const [state, dispatch] = useReducer(makeBTZReducer(DEFAULT_PRESET.state), DEFAULT_PRESET.state);
  const [viewMode, setViewMode] = useState<'primary' | 'advanced' | 'engineering'>('primary');
  const [skin, setSkin] = useState<'modern' | 'hardware'>('modern');
  useEffect(() => {
    if (typeof window === 'undefined') return;
    const v = localStorage.getItem('btz:view');
    if (v === 'primary' || v === 'advanced' || v === 'engineering') {
      setViewMode(v as any);
    }
    const sk = localStorage.getItem('btz:skin');
    if (sk === 'modern' || sk === 'hardware') {
      setSkin(sk as any);
    }
  }, []);
  useEffect(() => {
    if (typeof window === 'undefined') return;
    localStorage.setItem('btz:view', viewMode);
  }, [viewMode]);
  useEffect(() => {
    if (typeof window === 'undefined') return;
    localStorage.setItem('btz:skin', skin);
  }, [skin]);

  // Audio engine + analyser wiring
  const audio = useAudioEngine();
  const { running: audioRunning, start: startAudio, stop: stopAudio, update: updateAudio, analyserOut, ctxRef, nodeRef } = audio;
  const analyserData = useAnalyser(analyserOut, 60);

  const ir = useIRConvolver(ctxRef as any, nodeRef as any);
  useEffect(() => {
    if (!audioRunning || !ir.ready) return;
    const ctx = (ctxRef.current as AudioContext)!;
    try { ir.dry.current?.disconnect(); ir.wet.current?.disconnect(); } catch {}
    const analyser = (analyserOut as AnalyserNode);
    ir.dry.current?.connect(analyser).connect(ctx.destination);
    ir.wet.current?.connect(analyser).connect(ctx.destination);
  }, [audioRunning, ir.ready]);
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

  // Push UI param changes into audio engine
  useEffect(() => {
    updateAudio?.({
      mix: state.mix,
      drive: state.drive,
      active: state.active,
      clippingType: asClipType(state.clippingType),
      clippingBlend: state.clippingBlend,
    });
  }, [state.mix, state.drive, state.active, state.clippingType, state.clippingBlend, updateAudio]);

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
        const spectrumData = prev.spectrumData; // reuse buffer
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

        const waveformData = prev.waveformData; // reuse buffer
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
    <div className={cn("w-full max-w-7xl mx-auto rounded-3xl border border-audio-primary/20 overflow-hidden", skin === 'hardware' && 'skin-hardware')}
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
            <div className="hidden sm:flex bg-plugin-surface rounded-full p-1 border border-foreground/10">
              <button
                onClick={() => setSkin('modern')}
                className={cn(
                  "px-3 py-2 rounded-full text-xs font-medium transition-all duration-300",
                  skin === 'modern'
                    ? "bg-foreground text-background shadow-lg"
                    : "text-foreground/70 hover:text-foreground"
                )}
              >
                MODERN
              </button>
              <button
                onClick={() => setSkin('hardware')}
                className={cn(
                  "px-3 py-2 rounded-full text-xs font-medium transition-all duration-300",
                  skin === 'hardware'
                    ? "bg-foreground text-background shadow-lg"
                    : "text-foreground/70 hover:text-foreground"
                )}
              >
                HARDWARE
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
              <CentralVisualizerCanvas 
                spectrumData={meters.spectrumData}
                waveformData={meters.waveformData}
                isProcessing={meters.isProcessing}
                level={meters.outputLevel}
              />
            </div>

            {/* Main Controls - 5 Knobs with live visualization */}
            <div className="grid grid-cols-3 sm:grid-cols-5 gap-3 sm:gap-4 md:gap-6 gap-y-6 justify-items-center max-w-md sm:max-w-4xl mx-auto">
              <ThermalKnob label="PUNCH"  value={state.punch}  onChange={(v)=>updateParameter('punch', v)}
                           spectrumData={specPunch} waveformData={meters.waveformData}
                           colorA="#ff2fb9" colorB="#39ff88" />
              <ThermalKnob label="WARMTH" value={state.warmth} onChange={(v)=>updateParameter('warmth', v)}
                           spectrumData={specWarmth} waveformData={meters.waveformData}
                           colorA="#39ff88" colorB="#274bff" />
              <ThermalKnob label="BOOM"   value={state.boom}   onChange={(v)=>updateParameter('boom', v)}
                           spectrumData={specBoom} waveformData={meters.waveformData}
                           colorA="#ff8c00" colorB="#ff2fb9" />
              <ThermalKnob label="MIX"    value={state.mix}    onChange={(v)=>updateParameter('mix', v)}
                           spectrumData={specMix} waveformData={meters.waveformData}
                           colorA="#00d4ff" colorB="#8a2be2" />
              <ThermalKnob label="DRIVE"  value={state.drive}  onChange={(v)=>updateParameter('drive', v)}
                           spectrumData={specDrive} waveformData={meters.waveformData}
                           colorA="#ff2fb9" colorB="#ff8c00" />
            </div>

            <SectionCard title="FL STUDIO CLIPPER" subtitle="Billboard‑level loudness without distortion"
              right={<ToggleButton value={!!state.clippingEnabled} onChange={(v)=>updateParameter('clippingEnabled', v)} label="Clipping Enabled" />}>
              <h4 className="text-xs tracking-[.22em] text-foreground/60 mb-3">CLIPPING TYPE</h4>
              <ClipTypeRadio value={state.clippingType as string} onChange={(v)=>updateParameter('clippingType', v as any)} />

              <div className="mt-6 grid place-items-center">
                <ThermalKnob label="BLEND" value={state.clippingBlend ?? .5} onChange={(v)=>updateParameter('clippingBlend', v)}
                             spectrumData={meters.spectrumData} waveformData={meters.waveformData} colorA="#ff8c00" colorB="#39ff88" />
                <p className="mt-2 text-[12px] text-foreground/60">Wet/Dry mix of clipped signal</p>
              </div>
            </SectionCard>

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

            <OutputScope data={meters.waveformData} lufs={meters.lufsIntegrated} peak={meters.truePeak} />
          </div>
        ) : viewMode === 'advanced' ? (
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

            {/* IR Convolver Panel */}
            <IRConvolverPanel
              loadIRFromUrl={ir.loadIRFromUrl}
              loadIRFromArrayBuffer={ir.loadIRFromArrayBuffer}
              setWet={ir.setWet} setDry={ir.setDry}
              setPreDelay={ir.setPreDelay} setHP={ir.setHP} setLP={ir.setLP} setDamp={ir.setDamp}
            />
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