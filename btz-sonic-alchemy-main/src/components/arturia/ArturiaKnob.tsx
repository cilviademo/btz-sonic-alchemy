import React from 'react';
import { ThermalKnob } from '@/components/ThermalKnob';

export const ArturiaKnob: React.FC<React.ComponentProps<typeof ThermalKnob>> = (props) => {
  const { label } = props;
  return (
    <div className="px-3 py-4 rounded-lg border border-black/10 bg-[var(--art-plate)]">
      <div className="mb-2 text-[10px] tracking-[.22em] text-[var(--panel-ink)]/70 font-semibold text-center">{label}</div>
      <ThermalKnob {...props} />
      <div className="mt-2 mx-auto h-px w-16 bg-black/10" />
    </div>
  );
};
