import React from 'react';
import { VUMeter } from './VUMeter';

interface LevelsPanelProps {
  inputLevel: number;
  outputLevel: number;
}

export const LevelsPanel: React.FC<LevelsPanelProps> = ({ inputLevel, outputLevel }) => {
  return (
    <aside className="bg-plugin-panel rounded-xl p-4 border border-plugin-raised">
      <h2 className="text-sm font-semibold text-foreground/80 mb-4 text-center">LEVELS</h2>
      <div className="flex justify-center gap-4">
        <VUMeter level={inputLevel} label="IN" size="md" />
        <VUMeter level={outputLevel} label="OUT" size="md" />
      </div>
    </aside>
  );
};
