import React, { useState, useRef, useEffect } from 'react';
import { cn } from '@/lib/utils';

interface ModernKnobProps {
  value: number;
  onChange: (value: number) => void;
  min: number;
  max: number;
  step?: number;
  label: string;
  unit?: string;
  size?: 'sm' | 'md' | 'lg' | 'xl';
  disabled?: boolean;
  spectrum?: number[]; // For spectrum visualization around knob
}

export const ModernKnob: React.FC<ModernKnobProps> = ({
  value,
  onChange,
  min,
  max,
  step = 0.01,
  label,
  unit = '',
  size = 'md',
  disabled = false,
  spectrum = [],
}) => {
  const [isDragging, setIsDragging] = useState(false);
  const [startY, setStartY] = useState(0);
  const [startValue, setStartValue] = useState(0);
  const knobRef = useRef<HTMLDivElement>(null);

  const normalizedValue = (value - min) / (max - min);
  const angle = normalizedValue * 270 - 135; // -135 to +135 degrees

  const sizeClasses = {
    sm: 'w-12 h-12',
    md: 'w-16 h-16', 
    lg: 'w-20 h-20',
    xl: 'w-28 h-28'
  };

  const handleMouseDown = (e: React.MouseEvent) => {
    if (disabled) return;
    setIsDragging(true);
    setStartY(e.clientY);
    setStartValue(value);
    document.addEventListener('mousemove', handleMouseMove);
    document.addEventListener('mouseup', handleMouseUp);
  };

  const handleMouseMove = (e: MouseEvent) => {
    if (!isDragging) return;
    const sensitivity = 0.005;
    const deltaY = startY - e.clientY;
    const deltaValue = deltaY * (max - min) * sensitivity;
    const newValue = Math.max(min, Math.min(max, startValue + deltaValue));
    onChange(Math.round(newValue / step) * step);
  };

  const handleMouseUp = () => {
    setIsDragging(false);
    document.removeEventListener('mousemove', handleMouseMove);
    document.removeEventListener('mouseup', handleMouseUp);
  };

  useEffect(() => {
    return () => {
      document.removeEventListener('mousemove', handleMouseMove);
      document.removeEventListener('mouseup', handleMouseUp);
    };
  }, []);

  const formatValue = (val: number): string => {
    if (max <= 1) return (val * 100).toFixed(0);
    if (max <= 10) return val.toFixed(2);
    return val.toFixed(1);
  };

  return (
    <div className="flex flex-col items-center gap-2 group">
      {/* Knob Container */}
      <div className="relative">
        {/* Spectrum Ring (if provided) */}
        {spectrum.length > 0 && (
          <div className="absolute inset-0 rounded-full opacity-30">
            <svg className="w-full h-full" viewBox="0 0 100 100">
              {spectrum.slice(0, 32).map((level, i) => {
                const angle = (i / 32) * 360;
                const height = Math.max(2, level * 8);
                return (
                  <rect
                    key={i}
                    x="48"
                    y="10"
                    width="4"
                    height={height}
                    fill={`hsl(${210 + level * 70}, 70%, ${50 + level * 30}%)`}
                    transform={`rotate(${angle}, 50, 50)`}
                    opacity={0.6}
                  />
                );
              })}
            </svg>
          </div>
        )}

        {/* Main Knob */}
        <div
          ref={knobRef}
          onMouseDown={handleMouseDown}
          className={cn(
            'relative rounded-full cursor-pointer select-none transition-all duration-300',
            'shadow-[var(--shadow-knob)] hover:shadow-[var(--glow-accent)]',
            'border border-knob-shadow',
            sizeClasses[size],
            disabled ? 'opacity-50 cursor-not-allowed' : 'hover:scale-105 active:scale-95',
            isDragging && 'scale-105 shadow-[var(--shadow-knob-pressed)]'
          )}
          style={{
            background: 'var(--gradient-knob)',
            boxShadow: isDragging ? 'var(--shadow-knob-pressed)' : 'var(--shadow-knob)'
          }}
        >
          {/* Center Dot */}
          <div className="absolute inset-0 flex items-center justify-center">
            <div className="w-1 h-1 rounded-full bg-background/90" />
          </div>

          {/* Pointer Line */}
          <div
            className="absolute top-3 left-1/2 w-0.5 origin-bottom transition-all duration-300 rounded-full"
            style={{ 
              transform: `translateX(-50%) rotate(${angle}deg)`,
              height: size === 'xl' ? '24px' : size === 'lg' ? '16px' : '12px',
              background: 'linear-gradient(to bottom, hsl(var(--background)), hsl(var(--background)/0.8))'
            }}
          />
        </div>

        {/* Value Display */}
        <div className="absolute -bottom-8 left-1/2 transform -translate-x-1/2 text-xs font-mono text-foreground/90 font-medium">
          {formatValue(value)}{unit}
        </div>
      </div>

      {/* Label */}
      <label className="text-xs font-bold text-foreground tracking-widest uppercase mt-2">
        {label}
      </label>
    </div>
  );
};