import React, { useEffect, useState } from 'react';
import { cn } from '@/lib/utils';

interface VUMeterProps {
  level: number; // 0-1
  orientation?: 'horizontal' | 'vertical';
  size?: 'sm' | 'md' | 'lg';
  label?: string;
}

export const VUMeter: React.FC<VUMeterProps> = ({
  level,
  orientation = 'vertical',
  size = 'md',
  label,
}) => {
  const [peakLevel, setPeakLevel] = useState(level);
  const [peakHold, setPeakHold] = useState(0);

  useEffect(() => {
    if (level > peakLevel) {
      setPeakLevel(level);
    } else {
      setPeakLevel(prev => Math.max(level, prev * 0.98));
    }

    if (level > peakHold) {
      setPeakHold(level);
      setTimeout(() => setPeakHold(prev => prev * 0.95), 50);
    }
  }, [level, peakLevel, peakHold]);

  const getColor = (position: number) => {
    if (position > 0.9) return 'bg-audio-warning';
    if (position > 0.75) return 'bg-yellow-500';
    if (position > 0.5) return 'bg-audio-success';
    return 'bg-audio-primary';
  };

  const sizeClasses = {
    sm: orientation === 'vertical' ? 'w-3 h-20' : 'w-20 h-3',
    md: orientation === 'vertical' ? 'w-4 h-32' : 'w-32 h-4',
    lg: orientation === 'vertical' ? 'w-5 h-40' : 'w-40 h-5',
  };

  const segments = Array.from({ length: 20 }, (_, i) => i / 19);

  return (
    <div className="flex flex-col items-center gap-2">
      {label && <div className="text-xs text-foreground/70 font-medium">{label}</div>}
      
      <div
        className={cn(
          'relative bg-plugin-surface rounded-md border border-plugin-raised overflow-hidden',
          sizeClasses[size]
        )}
      >
        {/* Background segments */}
        <div className={cn(
          'absolute inset-1 flex gap-0.5',
          orientation === 'vertical' ? 'flex-col-reverse' : 'flex-row'
        )}>
          {segments.map((position, index) => (
            <div
              key={index}
              className={cn(
                'flex-1 rounded-sm transition-all duration-75',
                level >= position
                  ? getColor(position)
                  : 'bg-plugin-panel/30'
              )}
              style={{
                opacity: level >= position ? 1 : 0.3,
                boxShadow: level >= position && position > 0.9 
                  ? '0 0 4px var(--audio-warning)' 
                  : 'none'
              }}
            />
          ))}
        </div>
        
        {/* Peak indicator */}
        {peakHold > 0.1 && (
          <div
            className={cn(
              'absolute w-full h-0.5 bg-audio-primary-glow rounded',
              orientation === 'vertical' ? 'left-0' : 'top-0'
            )}
            style={{
              [orientation === 'vertical' ? 'bottom' : 'left']: `${peakHold * 100}%`,
              boxShadow: '0 0 6px var(--audio-primary-glow)'
            }}
          />
        )}
      </div>
      
      {/* dB scale */}
      <div className="text-xs text-foreground/50 font-mono">
        {Math.round((level * 60) - 60)}dB
      </div>
    </div>
  );
};