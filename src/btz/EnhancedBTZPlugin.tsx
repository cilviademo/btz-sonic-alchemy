import React, { useEffect, useMemo, useReducer, useState, useCallback } from 'react';
import './theme.css';
import { cn } from '@/lib/utils';
import { useGlobalHotkeys } from '@/hooks/useGlobalHotkeys';
import type { BTZPluginState, PanelId } from '@/btz/types';
import { DEFAULT_STATE } from '@/btz/types';
import { ThermalKnob } from './components/ThermalKnob';
import { CentralVisualizerCanvas } from './components/CentralVisualizerCanvas';
import { PresetStrip, type PresetItem } from './components/PresetStrip';
import { EnhancedMeterStrip } from './components/EnhancedMeterStrip';
import { OverlayGrid } from './components/OverlayGrid';
import { PanelDrawer } from './components/PanelDrawer';
import { renderPanel } from './components/PanelRegistry';
import { ModKnob } from './components/ModKnob';
import { ABToggle } from './components/ABToggle';
import { ActivePathStrip } from './components/ActivePathStrip';
import { QuickModeButtons } from './components/QuickModeButtons';

// ---- presets
const PRESETS: PresetItem[] = [
  { id:'default', label:'Default', state:{ punch:0,warmth:0,boom:0,drive:0,mix:1, sparkEnabled:true,sparkLUFS:-5,sparkCeiling:-0.3,sparkMix:1 } },
  { id:'dynamic-design', label:'Dynamic-Design', state:{ punch:.65, warmth:.2, boom:.25, drive:.35 } },
  { id:'punch-smack', label:'Punch-Smack', state:{ punch:.9, boom:.35, mix:.95 } },
  { id:'precision-q', label:'Precision-Q', state:{ warmth:.35, boom:.2 } },
  { id:'analog-reel', label:'Analog-Reel', state:{ warmth:.72, drive:.55 } },
  { id:'vintage-tape', label:'Vintage-Tape', state:{ warmth:.8, drive:.65, mix:.92 } },
  { id:'sonic-vital', label:'Sonic-Vital', state:{ warmth:.45, boom:.4 } },
  { id:'classic-76', label:'Classic-76', state:{ punch:.55, drive:.5 } },
  { id:'saturation-decap', label:'Saturation-Decap', state:{ drive:.85, mix:.85 } },
  { id:'drum-sculpt', label:'Drum-Sculpt', state:{ punch:.6, boom:.7 } },
  { id:'transient-neutron', label:'Transient-Neutron', state:{ punch:.75 } },
  { id:'bass-synth', label:'Bass-Synth', state:{ boom:.9, mix:.9 } },
  { id:'clarity-air', label:'Clarity-Air', state:{ shineEnabled:true, shineGainDb:4, shineFreqHz:28000 } },
  { id:'split-physic', label:'Split-Physic', state:{ warmth:.25, punch:.4, drive:.4 } },
  { id:'retro-goeq', label:'Retro-GoEQ', state:{ warmth:.5 } },
  { id:'intelligent-eq', label:'Intelligent-EQ', state:{ warmth:.3, boom:.2 } },
  { id:'tone-char', label:'Tone-Char', state:{ warmth:.4, drive:.35 } },
  { id:'tube-cascade', label:'Tube-Cascade', state:{ drive:.7, warmth:.55 } },
  { id:'low-mint', label:'Low-Mint', state:{ boom:.65 } },
  { id:'stereo-vital', label:'Stereo-Vital', state:{ mix:.9 } },
  { id:'vintage-76', label:'Vintage-76', state:{ punch:.65, drive:.45 } },
  { id:'dynamic-intense', label:'Dynamic-Intense', state:{ punch:.8, drive:.6 } },
  { id:'harmonic-spark', label:'Harmonic-Spark', state:{ drive:.75, warmth:.45 } },
  { id:'multiband-saturn', label:'Multiband-Saturn', state:{ drive:.8, mix:.85 } },
  { id:'mix-vault', label:'Mix-Vault', state:{ punch:.45, warmth:.45, boom:.45 } },
  { id:'drum-trigger', label:'Drum-Trigger', state:{ punch:.8, boom:.8, mix:.95 } },
] satisfies PresetItem[];

// ---- reducer
type Act =
  | { type:'set'; key:keyof BTZPluginState; value:BTZPluginState[keyof BTZPluginState] }
  | { type:'batch'; patch: Partial<BTZPluginState> }
  | { type:'reset' }
  | { type:'restore'; state: BTZPluginState };  // for undo/redo and A/B

function reducer(s:BTZPluginState, a:Act):BTZPluginState {
  switch(a.type){
    case 'set':   return { ...s, [a.key]: a.value };
    case 'batch': return { ...s, ...a.patch };
    case 'reset': return { ...DEFAULT_STATE };
    case 'restore': return a.state;
    default:      return s;
  }
}

export const EnhancedBTZPlugin:React.FC = () => {
  const [state, dispatch] = useReducer(reducer, DEFAULT_STATE);
  const [lufs, setLUFS] = useState(-14.2);
  const [peak, setPeak] = useState(-0.7);
  const [gainReduction, setGainReduction] = useState(1); // 1 = no reduction
  const [stereoCorrelation, setStereoCorrelation] = useState(1); // 1 = full stereo
  const [showGrid, setShowGrid] = useState(false);
  const [panel, setPanel] = useState<PanelId | null>(null);

  // A/B comparison (UX improvement #4)
  const [slotA, setSlotA] = useState<BTZPluginState>(DEFAULT_STATE);
  const [slotB, setSlotB] = useState<BTZPluginState>(DEFAULT_STATE);
  const [currentSlot, setCurrentSlot] = useState<'A' | 'B'>('A');

  // Undo/Redo (UX improvement #4)
  const [history, setHistory] = useState<BTZPluginState[]>([DEFAULT_STATE]);
  const [historyIndex, setHistoryIndex] = useState(0);

  // Meters / analyzer sim — throttled (~30fps) and paused if not active
  useEffect(() => {
    if (!state.active) return;
    let raf = 0, last = 0;
    const tick = (t:number) => {
      raf = requestAnimationFrame(tick);
      if (document.hidden) return;
      if (t - last < 33) return; // ~30fps
      last = t;
      const out = 0.25 + state.drive*.3 + state.punch*.2 + state.boom*.25;
      setLUFS(-14 + out*4 + Math.sin(t*.001)*.4);
      setPeak(-1 + out*.8 + Math.sin(t*.0015)*.2);

      // Simulate gain reduction when Spark is enabled
      const gr = state.sparkEnabled ? Math.max(0.85, 1 - state.drive * 0.15) : 1;
      setGainReduction(gr);

      // Simulate stereo correlation (wider with warmth)
      const corr = 0.7 + state.warmth * 0.3;
      setStereoCorrelation(corr);
    };
    raf = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(raf);
  }, [state.active, state.drive, state.punch, state.boom, state.sparkEnabled, state.warmth]);

  const toggleMeters = useCallback(() => setPanel(p => (p === 'meters' ? null : 'meters')), []);
  const resetPlugin = useCallback(() => dispatch({ type: 'reset' }), []);
  const toggleGrid = useCallback(() => setShowGrid(s => !s), []);

  // Undo/Redo handlers
  const undo = useCallback(() => {
    if (historyIndex > 0) {
      const newIndex = historyIndex - 1;
      setHistoryIndex(newIndex);
      dispatch({ type: 'restore', state: history[newIndex] });
    }
  }, [historyIndex, history]);

  const redo = useCallback(() => {
    if (historyIndex < history.length - 1) {
      const newIndex = historyIndex + 1;
      setHistoryIndex(newIndex);
      dispatch({ type: 'restore', state: history[newIndex] });
    }
  }, [historyIndex, history]);

  // A/B comparison handlers
  const switchAB = useCallback(() => {
    // Save current state to current slot
    if (currentSlot === 'A') {
      setSlotA(state);
      dispatch({ type: 'restore', state: slotB });
      setCurrentSlot('B');
    } else {
      setSlotB(state);
      dispatch({ type: 'restore', state: slotA });
      setCurrentSlot('A');
    }
  }, [currentSlot, state, slotA, slotB]);

  const copyAtoB = useCallback(() => {
    setSlotB(slotA);
    if (currentSlot === 'B') {
      dispatch({ type: 'restore', state: slotA });
    }
  }, [slotA, currentSlot]);

  const copyBtoA = useCallback(() => {
    setSlotA(slotB);
    if (currentSlot === 'A') {
      dispatch({ type: 'restore', state: slotB });
    }
  }, [slotB, currentSlot]);

  useGlobalHotkeys({
    toggleMeters,
    reset: resetPlugin,
    toggleGrid,
    undo
  });

  const applyPreset = useCallback((p:PresetItem) => {
    dispatch({type:'batch', patch:p.state});
  }, []);

  const applyQuickMode = useCallback((patch: Partial<BTZPluginState>) => {
    dispatch({type:'batch', patch});
  }, []);

  const update = useCallback(<K extends keyof BTZPluginState>(k:K, v:BTZPluginState[K]) => {
    dispatch({ type:'set', key:k, value:v });
    // Add to history for undo/redo
    setHistory(prev => [...prev.slice(0, historyIndex + 1), { ...state, [k]: v }]);
    setHistoryIndex(prev => prev + 1);
  }, [historyIndex, state]);

  const peakNorm = useMemo(() => Math.max(0, Math.min(1, (peak + 20) / 20)), [peak]);

  // Active signal path nodes (UX #12)
  const signalPath = useMemo(() => [
    { id: 'input', label: 'IN', active: true, color: '#39ff88' },
    { id: 'punch', label: 'Punch', active: state.punch > 0.01, color: '#ff2fb9' },
    { id: 'warmth', label: 'Warmth', active: state.warmth > 0.01, color: '#39ff88' },
    { id: 'boom', label: 'Boom', active: state.boom > 0.01, color: '#ff8a00' },
    { id: 'drive', label: 'Drive', active: state.drive > 0.01, color: '#ff8a00' },
    { id: 'spark', label: 'Spark', active: state.sparkEnabled, color: '#ff8a00' },
    { id: 'shine', label: 'Shine', active: state.shineEnabled, color: '#2fd3ff' },
    { id: 'master', label: 'Master', active: state.masterEnabled, color: '#39ff88' },
    { id: 'output', label: 'OUT', active: true, color: '#2fd3ff' },
  ], [state]);

  return (
    <div className="relative rounded-lg bg-neutral-950 p-4 text-white">
      <OverlayGrid show={showGrid} />

      {/* Top controls: A/B + Active Path */}
      <div className="mb-4 flex items-center justify-between gap-4">
        <ABToggle
          currentSlot={currentSlot}
          onSwitch={switchAB}
          onCopyAtoB={copyAtoB}
          onCopyBtoA={copyBtoA}
        />
        <div className="flex-1">
          <ActivePathStrip nodes={signalPath} />
        </div>
        <button
          type="button"
          onClick={() => update('precisionMode', !state.precisionMode)}
          className={cn(
            "rounded px-3 py-1.5 text-xs font-medium uppercase tracking-wide transition-all",
            state.precisionMode ? "btz-precision-active" : "bg-white/5 hover:bg-white/10"
          )}
          title="Precision Mode: quantized steps, ultra-fine control"
        >
          {state.precisionMode ? '⚡ Precision' : 'Precision'}
        </button>
      </div>

      {/* Enhanced meters */}
      <div className="mb-4">
        <EnhancedMeterStrip
          lufs={lufs}
          truePeakDb={peak}
          peakNorm={peakNorm}
          gainReduction={gainReduction}
          stereoCorrelation={stereoCorrelation}
          targetLUFS={state.sparkLUFS}
        />
      </div>

      {/* Visualizer + five core knobs (Hero Section - UX #8) */}
      <div className="grid grid-cols-1 gap-4 md:grid-cols-[1fr,450px]">
        <div className="btz-hero-section flex flex-col gap-6">
          <CentralVisualizerCanvas peakNorm={peakNorm} />
          <div className="grid grid-cols-5 gap-4">
            {[
              {k:'punch', cA:'#ff2fb9', cB:'#39ff88', label:'PUNCH', hasDetent:false},
              {k:'warmth',cA:'#39ff88', cB:'#2fd3ff', label:'WARMTH', hasDetent:false},
              {k:'boom',  cA:'#ff8a00', cB:'#ff2fb9', label:'BOOM', hasDetent:false},
              {k:'mix',   cA:'#2fd3ff', cB:'#8a2be2', label:'MIX', toDisplay:(v:number)=>`${Math.round(v*100)}%`, hasDetent:true},
              {k:'drive', cA:'#ff8a00', cB:'#ff2fb9', label:'DRIVE', toDisplay:(v:number)=>`${(v*24-12).toFixed(1)} dB`, hasDetent:true},
            ].map(n=>(
              <div key={n.k} className={cn("btz-knob-hero", n.hasDetent && "btz-knob-detent")}>
                <ThermalKnob
                  label={n.label}
                  value={state[n.k as keyof BTZPluginState] as number}
                  onChange={(v) => update(n.k as keyof BTZPluginState, v)}
                  colorA={n.cA} colorB={n.cB}
                  toDisplay={n.toDisplay}
                  fine={state.precisionMode ? 0.1 : 0.25}
                />
              </div>
            ))}
          </div>
        </div>

        {/* Right controls column */}
        <div className="flex flex-col gap-4">
          {/* Input/Output Trim (UX #11) */}
          <div className="btz-module-section space-y-3">
            <div className="btz-label-xs opacity-70">I/O Trim</div>
            <div className="grid grid-cols-2 gap-3">
              <div>
                <label className="mb-1 block text-[9px] uppercase opacity-60">Input</label>
                <input
                  type="range"
                  min="-12"
                  max="12"
                  step="0.1"
                  value={state.inputGain}
                  onChange={(e) => update('inputGain', Number(e.target.value))}
                  className="w-full"
                />
                <div className="mt-0.5 text-center text-[10px] btz-value-display">
                  {state.inputGain > 0 ? '+' : ''}{state.inputGain.toFixed(1)} dB
                </div>
              </div>
              <div>
                <label className="mb-1 block text-[9px] uppercase opacity-60">Output</label>
                <input
                  type="range"
                  min="-12"
                  max="12"
                  step="0.1"
                  value={state.outputGain}
                  onChange={(e) => update('outputGain', Number(e.target.value))}
                  className="w-full"
                />
                <div className="mt-0.5 text-center text-[10px] btz-value-display">
                  {state.outputGain > 0 ? '+' : ''}{state.outputGain.toFixed(1)} dB
                </div>
              </div>
            </div>
            <button
              type="button"
              onClick={() => update('autoGain', !state.autoGain)}
              className={cn(
                "w-full rounded px-2 py-1 text-xs uppercase transition-all",
                state.autoGain ? "bg-green-500/20 text-green-400" : "bg-white/5 hover:bg-white/10"
              )}
            >
              {state.autoGain ? '✓ Auto-Gain' : 'Auto-Gain Off'}
            </button>
          </div>

          {/* Quick Mode Buttons (UX #7) */}
          <QuickModeButtons onApply={applyQuickMode} />

          {/* Module Knobs with Micro-Params (UX #2) */}
          <div className="space-y-2">
            <ModKnob
              label="Spark"
              on={state.sparkEnabled}
              value={state.sparkMix}
              setOn={(v)=>update('sparkEnabled', v)}
              setVal={(v)=>update('sparkMix', v)}
              colorA="#ff8a00"
              colorB="#ff2fb9"
              open={()=>setPanel('spark')}
              microParams={[
                { label: 'LUFS', value: state.sparkLUFS.toFixed(0) },
                { label: 'Ceiling', value: state.sparkCeiling.toFixed(1), unit: ' dB' },
                { label: 'Mode', value: state.sparkMode },
              ]}
            />
            <ModKnob
              label="Shine"
              on={state.shineEnabled}
              value={state.shineMix}
              setOn={(v)=>update('shineEnabled', v)}
              setVal={(v)=>update('shineMix', v)}
              colorA="#2fd3ff"
              colorB="#8a2be2"
              open={()=>setPanel('shine')}
              microParams={[
                { label: 'Freq', value: `${(state.shineFreqHz / 1000).toFixed(1)}k` },
                { label: 'Gain', value: state.shineGainDb.toFixed(1), unit: ' dB' },
                { label: 'Q', value: state.shineQ.toFixed(2) },
              ]}
            />
            <ModKnob
              label="Master"
              on={state.masterEnabled}
              value={state.masterMix}
              setOn={(v)=>update('masterEnabled', v)}
              setVal={(v)=>update('masterMix', v)}
              colorA="#39ff88"
              colorB="#2fd3ff"
              open={()=>setPanel('master')}
              microParams={[
                { label: 'Blend', value: state.masterBlend },
                { label: 'Macro', value: `${Math.round(state.masterMacro * 100)}%` },
              ]}
            />
            <ModKnob
              label="Meters"
              on={true}
              value={1}
              setOn={()=>{}}
              setVal={()=>{}}
              colorA="#ff6a3d"
              colorB="#ff2fb9"
              disabled
              open={()=>setPanel('meters')}
            />
            <ModKnob
              label="Sub"
              on={state.subEnabled}
              value={state.subAmount ?? 0.5}
              setOn={(v)=>update('subEnabled', v)}
              setVal={(v)=>update('subAmount', v)}
              colorA="#ff2fb9"
              colorB="#2fd3ff"
              open={()=>setPanel('convolver')}
              microParams={[
                { label: 'Amount', value: `${Math.round((state.subAmount ?? 0.5) * 100)}%` },
              ]}
            />
            <ModKnob
              label="Console"
              on={state.consoleEnabled}
              value={1}
              setOn={(v)=>update('consoleEnabled', v)}
              setVal={()=>{}}
              colorA="#8a2be2"
              colorB="#2fd3ff"
              disabled
              open={()=>setPanel('deep')}
            />
          </div>

          <div className="mt-3 btz-label-micro opacity-40">
            Click module for details • Shift+A for A/B • Cmd+Z to undo
          </div>

          <PresetStrip presets={PRESETS} onApply={applyPreset} />
        </div>
      </div>

      {/* Panel Drawer */}
      <PanelDrawer open={!!panel} onClose={()=>setPanel(null)} title={panel?.toUpperCase()}>
        {panel && renderPanel(panel, state, update)}
      </PanelDrawer>
    </div>
  );
};