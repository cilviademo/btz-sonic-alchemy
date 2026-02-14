import React from 'react';
import type { BTZPluginState } from '@/btz/types';

export interface PresetItem {
  id: string;
  label: string;
  state: Partial<BTZPluginState>;
}

export const PresetStrip: React.FC<{
  presets: PresetItem[];
  onApply: (p:PresetItem)=>void;
}> = ({ presets, onApply }) => {
  return (
    <div className="mt-4">
      <div className="mb-1 text-[10px] uppercase tracking-widest opacity-70">Presets</div>
      <div className="flex gap-2 overflow-x-auto pb-2">
        {presets.map(p => (
          <button
            key={p.id}
            type="button"
            className="rounded bg-white/5 px-2 py-1 text-xs hover:bg-white/10 focus:outline-none focus:ring-1 focus:ring-white/30"
            onClick={()=>onApply(p)}
          >
            {p.label}
          </button>
        ))}
      </div>
      <div className="text-[10px] opacity-60">Tip: drag to scroll • Enter to apply (focused)</div>
    </div>
  );
};