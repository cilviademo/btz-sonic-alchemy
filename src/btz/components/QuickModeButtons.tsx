import React from 'react';
import { cn } from '@/lib/utils';
import type { BTZPluginState } from '@/btz/types';

interface ModeButton {
  id: string;
  label: string;
  icon?: string;
  patch: Partial<BTZPluginState>;
  description: string;
}

const QUICK_MODES: ModeButton[] = [
  {
    id: 'safe',
    label: 'Safe TP',
    icon: '🛡️',
    patch: { sparkEnabled: true, sparkCeiling: -1.0, sparkLUFS: -14 },
    description: 'Sets true-peak ceiling to -1.0 dBTP for safety'
  },
  {
    id: 'streaming',
    label: 'Streaming',
    icon: '📱',
    patch: { sparkEnabled: true, sparkCeiling: -0.3, sparkLUFS: -14, sparkMode: 'soft' },
    description: 'Optimized for streaming platforms (-14 LUFS target)'
  },
  {
    id: 'club',
    label: 'Club',
    icon: '🔊',
    patch: { sparkEnabled: true, sparkCeiling: -0.3, sparkLUFS: -9, sparkMode: 'hard' },
    description: 'Louder target for club/DJ use (-9 LUFS)'
  }
];

interface Props {
  onApply: (patch: Partial<BTZPluginState>) => void;
  className?: string;
}

export const QuickModeButtons: React.FC<Props> = ({ onApply, className }) => {
  return (
    <div className={cn("flex flex-col gap-2", className)}>
      <div className="text-[10px] uppercase tracking-widest opacity-50">Quick Modes</div>
      <div className="grid grid-cols-3 gap-2">
        {QUICK_MODES.map(mode => (
          <button
            key={mode.id}
            type="button"
            onClick={() => onApply(mode.patch)}
            className="group relative rounded bg-white/5 px-2 py-2 text-center text-xs hover:bg-white/10 focus:outline-none focus:ring-1 focus:ring-white/30"
            title={mode.description}
          >
            {mode.icon && (
              <div className="mb-0.5 text-sm opacity-60 group-hover:opacity-100">
                {mode.icon}
              </div>
            )}
            <div className="font-medium uppercase tracking-wide">{mode.label}</div>
          </button>
        ))}
      </div>
    </div>
  );
};
