import React from 'react';
import { cn } from '@/lib/utils';

interface Props {
  lufs?: number;
  truePeakDb?: number;
  peakNorm?: number;
  gainReduction?: number;  // 0..1 (1 = no reduction)
  stereoCorrelation?: number;  // -1 to 1
  targetLUFS?: number;  // for shading target zone
}

export const EnhancedMeterStrip: React.FC<Props> = ({
  lufs = -14.0,
  truePeakDb = -0.7,
  peakNorm = 0,
  gainReduction = 1,
  stereoCorrelation = 1,
  targetLUFS = -14
}) => {
  const peakPct = Math.max(0, Math.min(1, peakNorm)) * 100;
  const lufsPct = (() => {
    const clamped = Math.max(-60, Math.min(0, lufs));
    return ((clamped + 60) / 60) * 100;
  })();

  // Color zones for peak meter
  const peakColor = truePeakDb > -0.1 ? '#ff0000' : truePeakDb > -1.0 ? '#ffaa00' : '#39ff88';
  const peakGradient = `linear-gradient(90deg, #39ff88, ${peakColor})`;

  // Target zone for LUFS (±3dB around target)
  const targetMinPct = Math.max(0, ((targetLUFS - 3 + 60) / 60) * 100);
  const targetMaxPct = Math.min(100, ((targetLUFS + 3 + 60) / 60) * 100);

  // GR meter (inverted - more reduction = more bar)
  const grPct = (1 - gainReduction) * 100;

  // Stereo correlation: -1 (mono) to 1 (wide stereo)
  const corrPct = ((stereoCorrelation + 1) / 2) * 100;

  return (
    <div className="grid grid-cols-2 gap-4 text-xs">
      {/* LUFS with target zone */}
      <div>
        <div className="mb-1 flex items-center justify-between uppercase tracking-widest opacity-70">
          <span>LUFS</span>
          <span className="text-[9px] opacity-50">Target {targetLUFS}</span>
        </div>
        <div className="relative h-2 w-full bg-white/10">
          {/* Target zone shading */}
          <div
            className="absolute h-2 bg-white/5"
            style={{
              left: `${targetMinPct}%`,
              width: `${targetMaxPct - targetMinPct}%`
            }}
          />
          {/* Actual level */}
          <div
            className="h-2 transition-all duration-100"
            style={{
              width: `${lufsPct}%`,
              background: 'linear-gradient(90deg,#39ff88,#2fd3ff)'
            }}
          />
        </div>
        <div className="mt-1 flex justify-between">
          <span className={cn("opacity-80", lufs < targetLUFS - 3 && "text-yellow-400")}>
            {lufs.toFixed(1)}
          </span>
          <span className="text-[9px] opacity-40">-60</span>
        </div>
      </div>

      {/* Peak with safety markers */}
      <div>
        <div className="mb-1 flex items-center justify-between uppercase tracking-widest opacity-70">
          <span>Peak</span>
          <span className="text-[9px] opacity-50">-1.0 dBTP</span>
        </div>
        <div className="relative h-2 w-full bg-white/10">
          {/* Safety marker at -1.0 dBTP */}
          <div
            className="absolute h-2 w-0.5 bg-yellow-400/40"
            style={{ left: '95%' }}
            title="-1.0 dBTP safety"
          />
          {/* Actual peak */}
          <div
            className="h-2 transition-all duration-100"
            style={{ width: `${peakPct}%`, background: peakGradient }}
          />
        </div>
        <div className="mt-1 flex justify-between">
          <span className={cn("opacity-80", truePeakDb > -0.1 && "text-red-400")}>
            {truePeakDb.toFixed(1)} dBTP
          </span>
          <span className="text-[9px] opacity-40">-20</span>
        </div>
      </div>

      {/* Gain Reduction */}
      <div>
        <div className="mb-1 uppercase tracking-widest opacity-70">GR</div>
        <div className="h-2 w-full bg-white/10">
          <div
            className="h-2 transition-all duration-100"
            style={{
              width: `${grPct}%`,
              background: 'linear-gradient(90deg,#ff8a00,#ff2fb9)'
            }}
          />
        </div>
        <div className="mt-1 opacity-80">
          {gainReduction < 1 ? `-${((1 - gainReduction) * 12).toFixed(1)} dB` : '0.0 dB'}
        </div>
      </div>

      {/* Stereo Correlation */}
      <div>
        <div className="mb-1 uppercase tracking-widest opacity-70">Stereo</div>
        <div className="relative h-2 w-full bg-white/10">
          {/* Center marker (mono) */}
          <div
            className="absolute h-2 w-0.5 bg-white/20"
            style={{ left: '50%' }}
            title="Mono"
          />
          {/* Correlation indicator */}
          <div
            className="absolute h-2 w-1 transition-all duration-100"
            style={{
              left: `${corrPct - 2}%`,
              background: stereoCorrelation < 0.3 ? '#ff8a00' : '#39ff88'
            }}
          />
        </div>
        <div className="mt-1 opacity-80">
          {stereoCorrelation > 0 ? '+' : ''}{(stereoCorrelation * 100).toFixed(0)}%
        </div>
      </div>
    </div>
  );
};
