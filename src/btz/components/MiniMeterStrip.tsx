import React from 'react';

export const MiniMeterStrip: React.FC<{ lufs?: number; truePeakDb?: number; peakNorm?: number }>
= ({ lufs = -14.0, truePeakDb = -0.7, peakNorm = 0 }) => {
  const peakPct = Math.max(0, Math.min(1, peakNorm)) * 100;
  const lufsPct = (() => {
    const clamped = Math.max(-60, Math.min(0, lufs));
    return ((clamped + 60) / 60) * 100;
  })();

  return (
    <div className="grid grid-cols-2 gap-4 text-xs">
      <div>
        <div className="mb-1 uppercase tracking-widest opacity-70">LUFS</div>
        <div className="h-2 w-full bg-white/10">
          <div className="h-2" style={{ width: `${lufsPct}%`, background: 'linear-gradient(90deg,#39ff88,#2fd3ff)' }} />
        </div>
        <div className="mt-1 opacity-80">{lufs.toFixed(1)}</div>
      </div>
      <div>
        <div className="mb-1 uppercase tracking-widest opacity-70">Peak</div>
        <div className="h-2 w-full bg-white/10">
          <div className="h-2" style={{ width: `${peakPct}%`, background: 'linear-gradient(90deg,#ff8a00,#ff2fb9)' }} />
        </div>
        <div className="mt-1 opacity-80">{truePeakDb.toFixed(1)} dBTP</div>
      </div>
    </div>
  );
};