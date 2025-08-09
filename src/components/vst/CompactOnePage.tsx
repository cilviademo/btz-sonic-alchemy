import React, { useEffect, useMemo, useReducer, useRef, useState } from 'react';
import { cn } from '@/lib/utils';
import { makeBTZReducer } from '@/store/btzReducer';
import { DEFAULT_PRESET } from './defaults';
import { useAudioEngine } from '@/hooks/useAudioEngine';
import { useAnalyser } from '@/hooks/useAnalyser';
import { useIRConvolver } from '@/hooks/useIRConvolver';

import { ModuleKnob } from './ModuleKnob';
import { MiniMeterStrip } from './MiniMeterStrip';
import { CentralVisualizerCanvas } from '@/components/CentralVisualizerCanvas';
import { ThermalKnob } from './ThermalKnob';
import { LEDMeter } from './LEDMeter';
import { PresetScroller, PresetItem } from '@/components/PresetScroller';
import { useHotkeys } from '@/hooks/useHotkeys';

// Pop-out panels
import { SparkPanel } from './SparkPanel';
import { ShinePanel } from './ShinePanel';
import { MasterGluePanel } from './MasterGluePanel';
import { DeepControlsPanel } from './DeepControlsPanel';
import { IRConvolverPanel } from '@/components/IRConvolverPanel';
import { AIAutomationPanel } from './AIAutomationPanel';
import { AdvancedMeterPanel } from './AdvancedMeterPanel';
import { ProcessingChainVisualizer } from './ProcessingChainVisualizer';
import { GridOverlay } from '@/components/dev/GridOverlay';
import { useModalRoute } from './useModalRoute';
import type { BTZPluginState } from './types';

export const CompactOnePage: React.FC = () => {
  const [state, dispatch] = useReducer(makeBTZReducer(DEFAULT_PRESET.state), DEFAULT_PRESET.state);
  const historyRef = useRef<BTZPluginState[]>([]);
  const update = <K extends keyof BTZPluginState>(k: K, v: BTZPluginState[K]) => {
    historyRef.current.push({ ...state });
    if (historyRef.current.length > 50) historyRef.current.shift();
    dispatch({ type: 'set', key: k, value: v });
  };

  const audio = useAudioEngine();
  const analyser = useAnalyser(audio.analyserOut, 60);
  const ir = useIRConvolver(audio.ctxRef as any, audio.nodeRef as any);

  const { route, open, close } = useModalRoute();

  // Hotkeys are initialized after presets declaration

  // Engine param push
  const engineParams = useMemo(() => ({
    mix: state.mix, drive: state.drive, active: state.active,
    clippingType: state.clippingType, clippingBlend: state.clippingBlend,
    spark: {
      enabled: !!state.sparkEnabled, targetLUFS: state.sparkLUFS ?? -5,
      ceilingTP: state.sparkCeiling ?? -0.3, mix: state.sparkMix ?? 1,
      mode: state.sparkMode ?? 'soft', os: state.sparkOS ?? 8, autoOS: !!state.sparkAutoOS,
    },
    shine: {
      enabled: !!state.shineEnabled, freqHz: state.shineFreqHz ?? 20000,
      gainDb: state.shineGainDb ?? 3, q: state.shineQ ?? .5, mix: state.shineMix ?? .5,
      autoOS: !!state.shineAutoOS
    },
    master: {
      enabled: !!state.masterEnabled, macro: state.masterMacro ?? .5,
      blend: state.masterBlend ?? 'transparent', mix: state.masterMix ?? 1,
      comp: { attackMs: state.masterCompAttack ?? 10, releaseMs: state.masterCompRelease ?? 100, ratio: state.masterCompRatio ?? 2 },
      limiter: { ceilingDb: state.masterLimiterCeiling ?? -0.3, lookaheadMs: state.masterLimiterLookahead ?? 0.8 }
    },
    deep: {
      trans: { enabled: !!state.transEnabled, attack: state.transAttack ?? 0, sustain: state.transSustain ?? 0, detect: state.transDetect ?? 'wide' },
      eq:   { enabled: !!state.eqEnabled, low:{gain:state.eqLowGain??0,freq:state.eqLowFreq??80},
              mid: {gain:state.eqMidGain??0,freq:state.eqMidFreq??1200,q:state.eqMidQ??1},
              high:{gain:state.eqHighGain??0,freq:state.eqHighFreq??8000} },
      dyn:  { enabled: !!state.dynEnabled, thr: state.dynThreshold ?? 0, ratio: state.dynRatio ?? 2, atk: state.dynAttack ?? 8, rel: state.dynRelease ?? 120, knee: state.dynKnee ?? 2 },
      sub:  { enabled: !!state.subEnabled, amt: state.subAmount ?? 0, freq: state.subFreq ?? 50 },
      console: { enabled: !!state.consoleEnabled, drive: state.consoleDrive ?? .15, xtalk: state.consoleCrosstalk ?? .1 }
    }
  }), [state]);

  useEffect(() => { audio.update?.(engineParams); }, [engineParams]);

  // Presets for the existing scroller component
  const presets: PresetItem<Partial<BTZPluginState>>[] = useMemo(() => [
    { id: 'default', label: 'Default', payload: DEFAULT_PRESET.state },
    { id: 'mvp', label: 'MVP (-8 LUFS)', payload: { lufsTarget: -8, sparkEnabled: true, sparkLUFS: -8, clippingEnabled: true, drive: 0.7, mix: 1 } },
    { id: 'stream', label: 'Streaming Safe', payload: { lufsTarget: -14, sparkEnabled: true, sparkLUFS: -14 } },
    { id: 'punch-kick', label: 'Punchy Kick', payload: { punch: 0.85, boom: 0.7, clippingEnabled: true } },
  ], []);
  const [selectedPresetId, setSelectedPresetId] = useState<string>('default');

  const selectPresetByDelta = (delta: number) => {
    const idx = Math.max(0, presets.findIndex(p => p.id === selectedPresetId));
    const ni = (idx + delta + presets.length) % presets.length;
    const next = presets[ni];
    setSelectedPresetId(next.id);
    historyRef.current.push({ ...state });
    dispatch({ type: 'batch', patch: next.payload as any });
  };

  useHotkeys({
    ' ': () => update('active', !state.active),
    'Space': () => update('active', !state.active),
    'M': () => (route === 'meters' ? close() : open('meters')),
    'A': () => update('aiAutomation', !state.aiAutomation),
    '/': () => update('sparkEnabled', !state.sparkEnabled),
    'ArrowUp': () => selectPresetByDelta(1),
    'ArrowDown': () => selectPresetByDelta(-1),
    'ArrowRight': () => selectPresetByDelta(1),
    'ArrowLeft': () => selectPresetByDelta(-1),
    'R': () => { historyRef.current.push({ ...state }); setSelectedPresetId('default'); dispatch({ type:'batch', patch: DEFAULT_PRESET.state }); },
    'Cmd+Z': () => { const prev = historyRef.current.pop(); if (prev) { dispatch({ type:'batch', patch: prev as any }); } },
    'Ctrl+Z': () => { const prev = historyRef.current.pop(); if (prev) { dispatch({ type:'batch', patch: prev as any }); } },
  });
  return (
    <div
      className="mx-auto relative rounded-2xl border border-foreground/10 overflow-hidden"
      style={{ width: 1200, height: 700, background: 'hsl(var(--plugin-panel))' }}
      aria-label="BTZ Compact Console"
    >
      {/* HEADER */}
      <div className="flex items-center justify-between px-8 py-6 border-b border-foreground/10 bg-plugin-surface/60">
        <div className="text-xl font-black tracking-wide">
          BTZ <span className="text-foreground/60 font-medium ml-3">BOX TONE ZONE</span>
        </div>

        <div className="flex-1 px-6">
          <MiniMeterStrip lufs={-14.2} truePeakDb={-0.7} peakNorm={analyser?.levelOut ?? 0} />
        </div>

        <div className="flex gap-3">
          <button
            onClick={() => (audio.running ? audio.stop?.() : audio.start?.())}
            className={cn('px-4 py-2 rounded-full text-xs border',
              audio.running ? 'bg-audio-success text-background border-audio-success' : 'bg-plugin-surface text-foreground/80 border-foreground/10')}
          >
            {audio.running ? 'AUDIO • ON' : 'ENABLE AUDIO'}
          </button>
          <button
            onClick={() => update('active', !state.active)}
            className={cn('px-4 py-2 rounded-full text-xs border',
              state.active ? 'bg-audio-primary text-background border-audio-primary' : 'bg-plugin-surface text-foreground/80 border-foreground/10')}
          >
            ● POWER
          </button>
        </div>
      </div>

      {/* MAIN GRID */}
      <div className="px-8 pb-8 grid grid-cols-12 gap-6">
        {/* LEFT: Visualizer + macro knobs */}
        <div className="col-span-7 space-y-6">
          <div className="rounded-xl border border-foreground/10 p-4 bg-plugin-surface/40">
            <CentralVisualizerCanvas
              spectrumData={analyser?.spectrum ?? new Float32Array(64)}
              waveformData={analyser?.waveform ?? new Float32Array(128)}
              isProcessing={!!state.active && !!audio.running}
              level={analyser?.levelOut ?? 0}
            />
          </div>

          <div className="grid grid-cols-5 gap-6">
            <ThermalKnob
              label="PUNCH"
              value={state.punch ?? 0}
              onChange={(v)=>update('punch', v)}
              spectrumData={analyser?.spectrum ?? new Float32Array(64)}
              waveformData={analyser?.waveform ?? new Float32Array(128)}
              colorA="#ff2fb9" colorB="#39ff88"
            />
            <ThermalKnob
              label="WARMTH"
              value={state.warmth ?? 0}
              onChange={(v)=>update('warmth', v)}
              spectrumData={analyser?.spectrum ?? new Float32Array(64)}
              waveformData={analyser?.waveform ?? new Float32Array(128)}
              colorA="#39ff88" colorB="#274bff"
            />
            <ThermalKnob
              label="BOOM"
              value={state.boom ?? 0}
              onChange={(v)=>update('boom', v)}
              spectrumData={analyser?.spectrum ?? new Float32Array(64)}
              waveformData={analyser?.waveform ?? new Float32Array(128)}
              colorA="#ff8c00" colorB="#ff2fb9"
            />
            <ThermalKnob
              label="MIX"
              value={state.mix ?? 1}
              onChange={(v)=>update('mix', v)}
              spectrumData={analyser?.spectrum ?? new Float32Array(64)}
              waveformData={analyser?.waveform ?? new Float32Array(128)}
              colorA="#00d4ff" colorB="#8a2be2"
            />
            <ThermalKnob
              label="DRIVE"
              value={state.drive ?? 0}
              onChange={(v)=>update('drive', v)}
              spectrumData={analyser?.spectrum ?? new Float32Array(64)}
              waveformData={analyser?.waveform ?? new Float32Array(128)}
              colorA="#ffee66" colorB="#ff00ff"
            />
          </div>

          <PresetScroller
            title="PRESETS"
            presets={presets}
            selectedId={selectedPresetId}
            onSelect={(p) => {
              setSelectedPresetId(p.id);
              historyRef.current.push({ ...state });
              dispatch({ type: 'batch', patch: p.payload });
            }}
          />
        </div>

        {/* RIGHT: Output + module knobs */}
        <div className="col-span-5 space-y-6">
          <div className="rounded-xl border border-foreground/10 p-4 bg-plugin-surface/40">
            <div className="flex items-center justify-between mb-3">
              <div className="text-sm font-semibold text-foreground/80">OUTPUT</div>
              <div className="flex gap-4 text-xs text-foreground/70">
                <div className="font-mono">LUFS <span className="text-foreground">-14.0</span></div>
                <div className="font-mono">TP <span className="text-foreground">-0.7 dB</span></div>
              </div>
            </div>
            <div className="flex items-end gap-4" style={{height: 160}}>
              <div className="flex-1 grid grid-cols-2 gap-3 h-full">
                <LEDMeter value={analyser?.levelOut ?? 0} />
                <LEDMeter value={analyser?.levelOut ?? 0} />
              </div>
            </div>
          </div>

          <div className="grid grid-cols-3 gap-4">
            <ModuleKnob id="spark" label="SPARK" valuePct={(state.sparkMix ?? 1)*100}
                        colorA="#ff6a00" colorB="#ffd873"
                        onOpen={()=>open('spark')} onToggle={()=>update('sparkEnabled', !state.sparkEnabled)} enabled={!!state.sparkEnabled}/>
            <ModuleKnob id="shine" label="SHINE" valuePct={(state.shineMix ?? .5)*100}
                        colorA="#8ec5ff" colorB="#ffffff"
                        onOpen={()=>open('shine')} onToggle={()=>update('shineEnabled', !state.shineEnabled)} enabled={!!state.shineEnabled}/>
            <ModuleKnob id="master" label="MASTER" valuePct={(state.masterMacro ?? .5)*100}
                        colorA="#66ffad" colorB="#2de2ff"
                        onOpen={()=>open('master')} onToggle={()=>update('masterEnabled', !state.masterEnabled)} enabled={!!state.masterEnabled}/>
            <ModuleKnob id="ir" label="CONVOLVER" valuePct={50}
                        colorA="#c9a7ff" colorB="#ff8fab"
                        onOpen={()=>open('ir')} />
            <ModuleKnob id="ai" label="AI" valuePct={100}
                        colorA="#ffa64d" colorB="#ee2255"
                        onOpen={()=>open('ai')} onToggle={()=>update('aiAutomation', !state.aiAutomation)} enabled={!!state.aiAutomation}/>
            <ModuleKnob id="meters" label="METERS" valuePct={100}
                        colorA="#00d4ff" colorB="#8a2be2"
                        onOpen={()=>open('meters')} />
          </div>
        </div>
      </div>

      {/* Shortcuts strip */}
      <div className="px-8 py-2 border-t border-foreground/10 bg-plugin-surface/60 text-xs text-foreground/80 flex flex-wrap gap-x-4 gap-y-1">
        <span>Space: Bypass</span>
        <span>M: Toggle Meters</span>
        <span>↑/↓: Next/Prev Preset</span>
        <span>R: Reset</span>
        <span>Cmd/Ctrl+Z: Undo</span>
      </div>

      {/* SHEET MODAL */}
      <ModalSheet open={!!route} onClose={close} title={routeTitle(route)}>
        {route === 'spark'   && <SparkPanel  state={state} update={update} />}
        {route === 'shine'   && <ShinePanel  state={state} updateParameter={update as any} />}
        {route === 'master'  && <MasterGluePanel state={state} update={update} />}
        {route === 'deep'    && <DeepControlsPanel state={state} update={update} />}
        {route === 'ir'      && (
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
        )}
        {route === 'ai'      && (
          <AIAutomationPanel
            state={state}
            updateParameter={update as any}
            analysisData={{ transientStrength:0, lowEndEnergy:0, loudnessScore:0, richness:0, spectralCentroid:0 }}
          />
        )}
        {route === 'meters'  && (
          <AdvancedMeterPanel
            state={state}
            meters={{
              inputLevel: analyser?.levelIn ?? 0,
              outputLevel: analyser?.levelOut ?? 0,
              lufsIntegrated: -14.2,
              truePeak: -0.7,
              isProcessing: !!audio.running,
              analysisData: { transientStrength:0, lowEndEnergy:0, loudnessScore:0, richness:0, spectralCentroid:0 }
            }}
          />
        )}
        {route === 'chain'   && <ProcessingChainVisualizer state={state} analysisData={{ transientStrength:0,lowEndEnergy:0,loudnessScore:0, richness:0, spectralCentroid:0 }} />}
      </ModalSheet>

      <GridOverlay show={false} />
    </div>
  );
};

const routeTitle = (r: ReturnType<typeof useModalRoute>['route']) => {
  switch (r) {
    case 'spark': return 'SPARK — Master Clipper';
    case 'shine': return 'SHINE — Air Band';
    case 'master': return 'MASTER GLUE/MAX';
    case 'deep': return 'DEEP CONTROLS';
    case 'ir': return 'Convolution Reverb';
    case 'ai': return 'AI Automation';
    case 'meters': return 'Professional Metering';
    case 'chain': return 'Signal Processing Chain';
    default: return '';
  }
};

const ModalSheet: React.FC<{open:boolean; onClose:()=>void; title:string; children:React.ReactNode}> = ({ open, onClose, title, children }) => {
  if (!open) return null;
  return (
    <div className="fixed inset-0 z-[60]">
      <div className="absolute inset-0 bg-black/60" onClick={onClose} />
      <div className="absolute right-0 top-0 h-full w-[760px] max-w-[85vw] bg-plugin-surface border-l border-foreground/12 shadow-2xl overflow-y-auto">
        <div className="sticky top-0 bg-plugin-surface/90 backdrop-blur border-b border-foreground/10 flex items-center justify-between px-6 py-4">
          <div className="text-sm font-semibold tracking-wide">{title}</div>
          <button className="text-foreground/70 hover:text-foreground" onClick={onClose}>Close</button>
        </div>
        <div className="p-6">{children}</div>
      </div>
    </div>
  );
};
