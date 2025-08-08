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
  size?: 'sm' | 'md' | 'lg';
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
    lg: 'w-20 h-20'
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
            'relative rounded-full cursor-pointer select-none transition-all duration-200',
            'bg-gradient-to-br from-plugin-raised to-plugin-highlight',
            'shadow-[var(--shadow-depth)] hover:shadow-[var(--glow-active)]',
            'border-2 border-plugin-highlight/20',
            sizeClasses[size],
            disabled ? 'opacity-50 cursor-not-allowed' : 'hover:scale-105 active:scale-95',
            isDragging && 'scale-105 shadow-[var(--glow-active)]'
          )}
          style={{
            background: normalizedValue > 0.7 ? 'var(--gradient-active)' : 'var(--gradient-knob)'
          }}
        >
          {/* Center Dot */}
          <div className="absolute inset-0 flex items-center justify-center">
            <div className="w-2 h-2 rounded-full bg-foreground/80 shadow-sm" />
          </div>

          {/* Value Arc */}
          <svg className="absolute inset-0 w-full h-full -rotate-90" viewBox="0 0 100 100">
            <circle
              cx="50"
              cy="50"
              r="35"
              fill="none"
              stroke="hsl(var(--plugin-surface))"
              strokeWidth="4"
              strokeDasharray={`${270 * 2.199} ${1000}`}
              strokeDashoffset="0"
              opacity="0.3"
            />
            <circle
              cx="50"
              cy="50"
              r="35"
              fill="none"
              stroke={`hsl(var(--audio-primary))`}
              strokeWidth="4"
              strokeDasharray={`${normalizedValue * 270 * 2.199} ${1000}`}
              strokeDashoffset="0"
              className="transition-all duration-200"
              style={{
                filter: normalizedValue > 0.1 ? 'drop-shadow(0 0 4px hsl(var(--audio-primary) / 0.6))' : 'none'
              }}
            />
          </svg>

          {/* Pointer Line */}
          <div
            className="absolute top-2 left-1/2 w-0.5 h-4 bg-foreground/90 origin-bottom transition-all duration-200"
            style={{ transform: `translateX(-50%) rotate(${angle}deg)` }}
          />
        </div>

        {/* Value Display */}
        <div className="absolute -bottom-8 left-1/2 transform -translate-x-1/2 text-xs font-mono text-audio-primary font-medium">
          {formatValue(value)}{unit}
        </div>
      </div>

      {/* Label */}
      <label className="text-xs font-medium text-foreground/70 uppercase tracking-wider">
        {label}
      </label>
    </div>
  );
};