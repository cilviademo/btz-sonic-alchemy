import React, { useState, useRef } from 'react';
import { cn } from '@/lib/utils';

interface KnobProps {
  value: number;
  onChange: (value: number) => void;
  min?: number;
  max?: number;
  step?: number;
  label: string;
  unit?: string;
  size?: 'sm' | 'md' | 'lg';
  disabled?: boolean;
}

export const Knob: React.FC<KnobProps> = ({
  value,
  onChange,
  min = 0,
  max = 1,
  step = 0.01,
  label,
  unit = '',
  size = 'md',
  disabled = false,
}) => {
  const [isDragging, setIsDragging] = useState(false);
  const [startY, setStartY] = useState(0);
  const [startValue, setStartValue] = useState(0);
  const knobRef = useRef<HTMLDivElement>(null);

  const normalizedValue = (value - min) / (max - min);
  const rotation = -135 + normalizedValue * 270; // -135° to +135°

  const formatValue = (val: number) => {
    if (max <= 1) return val.toFixed(2);
    if (max <= 10) return val.toFixed(1);
    return Math.round(val).toString();
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
    const deltaY = startY - e.clientY;
    const sensitivity = 0.005;
    const range = max - min;
    const newValue = Math.max(min, Math.min(max, startValue + deltaY * sensitivity * range));
    onChange(newValue);
  };

  const handleMouseUp = () => {
    setIsDragging(false);
    document.removeEventListener('mousemove', handleMouseMove);
    document.removeEventListener('mouseup', handleMouseUp);
  };

  const sizeClasses = {
    sm: 'w-16 h-16',
    md: 'w-20 h-20',
    lg: 'w-24 h-24',
  };

  const textSizes = {
    sm: 'text-xs',
    md: 'text-sm',
    lg: 'text-base',
  };

  return (
    <div className="flex flex-col items-center gap-2 select-none">
      <div
        ref={knobRef}
        className={cn(
          'relative cursor-pointer transition-all duration-200',
          sizeClasses[size],
          disabled && 'opacity-50 cursor-not-allowed'
        )}
        onMouseDown={handleMouseDown}
      >
        {/* Knob Background */}
        <div
          className={cn(
            'absolute inset-0 rounded-full bg-knob-background',
            'shadow-[inset_0_2px_4px_rgba(0,0,0,0.8),0_2px_8px_rgba(0,0,0,0.4)]',
            'border border-plugin-raised',
            isDragging && 'shadow-[var(--glow-active)]'
          )}
        />
        
        {/* Knob Pointer */}
        <div
          className="absolute inset-2 rounded-full bg-gradient-to-br from-plugin-raised to-knob-background border border-plugin-panel"
          style={{ transform: `rotate(${rotation}deg)` }}
        >
          <div className="absolute top-1 left-1/2 w-0.5 h-3 bg-audio-primary rounded-full transform -translate-x-1/2" />
        </div>
        
        {/* Center Dot */}
        <div className="absolute top-1/2 left-1/2 w-2 h-2 bg-knob-accent rounded-full transform -translate-x-1/2 -translate-y-1/2 shadow-[var(--glow-accent)]" />
        
        {/* Value Arc */}
        <svg className="absolute inset-0 w-full h-full" viewBox="0 0 100 100">
          <circle
            cx="50"
            cy="50"
            r="45"
            fill="none"
            stroke="rgba(195, 255, 255, 0.1)"
            strokeWidth="2"
            strokeDasharray="189.6"
            strokeDashoffset="47.4"
            transform="rotate(-135 50 50)"
          />
          <circle
            cx="50"
            cy="50"
            r="45"
            fill="none"
            stroke="var(--audio-primary)"
            strokeWidth="3"
            strokeDasharray="189.6"
            strokeDashoffset={189.6 - normalizedValue * 142.2}
            transform="rotate(-135 50 50)"
            className="transition-all duration-200"
            style={{ filter: 'drop-shadow(0 0 4px var(--audio-primary))' }}
          />
        </svg>
      </div>
      
      {/* Label and Value */}
      <div className="text-center">
        <div className={cn('text-foreground font-medium', textSizes[size])}>{label}</div>
        <div className={cn('text-audio-primary font-mono', textSizes[size])}>
          {formatValue(value)}{unit}
        </div>
      </div>
    </div>
  );
};