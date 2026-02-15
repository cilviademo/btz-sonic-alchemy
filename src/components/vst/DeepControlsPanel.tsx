import React from 'react';
import { SectionCard } from '@/components/SectionCard';
import { ToggleButton } from '@/components/vst/ToggleButton';
import { Slider } from '@/components/ui/slider';
import { BTZPluginState } from './types';

export const DeepControlsPanel: React.FC<{
  state: BTZPluginState;
  update: <K extends keyof BTZPluginState>(k: K, v: BTZPluginState[K]) => void;
}> = ({ state, update }) => {
  const set = (k: keyof BTZPluginState) => (v:any)=>update(k as any, v);

  return (
    <SectionCard title="DEEP CONTROLS — Precision" subtitle="Numeric editing for surgical tweaks">
      <div className="flex flex-wrap gap-3 mb-5">
        <Toggle label="Transient"  v={!!state.transEnabled}  on={()=>set('transEnabled')(!state.transEnabled)} />
        <Toggle label="EQ"         v={!!state.eqEnabled}     on={()=>set('eqEnabled')(!state.eqEnabled)} />
        <Toggle label="Dynamics"   v={!!state.dynEnabled}    on={()=>set('dynEnabled')(!state.dynEnabled)} />
        <Toggle label="Sub"        v={!!state.subEnabled}    on={()=>set('subEnabled')(!state.subEnabled)} />
        <Toggle label="Console"    v={!!state.consoleEnabled}on={()=>set('consoleEnabled')(!state.consoleEnabled)} />
      </div>

      <div className="grid md:grid-cols-2 gap-8">
        <Block title="Transient" enabled={!!state.transEnabled}>
          <Row label="Attack"   value={`${state.transAttack ?? 0}%`}  min={-100} max={100} step={1} on={(v)=>set('transAttack')(v)} />
          <Row label="Sustain"  value={`${state.transSustain ?? 0}%`} min={-100} max={100} step={1} on={(v)=>set('transSustain')(v)} />
          <div className="grid grid-cols-3 gap-2">
            {(['wide','mid','tight'] as const).map(m => (
              <button key={m} onClick={()=>set('transDetect')(m)}
                className={`px-3 py-2 rounded border text-xs ${state.transDetect===m?'bg-foreground text-background':'bg-plugin-surface'}`}>
                {m.toUpperCase()}
              </button>
            ))}
          </div>
        </Block>

        <Block title="EQ" enabled={!!state.eqEnabled}>
          <Row label="Low Gain"  value={`${state.eqLowGain ?? 0} dB`}   min={-12} max={12} step={0.1} on={(v)=>set('eqLowGain')(v)} />
          <Row label="Low Freq"  value={`${state.eqLowFreq ?? 80} Hz`}  min={20} max={300} step={1}   on={(v)=>set('eqLowFreq')(v)} />
          <Row label="Mid Gain"  value={`${state.eqMidGain ?? 0} dB`}   min={-12} max={12} step={0.1} on={(v)=>set('eqMidGain')(v)} />
          <Row label="Mid Freq"  value={`${state.eqMidFreq ?? 1200} Hz`}min={200} max={6000} step={1} on={(v)=>set('eqMidFreq')(v)} />
          <Row label="Mid Q"     value={`${state.eqMidQ ?? 1}`}         min={0.2} max={5} step={0.05} on={(v)=>set('eqMidQ')(v)} />
          <Row label="High Gain" value={`${state.eqHighGain ?? 0} dB`}  min={-12} max={12} step={0.1} on={(v)=>set('eqHighGain')(v)} />
          <Row label="High Freq" value={`${state.eqHighFreq ?? 8000} Hz`}min={2000} max={16000} step={10} on={(v)=>set('eqHighFreq')(v)} />
        </Block>

        <Block title="Dynamics" enabled={!!state.dynEnabled}>
          <Row label="Threshold" value={`${state.dynThreshold ?? 0} dB`} min={-48} max={0} step={0.5} on={(v)=>set('dynThreshold')(v)} />
          <Row label="Ratio"     value={`${state.dynRatio ?? 2}:1`}     min={1} max={20} step={0.1} on={(v)=>set('dynRatio')(v)} />
          <Row label="Attack"    value={`${state.dynAttack ?? 8} ms`}   min={0.1} max={50} step={0.1} on={(v)=>set('dynAttack')(v)} />
          <Row label="Release"   value={`${state.dynRelease ?? 120} ms`}min={10} max={1000} step={1}  on={(v)=>set('dynRelease')(v)} />
          <Row label="Knee"      value={`${state.dynKnee ?? 2} dB`}     min={0} max={12} step={0.5} on={(v)=>set('dynKnee')(v)} />
        </Block>

        <Block title="Sub / Console" enabled={(state.subEnabled || state.consoleEnabled) ?? false}>
          <Row label="Sub Amt"      value={`${state.subAmount ?? 0}%`}           min={0} max={100} step={1} on={(v)=>set('subAmount')(v)} />
          <Row label="Sub Freq"     value={`${state.subFreq ?? 50} Hz`}          min={30} max={120} step={1} on={(v)=>set('subFreq')(v)} />
          <Row label="Console Drive" value={`${Math.round((state.consoleDrive ?? .15)*100)}%`} min={0} max={100} step={1}
               on={(v)=>set('consoleDrive')((v??0)/100)} />
          <Row label="Crosstalk"     value={`${Math.round((state.consoleCrosstalk ?? .1)*100)}%`} min={0} max={100} step={1}
               on={(v)=>set('consoleCrosstalk')((v??0)/100)} />
        </Block>
      </div>
    </SectionCard>
  );
};

const Toggle: React.FC<{label:string; v:boolean; on:()=>void}> = ({label,v,on}) => (
  <button onClick={on}
    className={`px-3 py-2 rounded border text-xs tracking-[.18em]
      ${v ? 'bg-foreground text-background border-transparent' : 'bg-plugin-surface border-foreground/10'}`}>
    {label.toUpperCase()} {v ? 'ON' : 'OFF'}
  </button>
);

const Block: React.FC<{title:string; enabled:boolean; children:React.ReactNode}> =
({ title, enabled, children }) => (
  <div className={`rounded-lg border p-4 ${enabled ? 'border-foreground/15' : 'border-foreground/10 opacity-60'}`}>
    <div className="text-[11px] tracking-[.18em] font-semibold opacity-70 mb-3">{title}</div>
    {children}
  </div>
);

const Row: React.FC<{label:string; value:string; min:number; max:number; step:number; on:(v:number)=>void}> =
({ label, value, min, max, step, on }) => (
  <div className="grid grid-cols-[120px_1fr_80px] items-center gap-3 mb-2">
    <div className="text-xs opacity-70">{label}</div>
    <Slider min={min} max={max} step={step} onValueChange={(v)=>on(v[0]??0)} />
    <div className="text-right text-sm font-mono">{value}</div>
  </div>
);
