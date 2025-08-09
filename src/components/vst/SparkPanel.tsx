import React from 'react';
import { Slider } from '@/components/ui/slider';
import { BTZPluginState } from './types';
import { VUMeterNeedle } from '@/components/arturia/VUMeterNeedle';

export const SparkPanel: React.FC<{
  state: BTZPluginState;
  meters: any;
  updateParameter: (key: keyof BTZPluginState, value: any) => void;
}> = ({ state, meters, updateParameter }) => {
  const lufs = state.sparkLufsTarget ?? state.lufsTarget ?? -5;
  const ceiling = state.sparkCeilingDb ?? -0.3;
  const mix = Math.round((state.sparkMix ?? state.clippingBlend ?? 1) * 100);
  const os = state.sparkOversampling ?? 'auto';

  const set = (k: keyof BTZPluginState) => (v: any) => updateParameter(k, v);

  return (
    <div className="grid gap-6">
      {/* Top controls */}
      <div className="grid md:grid-cols-4 gap-6 items-center">
        {/* Enable */}
        <div className="space-y-2">
          <div className="text-[11px] tracking-[.18em] font-semibold opacity-70">Enabled</div>
          <button
            onClick={() => set('sparkEnabled')(!(state.sparkEnabled ?? state.clippingEnabled ?? true))}
            className={(state.sparkEnabled ?? state.clippingEnabled ?? true)
              ? 'px-3 py-2 rounded-md border bg-[hsl(var(--audio-success))] text-[hsl(var(--background))]'
              : 'px-3 py-2 rounded-md border bg-[hsl(var(--plugin-surface))] text-[hsl(var(--foreground)/0.8)]'}
            aria-pressed={!!(state.sparkEnabled ?? state.clippingEnabled)}
          >
            {(state.sparkEnabled ?? state.clippingEnabled ?? true) ? 'ON' : 'OFF'}
          </button>
        </div>

        {/* LUFS target */}
        <div className="space-y-2">
          <div className="text-[11px] tracking-[.18em] font-semibold opacity-70">LUFS Target</div>
          <div className="flex items-center gap-3">
            <div className="w-24 text-sm font-medium">{lufs.toFixed(1)} LUFS</div>
            <div className="flex-1">
              <Slider min={-14} max={0} step={0.1} value={[lufs]}
                onValueChange={(v)=>{ set('sparkLufsTarget')(v[0]); set('lufsTarget')(v[0]); }} />
            </div>
          </div>
        </div>

        {/* Ceiling */}
        <div className="space-y-2">
          <div className="text-[11px] tracking-[.18em] font-semibold opacity-70">Ceiling (dBTP)</div>
          <div className="flex items-center gap-3">
            <div className="w-16 text-sm font-medium">{ceiling.toFixed(1)} dB</div>
            <div className="flex-1">
              <Slider min={-3} max={0} step={0.1} value={[ceiling]}
                onValueChange={(v)=> set('sparkCeilingDb')(v[0])} />
            </div>
          </div>
        </div>

        {/* Mix */}
        <div className="space-y-2">
          <div className="text-[11px] tracking-[.18em] font-semibold opacity-70">Blend</div>
          <div className="flex items-center gap-3">
            <div className="w-10 text-sm font-medium">{mix}%</div>
            <div className="flex-1">
              <Slider min={0} max={100} step={1} value={[mix]}
                onValueChange={(v)=>{ const val=(v[0]??0)/100; set('sparkMix')(val); set('clippingBlend')(val); }} />
            </div>
          </div>
        </div>
      </div>

      {/* Oversampling */}
      <div className="flex items-center gap-3">
        <div className="text-[11px] tracking-[.18em] font-semibold opacity-70">Oversampling</div>
        <div className="flex gap-2">
          {(['auto',1,2,4,8,16] as const).map(o => (
            <button key={String(o)} onClick={()=> set('sparkOversampling')(o)}
              className={(os===o) ? 'px-3 py-1.5 rounded-md border bg-[hsl(var(--foreground))] text-[hsl(var(--background))]'
                                   : 'px-3 py-1.5 rounded-md border bg-[hsl(var(--plugin-surface))] text-[hsl(var(--foreground)/0.8)]'}>
              {String(o).toUpperCase()}{o==='auto'?'':'x'}
            </button>
          ))}
        </div>
      </div>

      {/* Visuals */}
      <div className="grid md:grid-cols-3 gap-6 items-end">
        <div className="space-y-2">
          <div className="text-[11px] tracking-[.18em] font-semibold opacity-70">Input</div>
          <VUMeterNeedle value={Math.max(0, Math.min(1, meters.inputLevel || 0))} label="IN" />
        </div>
        <div className="space-y-2">
          <div className="text-[11px] tracking-[.18em] font-semibold opacity-70">Output</div>
          <VUMeterNeedle value={Math.max(0, Math.min(1, meters.outputLevel || 0))} label="OUT" />
        </div>
        <div className="space-y-2">
          <div className="text-[11px] tracking-[.18em] font-semibold opacity-70">GR (approx)</div>
          <VUMeterNeedle value={Math.max(0, Math.min(1, Math.abs((meters.inputLevel||0)-(meters.outputLevel||0))))} label="GR" />
        </div>
      </div>
    </div>
  );
}
