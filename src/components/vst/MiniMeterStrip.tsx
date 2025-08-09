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
          <div className="absolute inset-y-0 left-0 rounded-full"
               style={{ width: `${lufsPct}%`, backgroundImage: 'linear-gradient(90deg, rgba(0,212,255,0.9), rgba(138,43,226,0.9)), repeating-linear-gradient(90deg, transparent 0, transparent 10px, rgba(0,0,0,.35) 10px, rgba(0,0,0,.35) 12px)', backgroundBlendMode: 'overlay', boxShadow: '0 0 10px rgba(138,43,226,0.45)'}} />
        </div>
        <div className="text-[10px] font-mono text-foreground/70">{lufs.toFixed(1)}</div>
      </div>
      <div className="flex items-center gap-3 mt-1.5">
        <div className="text-[10px] uppercase tracking-[.18em] text-foreground/70">PEAK</div>
        <div className="relative h-[12px] flex-1 rounded-full bg-foreground/10 overflow-hidden">
          <div className="absolute inset-y-0 left-0 rounded-full"
               style={{ width: `${peakPct}%`, backgroundImage: 'linear-gradient(90deg, rgba(46,204,113,0.95), rgba(255,165,0,0.95), rgba(231,76,60,0.95)) , repeating-linear-gradient(90deg, transparent 0, transparent 10px, rgba(0,0,0,.35) 10px, rgba(0,0,0,.35) 12px)', backgroundBlendMode: 'overlay', boxShadow: '0 0 10px rgba(231,76,60,0.4)'}} />
        </div>
        <div className="text-[10px] font-mono text-foreground/70">{truePeakDb.toFixed(1)} dB</div>
      </div>
    </div>
  );
};
