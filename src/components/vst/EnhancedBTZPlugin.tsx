import React, {
  useState, useEffect, useCallback, useReducer, useMemo, Suspense
} from 'react';
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
import { asClipType } from '@/utils/params';
import { useIRConvolver } from '@/hooks/useIRConvolver';
import { ArturiaFrame } from '@/components/arturia/ArturiaFrame';
import { ArturiaToggle } from '@/components/arturia/ArturiaToggle';
import { VUMeterNeedle } from '@/components/arturia/VUMeterNeedle';
import { ArturiaKnob } from '@/components/arturia/ArturiaKnob';
import { SparkPanel } from '@/components/vst/SparkPanel';
import { ShinePanel } from '@/components/vst/ShinePanel';
import { MasterGluePanel } from '@/components/vst/MasterGluePanel';
import { DeepControlsPanel } from '@/components/vst/DeepControlsPanel';
import { PresetScroller, PresetItem } from '@/components/PresetScroller';

// helpers
import { useRafThrottle } from '@/utils/useRafThrottle';
import { ErrorBoundary } from '@/utils/ErrorBoundary';
import { useLocalStorage } from '@/utils/useLocalStorage';

const DEFAULT_PRESET: EnhancedPreset = {
  id: 'default',
  label: 'Default',
  state: {
    punch: 0, warmth: 0, boom: 0, mix: 1, drive: 0,
    texture: false, active: true, oversampling: true,
    clippingType: 'soft', clippingBlend: 0.5, clippingEnabled: false,
    lufsTarget: -5, aiEnhance: false, timbralTransfer: false, aiAutomation: true,
    gateThreshold: -40, transientAmount: 0, saturationAmount: 0,
    subHarmonics: false, consoleGlue: true, oversamplingRate: 4,

    sparkEnabled: true, sparkLUFS: -5, sparkCeiling: -0.3, sparkMix: 1,
    sparkOS: 8, sparkAutoOS: true, sparkMode: 'soft', sparkGR: 0,

    shineEnabled: false, shineFreqHz: 20000, shineGainDb: 3, shineQ: 0.5, shineMix: 0.5, shineAutoOS: true, shineAB: false,

    masterEnabled: false, masterMacro: .5, masterBlend: 'transparent', masterMix: 1,
    masterCompAttack: 10, masterCompRelease: 100, masterCompRatio: 2,
    masterLimiterCeiling: -0.3, masterLimiterLookahead: 0.8,

    transEnabled: false, eqEnabled: false, dynEnabled: false, subEnabled: false, consoleEnabled: false,
    transAttack: 0, transSustain: 0, transDetect: 'wide',
    eqLowGain: 0, eqLowFreq: 80, eqMidGain: 0, eqMidFreq: 1200, eqMidQ: 1, eqHighGain: 0, eqHighFreq: 8000,
    dynThreshold: 0, dynRatio: 2, dynAttack: 8, dynRelease: 120, dynKnee: 2,
    subAmount: 0, subFreq: 50, consoleDrive: .15, consoleCrosstalk: .1,
  }
};

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

function clamp01(v?: number) {
  return v == null ? v : Math.max(0, Math.min(1, v));
}

function clampState(s: Partial<BTZPluginState>): Partial<BTZPluginState> {
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

type ViewMode = 'primary' | 'advanced' | 'engineering';
type Skin = 'modern' | 'hardware';

export const EnhancedBTZPlugin: React.FC = () => {
  const [state, dispatch] = useReducer(makeBTZReducer(DEFAULT_PRESET.state), DEFAULT_PRESET.state);

  // safer localStorage with defaults
  const [viewMode, setViewMode] = useLocalStorage<ViewMode>('btz:view', 'primary');
  const [skin, setSkin] = useLocalStorage<Skin>('btz:skin', 'modern');

  // Audio engine + analyser
  const audio = useAudioEngine();
  const { running: audioRunning, start: startAudio, stop: stopAudio, update: updateAudio, analyserOut, ctxRef, nodeRef } = audio;

  // analyser at 60fps; rAF throttle UI application for perf
  const analyserData = useAnalyser(analyserOut, 60);
  const applyMetersThrottled = useRafThrottle(60); // 60Hz target

  // --- IR Convolver routing ---
  const ir = useIRConvolver(ctxRef as any, nodeRef as any);

  useEffect(() => {
    if (!audioRunning || !ir.ready || !analyserOut || !ctxRef.current) return;
    try {
      ir.dry.current?.disconnect();
      ir.wet.current?.disconnect();
      ir.dry.current?.connect(analyserOut);
      ir.wet.current?.connect(analyserOut);
      (analyserOut as AnalyserNode).connect((ctxRef.current as AudioContext).destination);
    } catch {}
  }, [audioRunning, ir.ready, ir.dry, ir.wet, analyserOut, ctxRef]);

  // meters (managed with rAF throttling to avoid layout thrash)
  const [meters, setMeters] = useState(() => ({
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
  }));

  // fast typed slices
  const specPunch = useMemo(() => meters.spectrumData.subarray(0, 16), [meters.spectrumData]);
  const specWarmth = useMemo(() => meters.spectrumData.subarray(16, 32), [meters.spectrumData]);
  const specBoom = useMemo(() => meters.spectrumData.subarray(0, 8), [meters.spectrumData]);
  const specMix = meters.spectrumData;
  const specDrive = useMemo(() => meters.spectrumData.subarray(32, 48), [meters.spectrumData]);

  // Preset scroller data
  const ALL_PRESETS: PresetItem[] = useMemo(() => {
    const featured = PERFORMANCE_PRESETS.map(p => ({ id: p.id, label: p.label, payload: p }));
    const styles = (PRO_STYLE_PRESETS as any[]).map(p => ({
      id: `style-${p.id}`, label: p.label, payload: { id: p.id, label: p.label, state: p.state }
    }));
    const order = (a: PresetItem, b: PresetItem) => (a.label === 'Default' ? -1 : b.label === 'Default' ? 1 : 0);
    return [...featured, ...styles].sort(order);
  }, []);

  // analyser -> meters (real audio path)
  useEffect(() => {
    if (!analyserOut || !analyserData) return;
    applyMetersThrottled(() => {
      setMeters(prev => ({
        ...prev,
        spectrumData: analyserData.spectrum,
        waveformData: analyserData.waveform,
        inputLevel: analyserData.levelIn,
        outputLevel: analyserData.levelOut,
        isProcessing: (state.active ?? true) && !!audioRunning,
      }));
    });
  }, [analyserOut, analyserData, audioRunning, state.active, applyMetersThrottled]);

  // Push UI param changes into audio engine (stable object)
  const engineParams = useMemo(() => ({
    mix: state.mix,
    drive: state.drive,
    active: state.active,
    clippingType: asClipType(state.clippingType),
    clippingBlend: state.clippingBlend,
    // SPARK mapping for current audio engine
    sparkMix: state.sparkMix ?? 1,
    sparkOn: !!state.sparkEnabled,
    ceilingDb: state.sparkCeiling ?? -0.3,
    osFactor: state.sparkOS ?? 8,
  }), [state.mix, state.drive, state.active, state.clippingType, state.clippingBlend, state.sparkMix, state.sparkEnabled, state.sparkCeiling, state.sparkOS]);

  useEffect(() => { updateAudio?.(engineParams); }, [engineParams, updateAudio]);

  // Fallback sim (if audio engine is off)
  useEffect(() => {
    if (typeof window === 'undefined') return;
    if (audioRunning) return;
    let raf = 0;
    let last = 0;

    const tick = (t: number) => {
      raf = requestAnimationFrame(tick);
      if (t - last < 1000 / 60) return;

      setMeters(prev => {
        if (!state.active) return { ...prev, isProcessing: false };

        const baseLevel = 0.2 + Math.random() * 0.5;
        let processedLevel = baseLevel * (1 + state.drive * 0.8) * state.mix;

        const spectrumData = prev.spectrumData;
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

        const waveformData = prev.waveformData;
        for (let i = 0; i < 128; i++) {
          const tt = (i / 128) * Math.PI * 4;
          let wave = Math.sin(tt) * processedLevel * (0.8 + Math.random() * 0.4);
          waveformData[i] = wave * 0.5 + 0.5;
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

  // Hotkeys (single-arg map; your hook signature)
  useHotkeys({
    'space': () => dispatch({ type: 'set', key: 'active', value: !state.active }),
    'shift+right': () => {/* TODO: next preset */},
    'shift+left': () => {/* TODO: prev preset */},
    'm': () => dispatch({ type: 'set', key: 'mix', value: 1 }),
    'b': () => dispatch({ type: 'set', key: 'clippingEnabled', value: !state.clippingEnabled }),
  });

  const updateParameter = useCallback((key: keyof BTZPluginState, value: any) => {
    dispatch({ type: 'set', key, value });
  }, []);

  const applyPreset = useCallback((preset: EnhancedPreset) => {
    morphParams(state, preset.state, 180, (patch) => dispatch({ type: 'batch', patch }), () => {});
  }, [state]);

  return (
    <ErrorBoundary>
      <div
        className={cn(
          'w-full max-w-7xl mx-auto rounded-3xl border border-audio-primary/20 overflow-hidden',
          skin === 'hardware' && 'skin-hardware'
        )}
        style={{
          background: 'var(--gradient-main)',
          boxShadow: 'var(--shadow-panel), 0 0 60px hsl(var(--audio-primary) / 0.1)'
        }}
        role="region"
        aria-label="BTZ Enhancer"
      >
        {/* Header */}
        <header className="relative border-b border-audio-primary/20 p-6 bg-gradient-to-r from-plugin-surface/50 to-plugin-panel/50">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-8">
              <div className="relative" aria-label="Logo BTZ">
                <h1
                  className="text-4xl font-black text-transparent bg-gradient-to-r from-audio-primary to-audio-secondary bg-clip-text tracking-wider"
                  style={{
                    filter: 'drop-shadow(0 0 20px hsl(var(--audio-primary) / 0.5))',
                    fontFamily: 'var(--font-display)'
                  }}
                >
                  BTZ
                </h1>
                <div
                  className="absolute -top-2 -right-2 w-3 h-3 rounded-full bg-audio-primary animate-pulse"
                  style={{ boxShadow: '0 0 10px hsl(var(--audio-primary))' }}
                  aria-hidden
                />
              </div>
              <div>
                <h2 className="text-lg font-bold text-foreground tracking-wide">BOX TONE ZONE</h2>
                <h3 className="text-xs text-foreground/70 tracking-[0.2em] uppercase">Enhanced Audio Processor</h3>
              </div>
            </div>

            <div className="flex items-center gap-3">
              {/* View Toggle */}
              <nav className="flex bg-plugin-surface rounded-full p-1 border border-audio-primary/20" aria-label="View mode">
                {(['primary','advanced','engineering'] as ViewMode[]).map(v => (
                  <button
                    key={v}
                    onClick={() => setViewMode(v)}
                    className={cn(
                      'px-4 py-2 rounded-full text-xs font-medium transition-all duration-300 focus:outline-none focus:ring-2 focus:ring-audio-primary',
                      viewMode === v ? 'bg-audio-primary text-background shadow-lg' : 'text-foreground/70 hover:text-foreground'
                    )}
                    aria-pressed={viewMode === v}
                  >
                    {v === 'primary' ? 'PERFORMANCE' : v === 'advanced' ? 'SOUND DESIGN' : 'ENGINEERING'}
                  </button>
                ))}
              </nav>

              {/* Skin Toggle */}
              <div className="hidden md:flex bg-plugin-surface rounded-full p-1 border border-foreground/10" aria-label="Skin">
                {(['modern','hardware'] as Skin[]).map(s => (
                  <button
                    key={s}
                    onClick={() => setSkin(s)}
                    className={cn(
                      'px-3 py-2 rounded-full text-xs font-medium transition-all duration-300 focus:outline-none focus:ring-2 focus:ring-foreground',
                      skin === s ? 'bg-foreground text-background shadow-lg' : 'text-foreground/70 hover:text-foreground'
                    )}
                    aria-pressed={skin === s}
                  >
                    {s.toUpperCase()}
                  </button>
                ))}
              </div>

              {/* Audio Engine */}
              <button
                onClick={() => (audioRunning ? stopAudio?.() : startAudio?.())}
                className={cn(
                  'px-4 py-2 rounded-full text-xs font-bold border transition-all duration-300 focus:outline-none focus:ring-2 focus:ring-audio-success',
                  audioRunning
                    ? 'bg-audio-success text-background border-audio-success'
                    : 'bg-plugin-raised/50 border-plugin-raised hover:bg-plugin-raised text-foreground/80'
                )}
                aria-pressed={!!audioRunning}
                aria-label="Enable audio engine"
              >
                {audioRunning ? 'AUDIO ON' : 'ENABLE AUDIO'}
              </button>

              {/* Power */}
              <ToggleButton
                value={state.active}
                onChange={(v) => updateParameter('active', v)}
                label="POWER"
                className={cn(
                  'px-8 py-3 rounded-full border-2 font-bold transition-all duration-300 focus:outline-none focus:ring-2 focus:ring-audio-primary',
                  state.active
                    ? 'bg-audio-primary border-audio-primary text-background shadow-[0_0_20px_hsl(var(--audio-primary))]'
                    : 'bg-plugin-raised/50 border-plugin-raised hover:bg-plugin-raised text-foreground/70'
                )}
              />
            </div>
          </div>
        </header>

        {/* Content */}
        <main className="p-6 sm:p-8">
          {viewMode === 'primary' ? (
            <div className="space-y-10">
              {/* Central Visualizer */}
              <div className="flex justify-center">
                <CentralVisualizerCanvas
                  spectrumData={meters.spectrumData}
                  waveformData={meters.waveformData}
                  isProcessing={meters.isProcessing}
                  level={meters.outputLevel}
                />
              </div>

              {/* Knob row */}
              <section className="grid grid-cols-3 sm:grid-cols-5 gap-3 sm:gap-5 md:gap-6 justify-items-center max-w-5xl mx-auto">
                <ThermalKnob label="PUNCH"  value={state.punch}  onChange={(v)=>updateParameter('punch', v)}
                             spectrumData={specPunch}  waveformData={meters.waveformData}
                             colorA="#ff2fb9" colorB="#39ff88" />
                <ThermalKnob label="WARMTH" value={state.warmth} onChange={(v)=>updateParameter('warmth', v)}
                             spectrumData={specWarmth} waveformData={meters.waveformData}
                             colorA="#39ff88" colorB="#274bff" />
                <ThermalKnob label="BOOM"   value={state.boom}   onChange={(v)=>updateParameter('boom', v)}
                             spectrumData={specBoom}   waveformData={meters.waveformData}
                             colorA="#ff8c00" colorB="#ff2fb9" />
                <ThermalKnob label="MIX"    value={state.mix}    onChange={(v)=>updateParameter('mix', v)}
                             spectrumData={specMix}    waveformData={meters.waveformData}
                             colorA="#00d4ff" colorB="#8a2be2" />
                <ThermalKnob label="DRIVE"  value={state.drive}  onChange={(v)=>updateParameter('drive', v)}
                             spectrumData={specDrive}  waveformData={meters.waveformData}
                             colorA="#ff2fb9" colorB="#ff8c00" />
              </section>

              {/* SPARK master clipper */}
              <SparkPanel state={state} update={(k,v)=>dispatch({type:'set', key:k, value:v})} />

              {/* Scrolling presets */}
              <PresetScroller
                presets={ALL_PRESETS}
                onSelect={(item) => {
                  const preset = item.payload as EnhancedPreset;
                  morphParams(state, preset.state, 200, (patch)=>dispatch({type:'batch', patch}), ()=>{});
                }}
              />

              {/* Output Scope */}
              <OutputScope data={meters.waveformData} lufs={meters.lufsIntegrated} peak={meters.truePeak} />
            </div>
          ) : viewMode === 'advanced' ? (
            <Suspense fallback={<div className="text-foreground/70 p-6">Loading advanced modules…</div>}>
              <div className="space-y-8 skin-arturia">
                {/* Top bar — analog meter with live needle next to AI panel */}
                <div className="grid grid-cols-1 xl:grid-cols-[1.6fr_.9fr] gap-8">
                  <ArturiaFrame title="AI AUTOMATION" subtitle="Adaptive processing">
                    <AIAutomationPanel
                      state={state}
                      updateParameter={updateParameter}
                      analysisData={meters.analysisData}
                    />
                  </ArturiaFrame>

                  <ArturiaFrame title="OUTPUT MONITOR" subtitle="Analog VU">
                    <div className="grid gap-4">
                      <VUMeterNeedle value={Math.min(1, Math.max(0, (meters.outputLevel ?? 0)))} />
                      <div className="grid grid-cols-2 gap-3">
                        <ArturiaToggle
                          checked={!!state.consoleGlue}
                          onChange={(v)=>updateParameter('consoleGlue', v)}
                          label="BUS GLUE"
                        />
                        <ArturiaToggle
                          checked={!!state.texture}
                          onChange={(v)=>updateParameter('texture', v)}
                          label="AIR/TEXTURE"
                        />
                      </div>
                    </div>
                  </ArturiaFrame>
                </div>

                {/* Knob bank in Arturia style, but keeping animated Thermal cores */}
                <ArturiaFrame title="DEEP CONTROLS" subtitle="Punch • Warmth • Boom • Mix • Drive">
                  <div className="grid grid-cols-3 sm:grid-cols-5 gap-12 justify-items-center">
                    <ArturiaKnob label="PUNCH"  value={state.punch}  onChange={(v)=>updateParameter('punch', v)}
                                 spectrumData={meters.spectrumData.subarray(0,16)} waveformData={meters.waveformData}
                                 colorA="#ff2fb9" colorB="#39ff88" />
                    <ArturiaKnob label="WARMTH" value={state.warmth} onChange={(v)=>updateParameter('warmth', v)}
                                 spectrumData={meters.spectrumData.subarray(16,32)} waveformData={meters.waveformData}
                                 colorA="#39ff88" colorB="#274bff" />
                    <ArturiaKnob label="BOOM"   value={state.boom}   onChange={(v)=>updateParameter('boom', v)}
                                 spectrumData={meters.spectrumData.subarray(0,8)} waveformData={meters.waveformData}
                                 colorA="#ff8c00" colorB="#ff2fb9" />
                    <ArturiaKnob label="MIX"    value={state.mix}    onChange={(v)=>updateParameter('mix', v)}
                                 spectrumData={meters.spectrumData} waveformData={meters.waveformData}
                                 colorA="#00d4ff" colorB="#8a2be2" />
                    <ArturiaKnob label="DRIVE"  value={state.drive}  onChange={(v)=>updateParameter('drive', v)}
                                 spectrumData={meters.spectrumData.subarray(32,48)} waveformData={meters.waveformData}
                                 colorA="#ff2fb9" colorB="#ff8c00" />
                  </div>
                </ArturiaFrame>

                {/* SPARK: advanced clipper */}
                <ArturiaFrame title="SPARK" subtitle="Transparent, brutal loudness">
                  <SparkPanel state={state} update={(k,v)=>dispatch({type:'set', key:k, value:v})} />
                </ArturiaFrame>

                {/* SHINE: air enhancer */}
                <ArturiaFrame title="SHINE" subtitle="Ethereal air band">
                  <ShinePanel state={state} updateParameter={(k,v)=>dispatch({type:'set', key:k, value:v})} />
                </ArturiaFrame>

                {/* MASTER & DEEP CONTROLS */}
                <MasterGluePanel state={state} update={(k,v)=>dispatch({type:'set', key:k, value:v})} />
                <DeepControlsPanel state={state} update={(k,v)=>dispatch({type:'set', key:k, value:v})} />

                {/* IR Convolver in Arturia chassis */}
                <IRConvolverPanel
                  loadIRFromUrl={ir.loadIRFromUrl}
                  loadIRFromArrayBuffer={ir.loadIRFromArrayBuffer}
                  setWet={ir.setWet}
                  setDry={ir.setDry}
                  setPreDelay={ir.setPreDelay}
                  setHP={ir.setHP}
                  setLP={ir.setLP}
                  setDamp={ir.setDamp}
                />
              </div>
            </Suspense>
          ) : (
            <div className="space-y-8">
              <ProcessingChainVisualizer state={state} analysisData={meters.analysisData} />
              <AdvancedMeterPanel state={state} meters={meters} />
            </div>
          )}
        </main>

        {/* Sticky HUD (after content, before boundary end) */}
        <StickyControls
          active={state.active}
          audioRunning={!!audioRunning}
          onTogglePower={() => dispatch({ type: 'set', key: 'active', value: !state.active })}
          onEngineClick={() => (audioRunning ? stopAudio?.() : startAudio?.())}
          lufs={meters.lufsIntegrated}
          peak={meters.truePeak}
        />
      </div>
    </ErrorBoundary>
  );
};
