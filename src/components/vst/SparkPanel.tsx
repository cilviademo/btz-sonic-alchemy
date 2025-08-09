import React from 'react';
import { SectionCard } from '@/components/SectionCard';
import { ToggleButton } from '@/components/ToggleButton';
import { Slider } from '@/components/ui/slider';
import { BTZPluginState, ClipType } from './types';

export const SparkPanel: React.FC<{
  state: BTZPluginState;
  update: <K extends keyof BTZPluginState>(k: K, v: BTZPluginState[K]) => void;
}> = ({ state, update }) => {
  const set = (k: keyof BTZPluginState) => (v:any)=>update(k as any, v);

  return (
    <SectionCard title="SPARK — Master Clipper" subtitle="Hard-hitting, true-peak safe">
      <div className="flex items-center justify-between mb-4">
        <ToggleButton value={!!state.sparkEnabled} onChange={set('sparkEnabled')} label="Enabled" />
        <div className="text-xs opacity-70">
          Auto OS:
          <button className={`ml-2 px-2 py-1 rounded ${state.sparkAutoOS ? 'bg-foreground text-background' : 'bg-plugin-surface'}`}
            onClick={()=>set('sparkAutoOS')(!state.sparkAutoOS)}>
            {state.sparkAutoOS ? 'ON' : 'OFF'}
          </button>
        </div>
      </div>

      <GridRow label="Target LUFS" value={`${(state.sparkLUFS ?? -5).toFixed(1)}`}>
        <Slider min={-14} max={0} step={0.1} value={[state.sparkLUFS ?? -5]}
          onValueChange={(v)=>set('sparkLUFS')(v[0])}/>
      </GridRow>

      <GridRow label="TP Ceiling" value={`${(state.sparkCeiling ?? -0.3).toFixed(1)} dB`}>
        <Slider min={-3} max={0} step={0.1} value={[state.sparkCeiling ?? -0.3]}
          onValueChange={(v)=>set('sparkCeiling')(v[0])}/>
      </GridRow>

      <GridRow label="Mix" value={`${Math.round((state.sparkMix ?? 1)*100)}%`}>
        <Slider min={0} max={100} step={1}
          value={[Math.round((state.sparkMix ?? 1)*100)]}
          onValueChange={(v)=>set('sparkMix')((v[0]??0)/100)}/>
      </GridRow>

      <div className="grid grid-cols-5 gap-3 my-4">
        {(['soft','hard','tube','tape','digital'] as ClipType[]).map(m => (
          <button key={m} onClick={()=>set('sparkMode')(m)}
            className={`px-3 py-2 rounded border text-xs tracking-[.18em]
              ${state.sparkMode===m ? 'bg-audio-warning text-background border-transparent' : 'bg-plugin-surface border-foreground/10'}`}>
            {m.toUpperCase()}
          </button>
        ))}
      </div>

      <GridRow label="Oversampling" value={`${state.sparkOS ?? 8}×`}>
        <Slider min={0} max={4} step={1}
          value={[([1,2,4,8,16] as const).indexOf(state.sparkOS ?? 8)]}
          onValueChange={(v)=>set('sparkOS')(([1,2,4,8,16] as const)[v[0] ?? 3])}/>
      </GridRow>

      <div className="mt-3 text-xs opacity-70">
        Dual-stage clip (pre-sat → primary) with 0.8ms lookahead limiter and true-peak ceiling. Auto-OS up to 16×.
      </div>
    </SectionCard>
  );
};

const GridRow: React.FC<{label:string; value:string; children:React.ReactNode}> = ({label, value, children}) => (
  <div className="grid grid-cols-[140px_1fr_80px] items-center gap-3 mb-3">
    <div className="text-[11px] tracking-[.18em] font-semibold opacity-70">{label}</div>
    <div>{children}</div>
    <div className="text-right text-sm font-mono opacity-80">{value}</div>
  </div>
);

