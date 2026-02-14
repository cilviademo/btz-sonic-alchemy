import React from 'react';
import { cn } from '@/lib/utils';
import { ToggleButton } from './ToggleButton';
import { PresetsSelect, PresetOption } from './PresetsSelect';

interface BTZHeaderProps {
  isProcessing: boolean;
  active: boolean;
  onToggleActive: () => void;
  oversampling: boolean;
  onToggleOversampling: () => void;
  onApplyPreset: (preset: PresetOption) => void;
}

export const BTZHeader: React.FC<BTZHeaderProps> = ({
  isProcessing,
  active,
  onToggleActive,
  oversampling,
  onToggleOversampling,
  onApplyPreset,
}) => {
  return (
    <header className="bg-gradient-to-r from-plugin-panel to-plugin-raised border-b border-plugin-raised p-4">
      <div className="flex items-center justify-between flex-wrap gap-4">
        <div className="flex items-center gap-4">
          <h1 className="text-2xl font-bold text-audio-primary font-['Inter']">
            BTZ — The Box Tone Zone Enhancer
          </h1>
          <p className="text-sm text-foreground/70 hidden md:block">
            Precision drum tone sculptor: punch, weight, character.
          </p>
        </div>

        <nav className="flex items-center gap-4">
          <div className="flex items-center gap-2">
            <div
              aria-label="AI processing status"
              className={cn(
                'w-2 h-2 rounded-full transition-all duration-200',
                isProcessing
                  ? 'bg-audio-success shadow-[0_0_8px_var(--audio-success)]'
                  : 'bg-foreground/30'
              )}
            />
            <span className="text-xs text-foreground/70">AI</span>
          </div>

          <ToggleButton value={active} onChange={onToggleActive} label={active ? 'ACTIVE' : 'BYPASSED'} />
          <ToggleButton value={oversampling} onChange={onToggleOversampling} label={oversampling ? 'HQ x8' : 'HQ OFF'} />

          <PresetsSelect onApply={onApplyPreset} />
        </nav>
      </div>
    </header>
  );
};
