import React, { useState, useEffect, useCallback, useReducer, useMemo } from 'react';
import { cn } from '@/lib/utils';

import { ThermalKnob } from '@/components/ThermalKnob';
import { OutputScope } from '@/components/OutputScope';
import { CentralVisualizerCanvas } from '@/components/CentralVisualizerCanvas';
import { SectionCard } from '@/components/SectionCard';

import { useAnalyser } from '@/hooks/useAnalyser';
import { useAudioEngine } from '@/hooks/useAudioEngine';
import { useIRConvolver } from '@/hooks/useIRConvolver';

import { makeBTZReducer } from '@/store/btzReducer';
import { morphParams } from '@/utils/morph';
import { asClipType } from '@/utils/params';

import { PresetScroller } from './PresetScroller';
import { SparkPanel } from './SparkPanel';
import { ShinePanel } from './ShinePanel';
import { MasterGluePanel } from './MasterGluePanel';
import { DeepControlsPanel } from './DeepControlsPanel';
import { AIAutomationPanel } from './AIAutomationPanel';
import { ProcessingChainVisualizer } from './ProcessingChainVisualizer';
import { AdvancedMeterPanel } from './AdvancedMeterPanel';

import { ToggleButton } from './ToggleButton';
import { PRO_STYLE_PRESETS } from './proStyles';

import type { BTZPluginState, EnhancedPreset } from './types';

// ----------------- PRESETS -----------------
const DEFAULT_PRESET: EnhancedPreset = {
  id: 'default',
  label: 'Default',
  state: {
    // Core
    punch: 0, warmth: 0, boom: 0, mix: 1, drive: 0,
    texture: false, active: true, oversampling: true,
    clippingType: 'soft', clippingBlend: 0.5, clippingEnabled: false,
    lufsTarget: -5, aiEnhance: false, timbralTransfer: false, aiAutomation: true,
    gateThreshold: -40, transientAmount: 0, saturationAmount: 0,
    subHarmonics: false, consoleGlue: true, oversamplingRate: 4,

    // SPARK (ON by default)
    sparkEnabled: true, sparkLUFS: -5, sparkCeiling: -0.3, sparkMix: 1,
    sparkOS: 8, sparkAutoOS: true, sparkMode: 'soft', sparkGR: 0,

    // SHINE (OFF default)
    shineEnabled: false, shineFreqHz: 20000, shineGainDb: 3, shineQ: 0.5, shineMix: 0.5,
    shineAutoOS: true, shineAB: false,

    // MASTER GLUE/MAX (OFF default)
    masterEnabled: false, masterMacro: 0.5, masterBlend: 'transparent', masterMix: 1,
    masterCompAttack: 10, masterCompRelease: 100, masterCompRatio: 2,
    masterLimiterCeiling: -0.3, masterLimiterLookahead: 0.8,

    // Deep toggles OFF
    transEnabled: false, eqEnabled: false, dynEnabled: false, subEnabled: false, consoleEnabled: false,

    // Deep params
    transAttack: 0, transSustain: 0, transDetect: 'wide',
    eqLowGain: 0, eqLowFreq: 80, eqMidGain: 0, eqMidFreq: 1200, eqMidQ: 1,
    eqHighGain: 0, eqHighFreq: 8000,
    dynThreshold: 0, dynRatio: 2, dynAttack: 8, dynRelease: 120, dynKnee: 2,
    subAmount: 0, subFreq: 50,
    consoleDrive: 0.15, consoleCrosstalk: 0.1,
  }
};

// Your quick "performance" tile presets across the top scroller
const PERFORMANCE_PRESETS: EnhancedPreset[] = [
  DEFAULT_PRESET,
  {
    id: 'mvp-loud-clean',
    label: 'MVP (-8 LUFS)',
    state: { punch: 0.7, warmth: 0.5, boom: 0.4, mix: 1, drive: 0.8, texture: false, active: true,
      oversampling: true, clippingType: 'soft', clippingBlend: 0.6, clippingEnabled: true,
      aiAutomation: true, gateThreshold: -25, transientAmount: 0.6, saturationAmount: 0.4,
      subHarmonics: true, consoleGlue: true, lufsTarget: -8, sparkEnabled: true, sparkLUFS: -8, sparkMix: 1, sparkMode: 'hard'
    }
  },
  { id: 'streaming-safe', label: 'Streaming Safe (-14 LUFS)', state: { punch: .5, warmth: .4, boom: .3, mix: 1, drive: .4, texture: false, active: true, oversampling: false, lufsTarget: -14, sparkEnabled: true, sparkLUFS: -14 } },
  { id: 'punchy-kick',   label: 'Punchy Kick',    state: { punch: .9, warmth: .3, boom: .8, mix: .9, drive: .5, texture: false, active: true, oversampling: false, clippingType: 'hard', clippingBlend: .4, clippingEnabled: true, sparkMode: 'hard' } },
  { id: 'silky-snare',   label: 'Silky Snare',    state: { punch: .8, warmth: .6, boom: .2, mix: .95, drive: .4, texture: true, active: true, oversampling: true, clippingType: 'tube', sparkMode: 'tube' } },
  { id: 'room-glue',     label: 'Room Glue',      state: { punch: .4, warmth: .7, boom: .5, mix: .8, drive: .6, texture: true, active: true, oversampling: false, clippingEnabled: true, clippingBlend: .6, sparkMode: 'tape' } },
  { id: 'tape-warmth',   label: 'Tape Warmth',    state: { punch: .6, warmth: .9, boom: .6, mix: .9, drive: .7, texture: true, active: true, oversampling: false, clippingEnabled: true, clippingBlend: .8, sparkMode: 'tape' } },
  { id: 'boom-sculpt',   label: 'Boom Sculpt',    state: { punch: .7, warmth: .4, boom: .9, mix: .85, drive: .8, texture: false, active: true, oversampling: true, clippingEnabled: true, sparkMode: 'soft', sparkMix: .9 } },
  { id: 'billboard-loud',label: 'Billboard Loud (-6 LUFS)', state: { punch: .9, warmth: .6, boom: .5, mix: 1, drive: .95, texture: false, active: true, oversampling: true, clippingEnabled: true, lufsTarget: -6, sparkLUFS: -6, sparkMode: 'hard' } },
];

// ----------------- UTIL -----------------
function clamp01(v?: number) { return v == null ? v : Math.max(0, Math.min(1, v)); }
function clampState(s: Partial<BTZPluginState>): Partial<BTZPluginState> {
  return {
    ...s,
    punch: clamp01(s.punch), warmth: clamp01(s.warmth), boom: clamp01(s.boom),
    mix: clamp01(s.mix), drive: clamp01(s.drive), clippingBlend: clamp01(s.clippingBlend),
  };
}

// ===================================================================
//                             COMPONENT
// ===================================================================
export const EnhancedBTZPlugin: React.FC = () => {
  const [state, dispatch] = useReducer(makeBTZReducer(DEFAULT_PRESET.state), DEFAULT_PRESET.state);
  const [skin, setSkin] = useState<'modern' | 'hardware'>('modern');

  useEffect(() => {
    if (typeof window === 'undefined') return;
    const sk = localStorage.getItem('btz:skin');
    if (sk === 'modern' || sk === 'hardware') setSkin(sk as any);
  }, []);
  useEffect(() => {
    if (typeof window === 'undefined') return;
    localStorage.setItem('btz:skin', skin);
  }, [skin]);

  // Audio engine + analyser
  const audio = useAudioEngine();
  const { running: audioRunning, start: startAudio, stop: stopAudio, update: updateAudio, analyserOut, ctxRef, nodeRef } = audio;
  const analyserData = useAnalyser(analyserOut, 60);

  // IR convolver (for the IR panel; connected to engine chain)
  const ir = useIRConvolver(ctxRef as any, nodeRef as any);
  useEffect(() => {
    if (!audioRunning || !ir.ready) return;
    const ctx = (ctxRef.current as AudioContext)!;
    try { ir.dry.current?.disconnect(); ir.wet.current?.disconnect(); } catch {}
    const analyser = (analyserOut as AnalyserNode);
    ir.dry.current?.connect(analyser).connect(ctx.destination);
    ir.wet.current?.connect(analyser).connect(ctx.destination);
  }, [audioRunning, ir.ready, analyserOut, ctxRef]);

  // Meters
  const [meters, setMeters] = useState({
    inputLevel: 0, outputLevel: 0,
    spectrumData: new Float32Array(64), waveformData: new Float32Array(128),
    lufsIntegrated: -14.2, truePeak: -2.1, isProcessing: false,
    analysisData: { transientStrength: 0, lowEndEnergy: 0, loudnessScore: 0, richness: 0, spectralCentroid: 1000 }
  });

  const specPunch = useMemo(() => meters.spectrumData.subarray(0, 16), [meters.spectrumData]);
  const specWarmth = useMemo(() => meters.spectrumData.subarray(16, 32), [meters.spectrumData]);
  const specBoom   = useMemo(() => meters.spectrumData.subarray(0, 8), [meters.spectrumData]);
  const specMix    = useMemo(() => meters.spectrumData, [meters.spectrumData]);
  const specDrive  = useMemo(() => meters.spectrumData.subarray(32, 48), [meters.spectrumData]);

  // Map analyser to meters
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

  // Fallback meters (no engine running)
  useEffect(() => {
    if (typeof window === 'undefined') return;
    if (audioRunning) return;
    let raf = 0, last = 0;
    const tick = (t: number) => {
      raf = requestAnimationFrame(tick);
      if (t - last < 1000 / 60) return;
      setMeters(prev => {
        if (!state.active) return { ...prev, isProcessing: false };
        const baseLevel = 0.2 + Math.random() * 0.5;
        let processedLevel = baseLevel * (1 + (state.drive ?? 0) * 0.8) * (state.mix ?? 1);
        const spectrumData = prev.spectrumData;
        for (let i = 0; i < 64; i++) {
          const freq = (i / 64) * 20000;
          let mag = Math.random() * 0.1;
          if (freq < 200) mag += (state.boom ?? 0) * 0.6;
          if (freq > 80 && freq < 800) mag += (state.punch ?? 0) * 0.5;
          if (freq > 1000 && freq < 8000) mag += (state.warmth ?? 0) * 0.4;
          if (freq > 8000) mag += (state.texture ? 0.3 : 0.1);
          if (state.clippingEnabled) mag += 0.1;
          spectrumData[i] = Math.max(0, Math.min(1, mag * (1 + (state.drive ?? 0) * 0.3)));
        }
        const waveformData = prev.waveformData;
        for (let i = 0; i < 128; i++) {
          const tt = (i / 128) * Math.PI * 4;
          let wave = Math.sin(tt) * processedLevel * (0.8 + Math.random() * 0.4);
          waveformData[i] = wave * 0.5 + 0.5;
        }
        const lufsIntegrated = prev.lufsIntegrated * 0.92 + (-14 + processedLevel * 6);
        const truePeak = prev.truePeak * 0.8 + Math.min(0, -1 + processedLevel * 2);
        return { inputLevel: baseLevel, outputLevel: processedLevel, spectrumData, waveformData, lufsIntegrated, truePeak, isProcessing: processedLevel > 0.1, analysisData: prev.analysisData };
      });
      last = t;
    };
    raf = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(raf);
  }, [state, audioRunning]);

  // ---- Engine param push (includes DSP notes) ----
  const engineParams = useMemo(() => ({
    mix: state.mix, drive: state.drive, active: state.active,
    clippingType: asClipType(state.clippingType ?? 'soft'), clippingBlend: state.clippingBlend ?? .5,

    spark: {
      enabled: !!state.sparkEnabled,
      targetLUFS: state.sparkLUFS ?? -5,
      ceilingTP: state.sparkCeiling ?? -0.3,
      mix: state.sparkMix ?? 1,
      mode: state.sparkMode ?? 'soft',
      os: state.sparkOS ?? 8,
      autoOS: !!state.sparkAutoOS,
      // DSP: lookahead 0.8ms, pre-sat → primary, TP ceiling, up to 16x OS with polyphase
    },

    shine: {
      enabled: !!state.shineEnabled,
      freqHz: state.shineFreqHz ?? 20000,
      gainDb: state.shineGainDb ?? 3,
      q: state.shineQ ?? 0.5,
      mix: state.shineMix ?? 0.5,
      autoOS: !!state.shineAutoOS,
      // DSP: auto OS when f>18k or gain>2dB; ultrasonic bell
    },

    master: {
      enabled: !!state.masterEnabled,
      macro: state.masterMacro ?? 0.5,
      blend: state.masterBlend ?? 'transparent',
      mix: state.masterMix ?? 1,
      comp: { attackMs: state.masterCompAttack ?? 10, releaseMs: state.masterCompRelease ?? 100, ratio: state.masterCompRatio ?? 2 },
      limiter: { ceilingDb: state.masterLimiterCeiling ?? -0.3, lookaheadMs: state.masterLimiterLookahead ?? 0.8 },
      // DSP: vintage/opto+xfmr, digital/vca+lookahead, transparent/rms+L-type. Macro ties comp+limit drive.
    },

    deep: {
      trans: { enabled: !!state.transEnabled, attack: state.transAttack ?? 0, sustain: state.transSustain ?? 0, detect: state.transDetect ?? 'wide' },
      eq:   { enabled: !!state.eqEnabled, low: { gain: state.eqLowGain ?? 0, freq: state.eqLowFreq ?? 80 }, mid: { gain: state.eqMidGain ?? 0, freq: state.eqMidFreq ?? 1200, q: state.eqMidQ ?? 1 }, high: { gain: state.eqHighGain ?? 0, freq: state.eqHighFreq ?? 8000 } },
      dyn:  { enabled: !!state.dynEnabled, thr: state.dynThreshold ?? 0, ratio: state.dynRatio ?? 2, atk: state.dynAttack ?? 8, rel: state.dynRelease ?? 120, knee: state.dynKnee ?? 2 },
      sub:  { enabled: !!state.subEnabled, amt: state.subAmount ?? 0, freq: state.subFreq ?? 50 },
      console: { enabled: !!state.consoleEnabled, drive: state.consoleDrive ?? 0.15, xtalk: state.consoleCrosstalk ?? 0.1 }
    }
  }), [state]);

  useEffect(() => { updateAudio?.(engineParams); }, [engineParams, updateAudio]);

  // --------- handlers ----------
  const updateParameter = useCallback(<K extends keyof BTZPluginState>(key: K, value: BTZPluginState[K]) => {
    dispatch({ type: 'set', key, value });
  }, []);

  const applyPreset = useCallback((preset: EnhancedPreset) => {
    morphParams(state, preset.state, 200, (patch) => dispatch({ type: 'batch', patch }), () => {});
  }, [state]);

  // =========================================
  //                RENDER
  // =========================================
  return (
    <div className={cn("w-full max-w-7xl mx-auto rounded-3xl border border-audio-primary/20 overflow-hidden", skin === 'hardware' && 'skin-hardware')}
         style={{ background: 'var(--gradient-main)', boxShadow: 'var(--shadow-panel), 0 0 60px hsl(var(--audio-primary) / 0.1)' }}>

      {/* Header */}
      <div className="relative border-b border-audio-primary/20 p-6 bg-gradient-to-r from-plugin-surface/50 to-plugin-panel/50">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-8">
            <div className="relative">
              <h1 className="text-4xl font-black text-transparent bg-gradient-to-r from-audio-primary to-audio-secondary bg-clip-text tracking-wider" style={{ filter: 'drop-shadow(0 0 20px hsl(var(--audio-primary) / 0.5))', fontFamily: 'var(--font-display)' }}>BTZ</h1>
              <div className="absolute -top-2 -right-2 w-3 h-3 rounded-full bg-audio-primary animate-pulse" style={{ boxShadow: '0 0 10px hsl(var(--audio-primary))' }} />
            </div>
            <div>
              <h2 className="text-lg font-bold text-foreground tracking-wide">BOX TONE ZONE</h2>
              <h3 className="text-xs text-foreground/70 tracking-[0.2em] uppercase">Enhanced Audio Processor</h3>
            </div>
          </div>

          <div className="flex items-center gap-3">
            <div className="hidden sm:flex bg-plugin-surface rounded-full p-1 border border-foreground/10">
              <button onClick={() => setSkin('modern')}   className={cn("px-3 py-2 rounded-full text-xs font-medium", skin === 'modern'   ? "bg-foreground text-background" : "text-foreground/70 hover:text-foreground")}>MODERN</button>
              <button onClick={() => setSkin('hardware')} className={cn("px-3 py-2 rounded-full text-xs font-medium", skin === 'hardware' ? "bg-foreground text-background" : "text-foreground/70 hover:text-foreground")}>HARDWARE</button>
            </div>
            <button onClick={() => (audioRunning ? stopAudio?.() : startAudio?.())}
              className={cn("px-4 py-2 rounded-full text-xs font-bold border", audioRunning ? "bg-audio-success text-background border-audio-success" : "bg-plugin-raised/50 border-plugin-raised hover:bg-plugin-raised text-foreground/80")}
              aria-pressed={!!audioRunning}>
              {audioRunning ? 'AUDIO ON' : 'ENABLE AUDIO'}
            </button>
            <ToggleButton value={state.active} onChange={(v) => updateParameter('active', v)} label="POWER"
              className={cn("px-8 py-3 rounded-full border-2 font-bold",
                state.active ? "bg-audio-primary border-audio-primary text-background shadow-[0_0_20px_hsl(var(--audio-primary))]" : "bg-plugin-raised/50 border-plugin-raised hover:bg-plugin-raised text-foreground/70")} />
          </div>
        </div>
      </div>

      {/* Unified Console */}
      <div className="p-6 md:p-8 space-y-8">

        {/* Visual + 5 knobs */}
        <div className="grid xl:grid-cols-[1.2fr_1fr] gap-8">
          <SectionCard title="" subtitle="">
            <div className="flex justify-center mb-6">
              <CentralVisualizerCanvas spectrumData={meters.spectrumData} waveformData={meters.waveformData} isProcessing={meters.isProcessing} level={meters.outputLevel} />
            </div>

            <div className="grid grid-cols-3 sm:grid-cols-5 gap-4 justify-items-center max-w-4xl mx-auto">
              <ThermalKnob label="PUNCH"  value={state.punch}  onChange={(v)=>updateParameter('punch', v)}  spectrumData={specPunch} waveformData={meters.waveformData} colorA="#ff2fb9" colorB="#39ff88" />
              <ThermalKnob label="WARMTH" value={state.warmth} onChange={(v)=>updateParameter('warmth', v)} spectrumData={specWarmth} waveformData={meters.waveformData} colorA="#39ff88" colorB="#274bff" />
              <ThermalKnob label="BOOM"   value={state.boom}   onChange={(v)=>updateParameter('boom', v)}   spectrumData={specBoom}  waveformData={meters.waveformData} colorA="#ff8c00" colorB="#ff2fb9" />
              <ThermalKnob label="MIX"    value={state.mix}    onChange={(v)=>updateParameter('mix', v)}    spectrumData={specMix}    waveformData={meters.waveformData} colorA="#00d4ff" colorB="#8a2be2" />
              <ThermalKnob label="DRIVE"  value={state.drive}  onChange={(v)=>updateParameter('drive', v)}  spectrumData={specDrive}  waveformData={meters.waveformData} colorA="#ff2fb9" colorB="#ff8c00" />
            </div>
          </SectionCard>

          <SectionCard title="OUTPUT" subtitle="">
            <OutputScope data={meters.waveformData} lufs={meters.lufsIntegrated} peak={meters.truePeak} />
          </SectionCard>
        </div>

        {/* Scrolling Presets (Performance tiles + renamed styles in dropdown style) */}
        <PresetScroller
          presets={PERFORMANCE_PRESETS}
          proStyles={PRO_STYLE_PRESETS}
          onApply={(p) => applyPreset(p)}
          onApplyStyle={(preset) => dispatch({ type: 'batch', patch: clampState(preset.state) })}
        />

        {/* SPARK (clipper) */}
        <SparkPanel state={state} update={updateParameter} />

        {/* SHINE + MASTER + DEEP in a 2-col row */}
        <div className="grid lg:grid-cols-2 gap-8">
          <ShinePanel state={state} updateParameter={updateParameter} />
          <MasterGluePanel state={state} update={updateParameter} />
        </div>
        <DeepControlsPanel state={state} update={updateParameter} />

        {/* IR Convolver + AI card side-by-side */}
        <div className="grid lg:grid-cols-2 gap-8">
          <SectionCard title="Convolution Reverb" subtitle="Drag & drop IR WAVs or use presets">
            <div className="text-center text-foreground/60 py-8">
              IR Convolver Panel placeholder - integrate your existing IR panel here
            </div>
          </SectionCard>

          <AIAutomationPanel state={state} updateParameter={updateParameter} analysisData={meters.analysisData} />
        </div>

        {/* Engineering: foldable cards (chain + meters) */}
        <div className="grid lg:grid-cols-2 gap-8">
          <FoldCard title="Signal Processing Chain">
            <ProcessingChainVisualizer state={state} analysisData={meters.analysisData} />
          </FoldCard>
          <FoldCard title="Professional Metering">
            <AdvancedMeterPanel state={state} meters={meters} />
          </FoldCard>
        </div>

      </div>
    </div>
  );
};

// Simple collapsible wrapper so the "engineering" widgets don't dominate the page
const FoldCard: React.FC<{ title: string; children: React.ReactNode }> = ({ title, children }) => {
  const [open, setOpen] = useState(true);
  return (
    <div className="rounded-2xl border border-foreground/10">
      <div className="flex items-center justify-between px-4 py-3">
        <div className="text-sm font-semibold tracking-[.2em]">{title.toUpperCase()}</div>
        <button onClick={() => setOpen(o=>!o)} className="text-xs px-2 py-1 rounded border border-foreground/10">{open ? 'Hide' : 'Show'}</button>
      </div>
      {open && <div className="p-4 pt-0"><div className="mt-2">{children}</div></div>}
    </div>
  );
};
