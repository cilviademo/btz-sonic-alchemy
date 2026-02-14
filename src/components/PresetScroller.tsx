import React, { useMemo } from 'react';
import { useHorizontalDragScroll } from '@/hooks/useHorizontalDragScroll';
import { cn } from '@/lib/utils';

export interface PresetItem<T = any> {
  id: string;
  label: string;
  subtitle?: string;
  payload: T;
}

export const PresetScroller: React.FC<{
  title?: string;
  presets: PresetItem[];
  selectedId?: string;
  onSelect: (preset: PresetItem) => void;
  className?: string;
}> = ({ title = 'PRESETS', presets, selectedId, onSelect, className }) => {
  const trackRef = useHorizontalDragScroll<HTMLDivElement>();
  const atEnds = useMemo(() => {
    const el = trackRef.current;
    if (!el) return { left: true, right: true } as const;
    return {
      left: el.scrollLeft <= 2,
      right: Math.abs(el.scrollWidth - el.clientWidth - el.scrollLeft) <= 2
    } as const;
  }, [trackRef.current?.scrollLeft, presets.length]);

  const scrollBy = (dir: 'left' | 'right') => {
    const el = trackRef.current; if (!el) return;
    const step = el.clientWidth * 0.75;
    el.scrollTo({ left: el.scrollLeft + (dir === 'left' ? -step : step), behavior: 'smooth' });
  };

  return (
    <div className={cn('w-full', className)}>
      <div className="flex items-center justify-between mb-3">
        <h3 className="text-xs tracking-[.22em] font-bold text-foreground/80 uppercase">{title}</h3>
        <div className="flex items-center gap-2">
          <Arrow disabled={atEnds.left} onClick={() => scrollBy('left')} />
          <Arrow right disabled={atEnds.right} onClick={() => scrollBy('right')} />
        </div>
      </div>

      <div className="relative">
        <div className="pointer-events-none absolute inset-y-0 left-0 w-8 bg-gradient-to-r from-[hsl(var(--background))] to-transparent rounded-l-xl" />
        <div className="pointer-events-none absolute inset-y-0 right-0 w-8 bg-gradient-to-l from-[hsl(var(--background))] to-transparent rounded-r-xl" />
        <div
          ref={trackRef}
          role="listbox"
          className="flex gap-3 overflow-x-auto no-scrollbar px-6 py-2 rounded-xl border border-foreground/10 bg-plugin-surface/60"
        >
          {presets.map((p) => {
            const active = selectedId === p.id;
            return (
              <button
                key={p.id}
                role="option"
                aria-selected={active}
                onClick={() => onSelect(p)}
                className={cn(
                  'shrink-0 px-5 py-4 rounded-xl text-left transition-all select-none border bg-gradient-to-b',
                  active
                    ? 'border-audio-primary/60 from-[hsl(var(--plugin-panel)/0.8)] to-[hsl(var(--plugin-surface))] shadow-[0_0_0_2px_hsl(var(--audio-primary)/.25),0_10px_30px_rgba(0,0,0,.35)]'
                    : 'border-foreground/12 from-[hsl(var(--plugin-surface))] to-[hsl(var(--plugin-panel))] hover:border-foreground/25 hover:-translate-y-[1px]'
                )}
              >
                <div className="text-[11px] tracking-[.18em] font-semibold text-foreground/90 uppercase">
                  {p.label}
                </div>
                {p.subtitle && (
                  <div className="mt-1 text-[10px] text-foreground/55 uppercase tracking-widest">{p.subtitle}</div>
                )}
              </button>
            );
          })}
        </div>
      </div>

      <div className="mt-2 text-[10px] text-foreground/50">
        Tip: swipe/drag to scroll • mouse-wheel • ←/→ • Enter
      </div>
    </div>
  );
};

const Arrow: React.FC<{ right?: boolean; disabled?: boolean; onClick: () => void }> = ({ right, disabled, onClick }) => (
  <button
    onClick={onClick}
    disabled={disabled}
    className={cn(
      'h-8 w-8 rounded-full grid place-items-center border text-xs',
      disabled ? 'opacity-30 cursor-not-allowed border-foreground/10'
               : 'hover:-translate-y-[1px] transition border-foreground/20 bg-plugin-surface'
    )}
    aria-label={right ? 'Next presets' : 'Previous presets'}
  >
    {right ? '›' : '‹'}
  </button>
);
