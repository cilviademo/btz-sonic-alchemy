import React, { useEffect, useState } from 'react';
import { cn } from '@/lib/utils';

interface ModernVUMeterProps {
  level: number; // 0-1 
  peak?: number;
  label: string;
  orientation?: 'horizontal' | 'vertical';
  size?: 'sm' | 'md' | 'lg';
  showPeakHold?: boolean;
  showNumeric?: boolean;
  style?: 'classic' | 'modern' | 'minimal';
}

export const ModernVUMeter: React.FC<ModernVUMeterProps> = ({
  level,
  peak = 0,
  label,
  orientation = 'vertical',
  size = 'md',
  showPeakHold = true,
  showNumeric = true,
  style = 'modern'
}) => {
  const [peakHold, setPeakHold] = useState(0);
  const [peakTimer, setPeakTimer] = useState<NodeJS.Timeout | null>(null);

  // Update peak hold
  useEffect(() => {
    if (level > peakHold) {
      setPeakHold(level);
      if (peakTimer) clearTimeout(peakTimer);
      const timer = setTimeout(() => {
        setPeakHold(prev => Math.max(0, prev - 0.02));
      }, 1500);
      setPeakTimer(timer);
    }
    return () => {
      if (peakTimer) clearTimeout(peakTimer);
    };
  }, [level, peakHold]);

  const sizeClasses = {
    sm: orientation === 'vertical' ? 'w-4 h-32' : 'w-32 h-4',
    md: orientation === 'vertical' ? 'w-6 h-48' : 'w-48 h-6',
    lg: orientation === 'vertical' ? 'w-8 h-64' : 'w-64 h-8'
  };

  const getSegmentColor = (position: number, currentLevel: number): string => {
    if (position > currentLevel) return 'hsl(var(--plugin-raised))';
    
    // Color zones
    if (position < 0.6) return 'hsl(142, 76%, 56%)'; // Green
    if (position < 0.8) return 'hsl(38, 92%, 65%)'; // Yellow  
    if (position < 0.95) return 'hsl(25, 95%, 58%)'; // Orange
    return 'hsl(0, 84%, 68%)'; // Red
  };

  const segments = Array.from({ length: 32 }, (_, i) => i / 31);
  const levelInDb = level > 0 ? 20 * Math.log10(level) : -Infinity;
  const peakInDb = peak > 0 ? 20 * Math.log10(peak) : -Infinity;

  return (
    <div className="flex flex-col items-center gap-2">
      {/* Meter Body */}
      <div className={cn(
        'relative bg-plugin-surface rounded-full border border-plugin-raised overflow-hidden',
        sizeClasses[size]
      )}>
        {/* Background segments */}
        <div className={cn(
          'absolute inset-1 flex gap-px',
          orientation === 'vertical' ? 'flex-col-reverse' : 'flex-row'
        )}>
          {segments.map((position, i) => (
            <div
              key={i}
              className={cn(
                'transition-all duration-100',
                orientation === 'vertical' ? 'h-full' : 'w-full'
              )}
              style={{
                backgroundColor: getSegmentColor(position, level),
                opacity: position <= level ? 1 : 0.2,
                boxShadow: position <= level && position > 0.8 ? 
                  `0 0 4px ${getSegmentColor(position, level)}` : 'none'
              }}
            />
          ))}
        </div>

        {/* Peak hold indicator */}
        {showPeakHold && peakHold > 0.1 && (
          <div
            className={cn(
              'absolute bg-audio-accent shadow-[0_0_8px_hsl(var(--audio-accent))]',
              orientation === 'vertical' ? 'left-0 right-0 h-0.5' : 'top-0 bottom-0 w-0.5'
            )}
            style={{
              [orientation === 'vertical' ? 'bottom' : 'left']: 
                `${(1 - peakHold) * 100}%`
            }}
          />
        )}

        {/* True peak indicator */}
        {peak > 0.95 && (
          <div className="absolute top-0 left-0 w-full h-1 bg-audio-danger animate-pulse" />
        )}
      </div>

      {/* Labels and values */}
      <div className="text-center">
        <div className="text-xs font-medium text-foreground/70 uppercase tracking-wider">
          {label}
        </div>
        {showNumeric && (
          <div className="text-xs font-mono text-audio-primary font-medium mt-1">
            <div>{levelInDb.toFixed(1)} dB</div>
            {peak > 0 && (
              <div className="text-audio-accent text-[10px]">
                pk: {peakInDb.toFixed(1)}
              </div>
            )}
          </div>
        )}
      </div>

      {/* dB Scale */}
      {size !== 'sm' && orientation === 'vertical' && (
        <div className="absolute right-full pr-2 top-0 h-full flex flex-col justify-between text-[9px] text-foreground/40 font-mono">
          <span>0</span>
          <span>-6</span>
          <span>-12</span>
          <span>-24</span>
          <span>-∞</span>
        </div>
      )}
    </div>
  );
};