import React from 'react';
import { cn } from '@/lib/utils';

type ModuleKnobProps = {
  id: string;
  label: string;
  valuePct?: number;          // 0..100 for ring fill
  colorA?: string;
  colorB?: string;
  onOpen: (id: string) => void;
  disabled?: boolean;
  onToggle?: () => void;      // optional on/off toggle
  enabled?: boolean;
};

export const ModuleKnob: React.FC<ModuleKnobProps> = ({
  id, label, valuePct = 0, colorA = '#ff7d00', colorB = '#6cf', onOpen, disabled,
  onToggle, enabled = true,
}) => {
  const v = Math.max(0, Math.min(100, valuePct));
  return (
    <div className="group flex flex-col items-center select-none">
      <button
        onClick={() => !disabled && onOpen(id)}
        className={cn(
          'relative grid place-items-center rounded-full w-[96px] h-[96px] md:w-[110px] md:h-[110px] transition-all',
          'bg-[rgba(255,255,255,0.03)] hover:bg-[rgba(255,255,255,0.06)] border border-white/10',
          disabled && 'opacity-40 cursor-not-allowed'
        )}
        aria-label={`${label} details`}
      >
        <svg width="100%" height="100%" viewBox="0 0 100 100" className="absolute inset-0">
          <defs>
            <linearGradient id={`g-${id}`} x1="0" y1="0" x2="1" y2="1">
              <stop offset="0%" stopColor={colorA}/>
              <stop offset="100%" stopColor={colorB}/>
            </linearGradient>
          </defs>
          <circle cx="50" cy="50" r="42" stroke="rgba(255,255,255,.12)" strokeWidth="8" fill="none" />
          <circle
            cx="50" cy="50" r="42"
            stroke={`url(#g-${id})`} strokeWidth="8" fill="none"
            strokeDasharray={`${2*Math.PI*42}`}
            strokeDashoffset={`${2*Math.PI*42 * (1 - v/100)}`}
            transform="rotate(-90 50 50)" strokeLinecap="round"
          />
          <circle cx="50" cy="50" r="6" fill="rgba(255,255,255,.85)"/>
        </svg>
      </button>

      <div className="mt-2 text-[11px] tracking-[.22em] text-foreground/80 uppercase">{label}</div>
      <div className="text-[10px] text-foreground/60">{Math.round(v)}%</div>

      {onToggle && (
        <button
          onClick={onToggle}
          className={cn(
            'mt-2 px-3 py-1 rounded text-[10px] border',
            enabled ? 'bg-audio-primary text-background border-audio-primary' : 'bg-plugin-surface text-foreground/70 border-foreground/10'
          )}
          aria-pressed={enabled}
        >
          {enabled ? 'ON' : 'OFF'}
        </button>
      )}
    </div>
  );
};
