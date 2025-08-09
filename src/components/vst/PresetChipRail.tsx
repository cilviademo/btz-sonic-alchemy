import React, { useRef, useEffect } from 'react';
import { EnhancedPreset, BTZPluginState } from './types';

export const PresetChipRail: React.FC<{
  presets: EnhancedPreset[];
  currentState: BTZPluginState;
  onClick: (p: EnhancedPreset) => void;
}> = ({ presets, onClick }) => {
  const railRef = useRef<HTMLDivElement>(null);

  // arrow keys + H/V scroll to nudge
  useEffect(() => {
    const el = railRef.current;
    if (!el) return;
    const onWheel = (e: WheelEvent) => {
      if (Math.abs(e.deltaY) > Math.abs(e.deltaX)) {
        el.scrollBy({ left: e.deltaY, behavior: 'smooth' });
        e.preventDefault();
      }
    };
    el.addEventListener('wheel', onWheel, { passive: false });
    return () => el.removeEventListener('wheel', onWheel);
  }, []);

  return (
    <div className="btz-presets" style={{ padding:'10px 12px' }}>
      <button className="arrow" onClick={() => railRef.current?.scrollBy({ left:-240, behavior:'smooth' })}>‹</button>
      <div className="rail" ref={railRef}>
        {presets.map(p => (
          <button key={p.id} className="chip" onClick={() => onClick(p)}>
            {p.label}
          </button>
        ))}
      </div>
      <button className="arrow" onClick={() => railRef.current?.scrollBy({ left:240, behavior:'smooth' })}>›</button>
    </div>
  );
};
