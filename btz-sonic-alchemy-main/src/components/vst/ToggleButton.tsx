import React from 'react';
import { cn } from '@/lib/utils';

interface ToggleButtonProps {
  value: boolean;
  onChange: (value: boolean) => void;
  label: string;
  disabled?: boolean;
  className?: string;
}

export const ToggleButton: React.FC<ToggleButtonProps> = ({
  value,
  onChange,
  label,
  disabled = false,
  className,
}) => {
  return (
    <button
      onClick={() => !disabled && onChange(!value)}
      disabled={disabled}
      className={cn(
        'relative px-6 py-3 rounded-lg font-medium text-sm transition-all duration-200',
        'border-2 select-none',
        value
          ? 'bg-gradient-to-br from-audio-primary to-audio-primary-glow border-audio-primary text-background shadow-[var(--glow-active)]'
          : 'bg-plugin-panel border-plugin-raised text-foreground hover:border-audio-primary/50',
        disabled && 'opacity-50 cursor-not-allowed',
        !disabled && 'cursor-pointer hover:scale-105 active:scale-95',
        className
      )}
    >
      <div className="flex items-center gap-2">
        <div
          className={cn(
            'w-2 h-2 rounded-full transition-all duration-200',
            value ? 'bg-background shadow-[0_0_8px_rgba(255,255,255,0.6)]' : 'bg-foreground/30'
          )}
        />
        {label}
      </div>
    </button>
  );
};