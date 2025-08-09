import React, { useEffect, useReducer, useState } from 'react';
import './theme.css';
import { cn } from '@/lib/utils';
import type { BTZPluginState } from './types';
import { ThermalKnob } from './ThermalKnob';
import { CentralVisualizerCanvas } from './CentralVisualizerCanvas';
import { PresetStrip, type PresetItem } from './PresetStrip';
import { MiniMeterStrip } from './MiniMeterStrip';
import { OverlayGrid } from './OverlayGrid';
import { PanelDrawer } from './PanelDrawer';
import { renderPanel, type PanelId } from './PanelRegistry';

// ---- defaults & presets (compatible with existing types)
const DEFAULT_STATE: BTZPluginState = {
  punch:0, warmth:0, boom:0, mix:1, drive:0,
  texture:false, active:true, oversampling:true,
  sparkEnabled:true, sparkLUFS:-5, sparkCeiling:-0.3, sparkMix:1, sparkOS:8, sparkAutoOS:true, sparkMode:'soft',
  shineEnabled:false, shineFreqHz:20000, shineGainDb:3, shineQ:0.5, shineMix:0.5, shineAutoOS:true,
  masterEnabled:false, masterMacro:.5, masterBlend:'transparent', masterMix:1,
  transEnabled:false, eqEnabled:false, dynEnabled:false, subEnabled:false, consoleEnabled:false
};

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
];

// ---- reducer
 type Act =
  | {type:'set'; key:keyof BTZPluginState; value:any}
  | {type:'batch'; patch:Partial<BTZPluginState>}
  | {type:'reset'};
function reducer(s:BTZPluginState, a:Act):BTZPluginState {
  switch(a.type){
    case 'set': return {...s, [a.key]: a.value};
    case 'batch': return {...s, ...a.patch};
    case 'reset': return {...DEFAULT_STATE};
    default: return s;
  }
}

export const EnhancedBTZPlugin:React.FC = () => {
  const [state, dispatch] = useReducer(reducer, DEFAULT_STATE);
  const [lufs, setLUFS] = useState(-14.2);
  const [peak, setPeak] = useState(-0.7);
  const [showGrid, setShowGrid] = useState(false);
  const [panel, setPanel] = useState<PanelId | null>(null);

  // simple analyser sim to keep UI alive (replace with real engine updates)
  useEffect(() => {
    let raf=0;
    const tick=(t:number)=>{ raf=requestAnimationFrame(tick);
      const out = 0.25 + state.drive*.3 + state.punch*.2 + state.boom*.25;
      setLUFS(-14 + out*4 + Math.sin(t*.001)*.4);
      setPeak(-1 + out*.8 + Math.sin(t*.0015)*.2);
    };
    raf=requestAnimationFrame(tick);
    return()=>cancelAnimationFrame(raf);
  }, [state.drive, state.punch, state.boom]);

  // quick grid toggle (G)
  useEffect(()=>{
    const on=(e:KeyboardEvent)=>{ if(e.key.toLowerCase()==='g') setShowGrid(s=>!s); };
    window.addEventListener('keydown', on);
    return ()=>window.removeEventListener('keydown', on);
  },[]);

  const applyPreset = (p:PresetItem) => dispatch({type:'batch', patch:p.state});
  const update = <K extends keyof BTZPluginState>(k: K, v: BTZPluginState[K]) => dispatch({type:'set', key:k, value:v});

  const peakNorm = Math.max(0, Math.min(1, (peak + 20) / 20));

  return (
    <div className="relative plugin-surface text-white/90 rounded-2xl" style={{width:1200, height:700}}>
      <OverlayGrid show={showGrid} />
      {/* top mini meters */}
      <MiniMeterStrip lufs={lufs} truePeakDb={peak} peakNorm={peakNorm} />

      <div className="grid grid-cols-12 gap-4 p-4 pt-1">
        {/* Visualizer + core knobs */}
        <div className="col-span-8 card p-4">
          <CentralVisualizerCanvas level={Math.max(state.punch,state.drive,state.boom)} />
          <div className="mt-5 flex items-center justify-between">
            {[
              {k:'punch', cA:'#ff2fb9', cB:'#39ff88', label:'PUNCH'},
              {k:'warmth',cA:'#39ff88', cB:'#2fd3ff', label:'WARMTH'},
              {k:'boom',  cA:'#ff8a00', cB:'#ff2fb9', label:'BOOM'},
              {k:'mix',   cA:'#2fd3ff', cB:'#8a2be2', label:'MIX'},
              {k:'drive', cA:'#ff8a00', cB:'#ff2fb9', label:'DRIVE'},
            ].map(n=>(
              <div key={n.k as string} className="group text-center">
                <ThermalKnob
                  label={n.label}
                  value={(state as any)[n.k] ?? 0}
                  onChange={(v)=>dispatch({type:'set', key:n.k as any, value:v})}
                  colorA={n.cA} colorB={n.cB}
                />
                <button
                  className="block mx-auto mt-1 text-[10px] text-white/40 opacity-0 group-hover:opacity-100 transition"
                  onClick={()=>setPanel('deep')}
                >
                  PRECISION…
                </button>
              </div>
            ))}
          </div>

        {/* Output + module knobs */}
        <div className="col-span-4 space-y-4">
          <div className="card p-4">
            <div className="text-[11px] tracking-[.22em] text-white/70 mb-3">OUTPUT</div>
            <div className="h-[140px] rounded-xl bg-black/25 border border-white/5 flex items-center justify-center">
              <div className="w-full px-4">
                <div className="h-2 rounded-full bg-white/8 relative overflow-hidden">
                  <div
                    className="absolute inset-y-0 left-0 bg-[linear-gradient(90deg,#2fd3ff,#ff2fb9,#ff8a00)]"
                    style={{width:`${Math.min(100, Math.max(0, (lufs+30)/30*100))}%`}}
                  />
                </div>
                <div className="mt-2 text-[10px] text-white/60 flex justify-between">
                  <span>LUFS {lufs.toFixed(1)}</span>
                  <span>PEAK {peak.toFixed(1)} dBTP</span>
                </div>
              </div>
            </div>
          </div>

          <div className="card p-4">
            <div className="grid grid-cols-3 gap-3">
              <ModKnob label="SPARK" on={!!state.sparkEnabled}
                       value={state.sparkMix ?? 1}
                       setOn={(v)=>dispatch({type:'set',key:'sparkEnabled',value:v})}
                       setVal={(v)=>dispatch({type:'set',key:'sparkMix',value:v})}
                       colorA="#ff8a00" colorB="#ff2fb9" open={()=>setPanel('spark')}/>
              <ModKnob label="SHINE" on={!!state.shineEnabled}
                       value={state.shineMix ?? 0.5}
                       setOn={(v)=>dispatch({type:'set',key:'shineEnabled',value:v})}
                       setVal={(v)=>dispatch({type:'set',key:'shineMix',value:v})}
                       colorA="#2fd3ff" colorB="#8a2be2" open={()=>setPanel('shine')}/>
              <ModKnob label="MASTER" on={!!state.masterEnabled}
                       value={state.masterMix ?? 1}
                       setOn={(v)=>dispatch({type:'set',key:'masterEnabled',value:v})}
                       setVal={(v)=>dispatch({type:'set',key:'masterMix',value:v})}
                       colorA="#39ff88" colorB="#2fd3ff" open={()=>setPanel('master')}/>
              <ModKnob label="AI" on={true}
                       value={1} setOn={()=>{}} setVal={()=>{}} colorA="#ff6a3d" colorB="#ff2fb9" disabled open={()=>setPanel('meters')}/>
              <ModKnob label="CONVOLVER" on={!!state.subEnabled}
                       value={(state as any).subAmount ?? 0}
                       setOn={(v)=>dispatch({type:'set',key:'subEnabled',value:v})}
                       setVal={(v)=>dispatch({type:'set',key:'subAmount' as any,value:v})}
                       colorA="#ff2fb9" colorB="#2fd3ff" open={()=>setPanel('convolver')}/>
              <ModKnob label="METERS" on={true} value={1} setOn={()=>{}} setVal={()=>{}} colorA="#8a2be2" colorB="#2fd3ff" disabled open={()=>setPanel('meters')}/>
            </div>
            <div className="mt-3 text:[10px] text-white/45">Click a module knob to open its deep panel.</div>
          </div>
        </div>

        {/* Presets */}
        <div className="col-span-12">
          <PresetStrip presets={PRESETS} onApply={applyPreset}/>
        </div>
      </div>
    </div>
  );
};

// small wrapper knob for module rows (with ON/OFF)
const ModKnob:React.FC<{
  label:string; on:boolean; value:number; setOn:(v:boolean)=>void; setVal:(v:number)=>void;
  colorA:string; colorB:string; disabled?:boolean; open?:()=>void;
}> = ({label,on,value,setOn,setVal,colorA,colorB,disabled,open})=>{
  return (
    <div className="text-center">
      <button className="group block mx-auto" onClick={!disabled ? open : undefined}>
        <ThermalKnob value={value} onChange={setVal} colorA={colorA} colorB={colorB} size={104} disabled={disabled}/>
        <div className="text-[10px] tracking-[.22em] text-white/70 mt-1">{label}</div>
      </button>
      <button
        className={cn(
          'mt-1 px-2 py-1 rounded text-[10px] border',
          on ? 'bg-orange-500/90 border-transparent' : 'bg-white/10 border-white/15'
        )}
        onClick={()=>!disabled && setOn(!on)}
        disabled={disabled}
      >
        {on ? 'ON' : 'OFF'}
      </button>
    </div>
  );
};
