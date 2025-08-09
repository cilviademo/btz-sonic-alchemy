import React, { useRef } from 'react';
import { EnhancedPreset, BTZPluginState } from './types';
import { SectionCard } from '@/components/SectionCard';

export const PresetScroller: React.FC<{
  presets: EnhancedPreset[];
  proStyles: { id: string; label: string; state: Partial<BTZPluginState> }[];
  onApply: (p: EnhancedPreset) => void;
  onApplyStyle: (p: { id: string; label: string; state: Partial<BTZPluginState> }) => void;
}> = ({ presets, proStyles, onApply, onApplyStyle }) => {
  const rail = useRef<HTMLDivElement>(null);

  const scrollBy = (dx: number) => {
    const el = rail.current; if (!el) return;
    el.scrollTo({ left: el.scrollLeft + dx, behavior: 'smooth' });
  };

  return (
    <SectionCard title="PRESETS" subtitle="">
      <div className="relative">
        <button aria-label="scroll left" onClick={() => scrollBy(-300)}
          className="absolute left-0 top-1/2 -translate-y-1/2 z-10 px-2 py-2 rounded-full bg-plugin-surface/80 border border-foreground/10">‹</button>

        <div ref={rail}
             className="no-scrollbar overflow-x-auto flex gap-3 px-8 py-2 snap-x snap-mandatory"
             style={{ scrollSnapType: 'x proximity' }}>
          {presets.map(p => (
            <button key={p.id}
              onClick={() => onApply(p)}
              className="snap-start whitespace-nowrap px-4 py-2 rounded-lg border border-foreground/10 bg-plugin-surface hover:bg-plugin-panel text-xs font-semibold tracking-[.22em]">
              {p.label.toUpperCase()}
            </button>
          ))}

          {/* Divider */}
          <div className="w-px bg-foreground/10 mx-2" />

          {proStyles.map(p => (
            <button key={p.id}
              onClick={() => onApplyStyle(p)}
              className="snap-start whitespace-nowrap px-4 py-2 rounded-lg border border-foreground/10 bg-plugin-surface/70 hover:bg-plugin-surface text-xs tracking-[.18em]">
              {p.label}
            </button>
          ))}
        </div>

        <button aria-label="scroll right" onClick={() => scrollBy(300)}
          className="absolute right-0 top-1/2 -translate-y-1/2 z-10 px-2 py-2 rounded-full bg-plugin-surface/80 border border-foreground/10">›</button>
      </div>

      <div className="text-[11px] opacity-60 mt-2">Tip: swipe/drag to scroll • mouse-wheel ⇄ • ← / → keys • Enter to apply focused item.</div>
    </SectionCard>
  );
};
