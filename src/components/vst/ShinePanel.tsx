import React from 'react';
import { Slider } from '@/components/ui/slider';
import { BTZPluginState } from './types';
import { ToggleButton } from '@/components/ToggleButton';
import { SectionCard } from '@/components/SectionCard';

export const ShinePanel: React.FC<{
  state: BTZPluginState;
  updateParameter: (key: keyof BTZPluginState, value: any) => void;
}> = ({ state, updateParameter }) => {
  const set = (k: keyof BTZPluginState) => (v: any) => updateParameter(k, v);
  const freq = state.shineFreqHz ?? 20000;
  const gain = state.shineGainDb ?? 3;
  const q = state.shineQ ?? 0.5;
  const mix = Math.round((state.shineMix ?? 0.5) * 100);

  return (
    <SectionCard title="SHINE — Air Band" subtitle="Ethereal highs (SSL Fusion • Maag)">
      <div className="flex items-center justify-between mb-4">
        <ToggleButton value={!!state.shineEnabled} onChange={set('shineEnabled')} label="Enabled" />
        <div className="text-xs opacity-70">
          Auto OS:
          <button className={`ml-2 px-2 py-1 rounded ${state.shineAutoOS ? 'bg-foreground text-background' : 'bg-plugin-surface'}`}
            onClick={()=>set('shineAutoOS')(!state.shineAutoOS)}>
            {state.shineAutoOS ? 'ON' : 'OFF'}
          </button>
        </div>
      </div>

      <div className="grid md:grid-cols-5 gap-6 items-center">
        <Field label="Frequency" value={`${Math.round(freq/1000)} kHz`}>
          <Slider min={10000} max={80000} step={100} value={[freq]}
            onValueChange={(v)=> set('shineFreqHz')(v[0])}/>
        </Field>
        <Field label="Gain" value={`${gain.toFixed(1)} dB`}>
          <Slider min={-12} max={12} step={0.1} value={[gain]}
            onValueChange={(v)=> set('shineGainDb')(v[0])}/>
        </Field>
        <Field label="Q" value={`${q.toFixed(2)}`}>
          <Slider min={0.1} max={2} step={0.01} value={[q]}
            onValueChange={(v)=> set('shineQ')(v[0])}/>
        </Field>
        <Field label="Blend" value={`${mix}%`} span={2}>
          <Slider min={0} max={100} step={1} value={[mix]}
            onValueChange={(v)=> set('shineMix')((v[0]??0)/100)}/>
        </Field>
        <div className="md:col-span-5">
          <button
            onClick={() => set('shineAB')(!(state.shineAB ?? false))}
            className={(state.shineAB ?? false)
              ? 'px-3 py-2 rounded-md border bg-[hsl(var(--foreground))] text-[hsl(var(--background))]'
              : 'px-3 py-2 rounded-md border bg-[hsl(var(--plugin-surface))] text-[hsl(var(--foreground)/0.8)]'}
          >
            A / B
          </button>
        </div>
      </div>

      <div className="text-xs opacity-70 mt-3">
        Ultrasonic EQ to <b>80 kHz</b>. Adaptive oversampling (≤ <b>16×</b>) on high-boost moves.
      </div>
    </SectionCard>
  );
};

const Field: React.FC<{label:string; value:string; span?:number; children:React.ReactNode}> =
({ label, value, span=1, children }) => (
  <div className={`space-y-2 ${span>1 ? `md:col-span-${span}` : ''}`}>
    <div className="text-[11px] tracking-[.18em] font-semibold opacity-70">{label}</div>
    <div className="flex items-center gap-3">
      <div className="w-24 text-sm font-medium">{value}</div>
      <div className="flex-1">{children}</div>
    </div>
  </div>
);

