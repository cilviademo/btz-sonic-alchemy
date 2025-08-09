import React from 'react';
import { SectionCard } from '@/components/SectionCard';
import { ToggleButton } from '@/components/ToggleButton';
import { Slider } from '@/components/ui/slider';
import { BTZPluginState, GlueBlend } from './types';

export const MasterGluePanel: React.FC<{
  state: BTZPluginState;
  update: <K extends keyof BTZPluginState>(k: K, v: BTZPluginState[K]) => void;
}> = ({ state, update }) => {
  const set = (k: keyof BTZPluginState) => (v:any)=>update(k as any, v);

  return (
    <SectionCard title="MASTER GLUE/MAX" subtitle="Shadow Hills glue → L-style limiting">
      <div className="flex items-center justify-between mb-4">
        <ToggleButton value={!!state.masterEnabled} onChange={set('masterEnabled')} label="Enabled" />
        <ModeSwitch value={(state.masterBlend ?? 'transparent') as GlueBlend} onChange={set('masterBlend') as any} />
      </div>

      <Row label="Macro" value={`${Math.round((state.masterMacro ?? .5)*100)}%`}>
        <Slider min={0} max={100} step={1}
          value={[Math.round((state.masterMacro ?? .5)*100)]}
          onValueChange={(v)=>set('masterMacro')((v[0]??0)/100)} />
      </Row>

      <Row label="Mix" value={`${Math.round((state.masterMix ?? 1)*100)}%`}>
        <Slider min={0} max={100} step={1}
          value={[Math.round((state.masterMix ?? 1)*100)]}
          onValueChange={(v)=>set('masterMix')((v[0]??0)/100)} />
      </Row>

      <div className="grid md:grid-cols-2 gap-6 mt-2">
        <Section label="Compressor">
          <Mini label="Attack" unit="ms" min={0.1} max={50} step={0.1}
            value={state.masterCompAttack ?? 10} onChange={set('masterCompAttack')} />
          <Mini label="Release" unit="ms" min={10} max={1000} step={1}
            value={state.masterCompRelease ?? 100} onChange={set('masterCompRelease')} />
          <Mini label="Ratio" unit=":" min={1} max={10} step={0.1}
            value={state.masterCompRatio ?? 2} onChange={set('masterCompRatio')} />
        </Section>
        <Section label="Limiter">
          <Mini label="Ceiling" unit="dB" min={-1} max={0} step={0.1}
            value={state.masterLimiterCeiling ?? -0.3} onChange={set('masterLimiterCeiling')} />
          <Mini label="Lookahead" unit="ms" min={0.1} max={5} step={0.1}
            value={state.masterLimiterLookahead ?? 0.8} onChange={set('masterLimiterLookahead')} />
        </Section>
      </div>
    </SectionCard>
  );
};

const ModeSwitch: React.FC<{value: GlueBlend; onChange:(v:GlueBlend)=>void}> = ({ value, onChange }) => {
  const modes: GlueBlend[] = ['vintage','digital','transparent'];
  return (
    <div className="flex gap-2">
      {modes.map(m => (
        <button key={m} onClick={()=>onChange(m)}
          className={`px-3 py-2 rounded border text-xs tracking-[.18em]
            ${value===m ? 'bg-foreground text-background border-transparent' : 'bg-plugin-surface border-foreground/10'}`}>
          {m.toUpperCase()}
        </button>
      ))}
    </div>
  );
};

const Row: React.FC<{label:string; value:string; children:React.ReactNode}> = ({label, value, children}) => (
  <div className="grid grid-cols-[140px_1fr_80px] items-center gap-3 mb-3">
    <div className="text-[11px] tracking-[.22em] font-semibold opacity-70">{label}</div>
    <div>{children}</div>
    <div className="text-right text-sm font-mono opacity-80">{value}</div>
  </div>
);

const Section: React.FC<{label:string; children:React.ReactNode}> = ({label, children}) => (
  <div className="rounded-lg border border-foreground/10 p-4">
    <div className="text-[11px] tracking-[.18em] font-semibold opacity-70 mb-3">{label}</div>
    <div className="grid gap-3">{children}</div>
  </div>
);

const Mini: React.FC<{label:string; unit:string; min:number; max:number; step:number; value:number; onChange:(v:number)=>void}> =
({ label, unit, min, max, step, value, onChange }) => (
  <div className="grid grid-cols-[90px_1fr_70px] items-center gap-2">
    <div className="text-xs opacity-70">{label}</div>
    <Slider min={min} max={max} step={step} value={[value]} onValueChange={(v)=>onChange(v[0]??value)} />
    <div className="text-right text-sm font-mono">{`${value}${unit}`}</div>
  </div>
);
