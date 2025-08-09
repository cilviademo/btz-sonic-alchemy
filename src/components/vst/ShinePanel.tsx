import React from 'react';
import { Slider } from '@/components/ui/slider';
import { BTZPluginState } from './types';

export const ShinePanel: React.FC<{
  state: BTZPluginState;
  meters: any;
  updateParameter: (key: keyof BTZPluginState, value: any) => void;
}> = ({ state, updateParameter }) => {
  const freq = state.shineFreqHz ?? 20000;
  const gain = state.shineGainDb ?? 3;
  const q = state.shineQ ?? 0.5;
  const mix = Math.round((state.shineMix ?? 0.5) * 100);

  const set = (k: keyof BTZPluginState) => (v: any) => updateParameter(k, v);

  return (
    <div className="grid gap-6">
      <div className="grid md:grid-cols-5 gap-6 items-center">
        {/* Enable */}
        <div className="space-y-2">
          <div className="text-[11px] tracking-[.18em] font-semibold opacity-70">Enabled</div>
          <button
            onClick={() => set('shineEnabled')(!(state.shineEnabled ?? true))}
            className={(state.shineEnabled ?? true)
              ? 'px-3 py-2 rounded-md border bg-[hsl(var(--audio-primary))] text-[hsl(var(--background))]'
              : 'px-3 py-2 rounded-md border bg-[hsl(var(--plugin-surface))] text-[hsl(var(--foreground)/0.8)]'}
            aria-pressed={!!state.shineEnabled}
          >
            {(state.shineEnabled ?? true) ? 'ON' : 'OFF'}
          </button>
        </div>

        {/* Frequency */}
        <div className="space-y-2 md:col-span-2">
          <div className="text-[11px] tracking-[.18em] font-semibold opacity-70">Frequency</div>
          <div className="flex items-center gap-3">
            <div className="w-24 text-sm font-medium">{Math.round(freq/1000)} kHz</div>
            <div className="flex-1">
              <Slider min={10000} max={80000} step={100} value={[freq]}
                onValueChange={(v)=> set('shineFreqHz')(v[0])} />
            </div>
          </div>
        </div>

        {/* Gain */}
        <div className="space-y-2">
          <div className="text-[11px] tracking-[.18em] font-semibold opacity-70">Gain</div>
          <div className="flex items-center gap-3">
            <div className="w-16 text-sm font-medium">{gain.toFixed(1)} dB</div>
            <div className="flex-1">
              <Slider min={-12} max={12} step={0.1} value={[gain]}
                onValueChange={(v)=> set('shineGainDb')(v[0])} />
            </div>
          </div>
        </div>

        {/* Q / Resonance */}
        <div className="space-y-2">
          <div className="text-[11px] tracking-[.18em] font-semibold opacity-70">Q</div>
          <div className="flex items-center gap-3">
            <div className="w-10 text-sm font-medium">{q.toFixed(2)}</div>
            <div className="flex-1">
              <Slider min={0.1} max={2} step={0.01} value={[q]}
                onValueChange={(v)=> set('shineQ')(v[0])} />
            </div>
          </div>
        </div>
      </div>

      {/* Mix + A/B */}
      <div className="flex items-center gap-6">
        <div className="space-y-2 w-full max-w-sm">
          <div className="text-[11px] tracking-[.18em] font-semibold opacity-70">Blend</div>
          <div className="flex items-center gap-3">
            <div className="w-10 text-sm font-medium">{mix}%</div>
            <div className="flex-1">
              <Slider min={0} max={100} step={1} value={[mix]}
                onValueChange={(v)=> set('shineMix')((v[0]??0)/100)} />
            </div>
          </div>
        </div>
        <button
          onClick={() => set('shineAB')(!(state.shineAB ?? false))}
          className={(state.shineAB ?? false)
            ? 'px-3 py-2 rounded-md border bg-[hsl(var(--foreground))] text-[hsl(var(--background))]'
            : 'px-3 py-2 rounded-md border bg-[hsl(var(--plugin-surface))] text-[hsl(var(--foreground)/0.8)]'}
        >
          A / B
        </button>
      </div>

      <div className="text-xs opacity-70">
        Adaptive oversampling engages automatically at high boost to keep air silky and artifact-free.
      </div>
    </div>
  );
}
