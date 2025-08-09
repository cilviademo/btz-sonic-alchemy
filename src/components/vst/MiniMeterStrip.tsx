import React from 'react';

export const MiniMeterStrip: React.FC<{ lufs?: number; truePeakDb?: number; peakNorm?: number }>
= ({ lufs = -14.0, truePeakDb = -0.7, peakNorm = 0 }) => {
  const peakPct = Math.max(0, Math.min(1, peakNorm)) * 100;
  const lufsPct = (() => {
    const clamped = Math.max(-60, Math.min(0, lufs)); // map -60..0 LUFS to 0..100%
    return ((clamped + 60) / 60) * 100;
  })();

  return (
    <div className="w-full">
      <div className="flex items-center gap-3">
        <div className="text-[10px] uppercase tracking-[.18em] text-foreground/70">LUFS</div>
        <div className="relative h-[12px] flex-1 rounded-full bg-foreground/10 overflow-hidden">
          <div className="absolute inset-y-0 left-0 rounded-full bg-audio-primary" style={{ width: `${lufsPct}%` }} />
        </div>
        <div className="text-[10px] font-mono text-foreground/70">{lufs.toFixed(1)}</div>
      </div>
      <div className="flex items-center gap-3 mt-1.5">
        <div className="text-[10px] uppercase tracking-[.18em] text-foreground/70">PEAK</div>
        <div className="relative h-[12px] flex-1 rounded-full bg-foreground/10 overflow-hidden">
          <div className="absolute inset-y-0 left-0 rounded-full bg-audio-success" style={{ width: `${peakPct}%` }} />
        </div>
        <div className="text-[10px] font-mono text-foreground/70">{truePeakDb.toFixed(1)} dB</div>
      </div>
    </div>
  );
};
