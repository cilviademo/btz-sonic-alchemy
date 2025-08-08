import React, { useState, useRef, useEffect } from 'react';
import { cn } from '@/lib/utils';

interface ModernKnobWithSpectrumProps {
  value: number;
  onChange: (value: number) => void;
  min: number;
  max: number;
  step?: number;
  label: string;
  unit?: string;
  size?: 'sm' | 'md' | 'lg' | 'xl';
  disabled?: boolean;
  spectrumData: Float32Array;
  color?: string;
}

export const ModernKnobWithSpectrum: React.FC<ModernKnobWithSpectrumProps> = ({
  value,
  onChange,
  min,
  max,
  step = 0.01,
  label,
  unit = '',
  size = 'md',
  disabled = false,
  spectrumData,
  color = 'hsl(var(--audio-primary))',
}) => {
  const [isDragging, setIsDragging] = useState(false);
  const [startY, setStartY] = useState(0);
  const [startValue, setStartValue] = useState(0);
  const knobRef = useRef<HTMLDivElement>(null);

  const normalizedValue = (value - min) / (max - min);
  const angle = normalizedValue * 270 - 135; // -135 to +135 degrees

  const sizeClasses = {
    sm: 'w-16 h-16',
    md: 'w-20 h-20', 
    lg: 'w-24 h-24',
    xl: 'w-32 h-32'
  };

  const sizeSpectrum = {
    sm: { rings: 8, radius: 30 },
    md: { rings: 12, radius: 38 },
    lg: { rings: 16, radius: 46 },
    xl: { rings: 20, radius: 60 }
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

  const { rings, radius } = sizeSpectrum[size];
  
  return (
    <div className="flex flex-col items-center gap-3 group">
      {/* Knob with Spectrum */}
      <div className="relative">
        {/* Spectrum Ring Visualization */}
        <div className="absolute inset-0 rounded-full" style={{ transform: 'scale(1.2)' }}>
          <svg className="w-full h-full" viewBox="0 0 120 120">
            <defs>
              <radialGradient id={`specGrad-${label}`} cx="50%" cy="50%" r="50%">
                <stop offset="0%" stopColor={color} stopOpacity="0.1"/>
                <stop offset="70%" stopColor={color} stopOpacity="0.6"/>
                <stop offset="100%" stopColor={color} stopOpacity="0.9"/>
              </radialGradient>
            </defs>
            {Array.from(spectrumData.slice(0, rings)).map((level, i) => {
              const angle = (i / rings) * 360;
              const intensity = Math.min(1, level * 2);
              const height = 4 + intensity * 12;
              const opacity = 0.3 + intensity * 0.7;
              
              return (
                <rect
                  key={i}
                  x="58"
                  y="15"
                  width="4"
                  height={height}
                  fill={color}
                  opacity={opacity}
                  rx="2"
                  transform={`rotate(${angle}, 60, 60)`}
                  style={{
                    filter: `drop-shadow(0 0 ${2 + intensity * 4}px ${color})`,
                    animation: intensity > 0.5 ? 'pulse 0.3s ease-in-out' : 'none'
                  }}
                />
              );
            })}
          </svg>
        </div>

        {/* Main Knob - Hardware-style metal finish */}
        <div
          ref={knobRef}
          onMouseDown={handleMouseDown}
          className={cn(
            'relative rounded-full cursor-pointer select-none transition-all duration-300',
            'border border-foreground/20',
            sizeClasses[size],
            disabled ? 'opacity-50 cursor-not-allowed' : 'hover:scale-105 active:scale-95',
            isDragging && 'scale-105'
          )}
          style={{
            background: `conic-gradient(from 0deg, 
              hsl(220, 15%, 25%), hsl(220, 10%, 35%), hsl(220, 15%, 45%), 
              hsl(220, 20%, 35%), hsl(220, 15%, 25%)
            )`,
            boxShadow: isDragging 
              ? `0 0 30px ${color}60, inset 0 4px 12px rgba(0,0,0,0.8), inset 0 -2px 6px rgba(255,255,255,0.1)`
              : `0 8px 25px ${color}30, inset 0 4px 12px rgba(0,0,0,0.6), inset 0 -2px 6px rgba(255,255,255,0.15)`
          }}
        >
          {/* Center Dot */}
          <div className="absolute inset-0 flex items-center justify-center">
            <div 
              className="w-2 h-2 rounded-full"
              style={{ 
                background: color,
                boxShadow: `0 0 8px ${color}`
              }} 
            />
          </div>

          {/* Value Arc */}
          <div className="absolute inset-2 rounded-full overflow-hidden">
            <svg className="w-full h-full transform -rotate-90">
              <circle
                cx="50%"
                cy="50%"
                r="40%"
                fill="none"
                stroke="hsl(var(--plugin-raised))"
                strokeWidth="2"
                opacity="0.3"
              />
              <circle
                cx="50%"
                cy="50%"
                r="40%"
                fill="none"
                stroke={color}
                strokeWidth="3"
                strokeLinecap="round"
                strokeDasharray={`${normalizedValue * 251.2} 251.2`}
                style={{
                  filter: `drop-shadow(0 0 4px ${color})`,
                  transition: 'stroke-dasharray 0.3s ease-out'
                }}
              />
            </svg>
          </div>

          {/* Hardware-style Pointer */}
          <div
            className="absolute top-2 left-1/2 w-1 origin-bottom transition-all duration-300"
            style={{ 
              transform: `translateX(-50%) rotate(${angle}deg)`,
              height: size === 'xl' ? '28px' : size === 'lg' ? '20px' : '16px',
              background: `linear-gradient(to bottom, rgba(255,255,255,0.9), rgba(255,255,255,0.7))`,
              borderRadius: '2px',
              boxShadow: `0 0 4px rgba(0,0,0,0.8), 0 0 8px ${color}80`
            }}
          />
        </div>

        {/* Value Display */}
        <div className="absolute -bottom-8 left-1/2 transform -translate-x-1/2 text-xs font-mono font-bold"
             style={{ color: color }}>
          {formatValue(value)}{unit}
        </div>
      </div>

      {/* Label */}
      <label className="text-sm font-bold text-foreground tracking-widest uppercase">
        {label}
      </label>
    </div>
  );
};