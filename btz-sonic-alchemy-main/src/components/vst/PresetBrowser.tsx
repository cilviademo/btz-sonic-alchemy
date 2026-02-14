import React, { useState } from 'react';
import { EnhancedPreset } from './types';
import { cn } from '@/lib/utils';

interface PresetBrowserProps {
  presets: EnhancedPreset[];
  onApplyPreset: (preset: EnhancedPreset) => void;
}

export const PresetBrowser: React.FC<PresetBrowserProps> = ({
  presets,
  onApplyPreset
}) => {
  const [selectedPreset, setSelectedPreset] = useState<string | null>(null);

  const handlePresetClick = (preset: EnhancedPreset) => {
    setSelectedPreset(preset.id);
    onApplyPreset(preset);
  };

  return (
    <div className="flex flex-col items-center gap-4">
      <div className="text-sm font-bold text-foreground tracking-widest uppercase">
        PRESETS
      </div>
      
      <div className="flex gap-3 bg-plugin-surface rounded-2xl p-3 border border-audio-primary/20">
        {presets.map((preset) => {
          const isSelected = selectedPreset === preset.id;
          const isDefault = preset.id === 'default';
          
          return (
            <button
              key={preset.id}
              onClick={() => handlePresetClick(preset)}
              className={cn(
                "px-6 py-3 rounded-xl font-bold text-sm transition-all duration-300 min-w-[100px]",
                isSelected
                  ? "text-background shadow-lg transform scale-105"
                  : "text-foreground/70 hover:text-foreground hover:bg-plugin-raised/50",
                isDefault && "border-2 border-audio-primary/30"
              )}
              style={isSelected ? {
                background: isDefault 
                  ? 'linear-gradient(135deg, hsl(var(--audio-primary)), hsl(var(--audio-secondary)))'
                  : 'hsl(var(--audio-tertiary))',
                boxShadow: isDefault
                  ? '0 0 20px hsl(var(--audio-primary) / 0.6)'
                  : '0 0 15px hsl(var(--audio-tertiary) / 0.5)'
              } : {}}
            >
              {preset.label}
              {isDefault && (
                <div className="text-xs opacity-80 mt-1">
                  0%
                </div>
              )}
            </button>
          );
        })}
      </div>
    </div>
  );
};