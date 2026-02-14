import React from 'react';
import type { BTZPluginState } from './types';

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
    <div className="card p-3">
      <div className="text-[11px] tracking-[.22em] text-white/60 mb-2">PRESETS</div>
      <div className="flex gap-2 overflow-x-auto no-scrollbar">
        {presets.map(p => (
          <button
            key={p.id}
            className="px-4 py-2 rounded-xl border border-white/10 bg-white/5 hover:bg-white/10 text-sm whitespace-nowrap"
            onClick={()=>onApply(p)}
          >
            {p.label}
          </button>
        ))}
      </div>
      <div className="text-[10px] text-white/35 mt-2">
        Tip: drag to scroll • ←/→ prev/next • Enter to apply
      </div>
    </div>
  );
};
