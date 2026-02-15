import React from 'react';

interface AIAnalysisPanelProps {
  punch: number;
  warmth: number;
  boom: number;
}

const Bar: React.FC<{ label: string; value: number; colorClass: string }> = ({ label, value, colorClass }) => (
  <div className="space-y-2">
    <div className="flex justify-between text-xs">
      <span className="text-foreground/70">{label}</span>
      <span className={colorClass}>{Math.round(value)}%</span>
    </div>
    <div className="h-1 bg-plugin-surface rounded-full overflow-hidden">
      <div className={`h-full rounded-full transition-all duration-500 ${colorClass}`} style={{ width: `${value}%` }} />
    </div>
  </div>
);

export const AIAnalysisPanel: React.FC<AIAnalysisPanelProps> = ({ punch, warmth, boom }) => {
  // Derive pseudo analysis from controls
  const transients = 50 + punch * 50;
  const lowEnd = 40 + boom * 50;
  const richness = 30 + warmth * 50;
  const loudness = 60 + (punch + warmth) * 20;

  return (
    <aside className="bg-plugin-panel rounded-xl p-4 border border-plugin-raised">
      <h2 className="text-sm font-semibold text-foreground/80 mb-4 text-center">AI ANALYSIS</h2>
      <div className="space-y-4">
        <Bar label="Transients" value={transients} colorClass="bg-audio-primary text-audio-primary" />
        <Bar label="Low End" value={lowEnd} colorClass="bg-audio-success text-audio-success" />
        <Bar label="Richness" value={richness} colorClass="bg-audio-secondary text-audio-secondary" />
        <Bar label="Loudness" value={loudness} colorClass="bg-audio-warning text-audio-warning" />
      </div>
    </aside>
  );
};
