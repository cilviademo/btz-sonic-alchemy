import React, { useEffect, useMemo, useReducer, useState, useCallback } from 'react';
import './theme.css';
import { cn } from '@/lib/utils';
import { useGlobalHotkeys } from '@/hooks/useGlobalHotkeys';
import type { BTZPluginState, PanelId } from '@/btz/types';
import { DEFAULT_STATE } from '@/btz/types';
import { ThermalKnob } from './components/ThermalKnob';
import { CentralVisualizerCanvas } from './components/CentralVisualizerCanvas';
import { PresetStrip, type PresetItem } from './components/PresetStrip';
import { MiniMeterStrip } from './components/MiniMeterStrip';
import { OverlayGrid } from './components/OverlayGrid';
import { PanelDrawer } from './components/PanelDrawer';
import { renderPanel } from './components/PanelRegistry';
import { ModKnob } from './components/ModKnob';

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
  | { type:'reset' };

function reducer(s:BTZPluginState, a:Act):BTZPluginState {
  switch(a.type){
    case 'set':   return { ...s, [a.key]: a.value } as BTZPluginState;
    case 'batch': return { ...s, ...a.patch };
    case 'reset': return { ...DEFAULT_STATE };
    default:      return s;
  }
}

export const EnhancedBTZPlugin:React.FC = () => {
  const [state, dispatch] = useReducer(reducer, DEFAULT_STATE);
  const [lufs, setLUFS] = useState(-14.2);
  const [peak, setPeak] = useState(-0.7);
  const [showGrid, setShowGrid] = useState(false);
  const [panel, setPanel] = useState<PanelId | null>(null);

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
    };
    raf = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(raf);
  }, [state.active, state.drive, state.punch, state.boom]);

  useGlobalHotkeys({
    toggleMeters: () => setPanel(p => (p === 'meters' ? null : 'meters')),
    reset:        () => dispatch({ type: 'reset' }),
    toggleGrid:   () => setShowGrid(s => !s)
  });

  const applyPreset = (p:PresetItem) => dispatch({type:'batch', patch:p.state});
  const update = useCallback(<K extends keyof BTZPluginState>(k:K, v:BTZPluginState[K]) => {
    dispatch({ type:'set', key:k, value:v });
  }, []);

  const setParam = useCallback(
    <K extends keyof BTZPluginState>(k: K) =>
      (v: BTZPluginState[K]) => dispatch({ type: 'set', key: k, value: v }),
    []
  );

  const peakNorm = useMemo(() => Math.max(0, Math.min(1, (peak + 20) / 20)), [peak]);

  return (
    <div className="relative rounded-lg bg-neutral-950 p-4 text-white">
      <OverlayGrid show={showGrid} />

      {/* Top strip: mini meters */}
      <div className="mb-4">
        <MiniMeterStrip lufs={lufs} truePeakDb={peak} peakNorm={peakNorm} />
      </div>

      {/* Visualizer + five core knobs */}
      <div className="grid grid-cols-1 gap-4 md:grid-cols-[1fr,420px]">
        <div className="flex flex-col gap-4">
          <CentralVisualizerCanvas peakNorm={peakNorm} />
          <div className="grid grid-cols-5 gap-4">
            {[
              {k:'punch', cA:'#ff2fb9', cB:'#39ff88', label:'PUNCH'},
              {k:'warmth',cA:'#39ff88', cB:'#2fd3ff', label:'WARMTH'},
              {k:'boom',  cA:'#ff8a00', cB:'#ff2fb9', label:'BOOM'},
              {k:'mix',   cA:'#2fd3ff', cB:'#8a2be2', label:'MIX', toDisplay:(v:number)=>`${Math.round(v*100)}%`},
              {k:'drive', cA:'#ff8a00', cB:'#ff2fb9', label:'DRIVE', toDisplay:(v:number)=>`${(v*24-12).toFixed(1)} dB`},
            ].map(n=>(
              <ThermalKnob
                key={n.k}
                label={n.label}
                value={state[n.k as keyof BTZPluginState] as number}
                onChange={setParam(n.k as keyof BTZPluginState)}
                colorA={n.cA} colorB={n.cB}
                toDisplay={n.toDisplay as any}
              />
            ))}
          </div>
        </div>

        {/* Right controls column */}
        <div className="rounded-lg bg-white/5 p-3">
          <div className="mb-2 text-[10px] uppercase tracking-widest opacity-70">Output</div>
          <div className="mb-4 grid grid-cols-2 gap-3">
            <div className="rounded bg-white/5 px-2 py-1 text-xs">LUFS {lufs.toFixed(1)}</div>
            <div className="rounded bg-white/5 px-2 py-1 text-xs">PEAK {peak.toFixed(1)} dBTP</div>
          </div>

          <div className="space-y-3">
            <ModKnob
              label="Spark"
              on={state.sparkEnabled}
              value={state.sparkMix}
              setOn={(v)=>update('sparkEnabled', v)}
              setVal={(v)=>update('sparkMix', v)}
              colorA="#ff8a00"
              colorB="#ff2fb9"
              open={()=>setPanel('spark')}
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

          <div className="mt-3 text-[10px] opacity-60">Click a module knob to open its deep panel.</div>

          <PresetStrip presets={PRESETS} onApply={applyPreset} />
        </div>
      </div>

      {/* Panel Drawer */}
      <PanelDrawer open={!!panel} onClose={()=>setPanel(null)} title={panel?.toUpperCase()}>
        {panel && renderPanel(panel, state, (k,v)=>update(k as any, v as any))}
      </PanelDrawer>
    </div>
  );
};