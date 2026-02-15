import React from 'react';
import { cn } from '@/lib/utils';

export const ArturiaToggle: React.FC<{
  checked: boolean; onChange: (v:boolean)=>void; label: string;
}> = ({ checked, onChange, label }) => {
  return (
    <button
      onClick={() => onChange(!checked)}
      className={cn(
        'inline-flex items-center gap-2 px-3 py-2 rounded-md border',
        'transition-all duration-200',
        checked ? 'bg-[var(--art-accent)] text-white border-transparent shadow'
                : 'bg-[var(--art-plate)] text-[var(--panel-ink)] border-black/10'
      )}
      aria-pressed={checked}
    >
      <span className={cn(
        'w-2.5 h-2.5 rounded-full',
        checked ? 'bg-white' : 'bg-[var(--art-led)]'
      )}/>
      <span className="text-[11px] tracking-[.18em] font-semibold">{label}</span>
    </button>
  );
};
