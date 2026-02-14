import React from 'react';

interface CompliancePanelProps {
  lufsIntegrated: number; // e.g., -12.3
  truePeak: number; // dBTP, e.g., -1.0
}

const Gauge: React.FC<{ label: string; value: number; target: number; unit: string; goodIf: 'higher' | 'lower' }>
= ({ label, value, target, unit, goodIf }) => {
  const pct = goodIf === 'higher'
    ? Math.min(100, Math.max(0, ((value - (target - 10)) / 10) * 100))
    : Math.min(100, Math.max(0, (((target + 4) - value) / 4) * 100));
  const ok = goodIf === 'higher' ? value >= target : value <= target;
  return (
    <div className="space-y-2">
      <div className="flex items-center justify-between text-xs">
        <span className="text-foreground/70">{label}</span>
        <span className={ok ? 'text-audio-success' : 'text-audio-warning'}>
          {value.toFixed(1)} {unit}
        </span>
      </div>
      <div className="h-2 bg-plugin-surface rounded-full overflow-hidden">
        <div
          className={`h-full ${ok ? 'bg-audio-success' : 'bg-audio-warning'}`}
          style={{ width: `${pct}%` }}
        />
      </div>
      <div className="text-[10px] text-foreground/50">Target: {target} {unit}</div>
    </div>
  );
};

export const CompliancePanel: React.FC<CompliancePanelProps> = ({ lufsIntegrated, truePeak }) => {
  const compliant = lufsIntegrated >= -8 && truePeak <= -1;
  return (
    <aside className="bg-plugin-panel rounded-xl p-4 border border-plugin-raised">
      <h2 className="text-sm font-semibold text-foreground/80 mb-4 text-center">STREAMING COMPLIANCE</h2>
      <div className="space-y-4">
        <Gauge label="Integrated Loudness" value={lufsIntegrated} target={-8} unit="LUFS" goodIf="higher" />
        <Gauge label="True Peak" value={truePeak} target={-1} unit="dBTP" goodIf="lower" />
        <div className={`text-xs text-center ${compliant ? 'text-audio-success' : 'text-audio-warning'}`}>
          {compliant ? 'Compliant with -8 LUFS / -1 dBTP' : 'Adjust Drive/Mix to reach target'}
        </div>
      </div>
    </aside>
  );
};
