import React from 'react';
import { cn } from '@/lib/utils';

interface Props {
  currentSlot: 'A' | 'B';
  onSwitch: () => void;
  onCopyAtoB: () => void;
  onCopyBtoA: () => void;
}

export const ABToggle: React.FC<Props> = ({ currentSlot, onSwitch, onCopyAtoB, onCopyBtoA }) => {
  return (
    <div className="flex items-center gap-2">
      <button
        type="button"
        onClick={onSwitch}
        className={cn(
          "flex items-center gap-1 rounded px-3 py-1.5 text-xs font-semibold uppercase tracking-wider transition-all",
          "hover:bg-white/10"
        )}
        title="Toggle A/B comparison (Shift+A)"
      >
        <span className={cn(
          "transition-colors",
          currentSlot === 'A' ? "text-white" : "text-white/40"
        )}>
          A
        </span>
        <span className="text-white/60">/</span>
        <span className={cn(
          "transition-colors",
          currentSlot === 'B' ? "text-white" : "text-white/40"
        )}>
          B
        </span>
      </button>

      <div className="flex gap-1">
        <button
          type="button"
          onClick={onCopyAtoB}
          className="rounded bg-white/5 px-2 py-1 text-[10px] uppercase opacity-60 hover:opacity-100"
          title="Copy A → B"
        >
          A→B
        </button>
        <button
          type="button"
          onClick={onCopyBtoA}
          className="rounded bg-white/5 px-2 py-1 text-[10px] uppercase opacity-60 hover:opacity-100"
          title="Copy B → A"
        >
          B→A
        </button>
      </div>
    </div>
  );
};
