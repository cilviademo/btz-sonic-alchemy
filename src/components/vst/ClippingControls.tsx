import React from 'react';
import { ModernKnob } from './ModernKnob';
import { cn } from '@/lib/utils';

type ClippingType = 'soft' | 'hard' | 'tube' | 'tape' | 'digital';

interface ClippingControlsProps {
  type: ClippingType;
  blend: number;
  onTypeChange: (type: ClippingType) => void;
  onBlendChange: (blend: number) => void;
}

const CLIPPING_TYPES: { id: ClippingType; label: string; color: string; icon: string }[] = [
  { id: 'soft', label: 'Soft', color: 'hsl(120, 60%, 50%)', icon: '∿' },
  { id: 'hard', label: 'Hard', color: 'hsl(0, 80%, 60%)', icon: '⫸' },
  { id: 'tube', label: 'Tube', color: 'hsl(35, 90%, 60%)', icon: '○' },
  { id: 'tape', label: 'Tape', color: 'hsl(15, 85%, 55%)', icon: '▢' },
  { id: 'digital', label: 'Digital', color: 'hsl(240, 80%, 60%)', icon: '▣' },
];

export const ClippingControls: React.FC<ClippingControlsProps> = ({
  type,
  blend,
  onTypeChange,
  onBlendChange
}) => {
  const currentType = CLIPPING_TYPES.find(t => t.id === type) || CLIPPING_TYPES[0];

  return (
    <div className="flex flex-col items-center gap-4">
      <div className="text-sm font-bold text-foreground tracking-widest uppercase mb-2">
        CLIPPING
      </div>
      
      <div className="flex items-center gap-6">
        {/* Clipping Type Selector */}
        <div className="flex flex-col items-center gap-3">
          <div className="text-xs font-medium text-foreground/70 uppercase tracking-wide">
            Type
          </div>
          <div className="flex bg-plugin-surface rounded-2xl p-2 border border-audio-primary/20">
            {CLIPPING_TYPES.map((clipType) => (
              <button
                key={clipType.id}
                onClick={() => onTypeChange(clipType.id)}
                className={cn(
                  "px-4 py-3 rounded-xl transition-all duration-300 text-sm font-bold min-w-[60px] flex flex-col items-center gap-1",
                  type === clipType.id
                    ? "text-background shadow-lg transform scale-105"
                    : "text-foreground/60 hover:text-foreground/80 hover:bg-plugin-raised/50"
                )}
                style={type === clipType.id ? {
                  background: clipType.color,
                  boxShadow: `0 0 20px ${clipType.color}60`
                } : {}}
              >
                <span className="text-lg">{clipType.icon}</span>
                <span className="text-xs">{clipType.label}</span>
              </button>
            ))}
          </div>
        </div>

        {/* Blend Knob */}
        <div className="flex flex-col items-center">
          <ModernKnob 
            value={blend} 
            onChange={onBlendChange} 
            label="BLEND" 
            min={0} 
            max={1}
            size="lg"
            spectrum={new Array(16).fill(0).map(() => Math.random() * blend)}
          />
        </div>
      </div>

      {/* Clipping Visualization */}
      <div className="w-32 h-16 bg-plugin-surface rounded-lg border border-audio-primary/10 overflow-hidden">
        <svg className="w-full h-full" viewBox="0 0 128 64">
          <defs>
            <linearGradient id="waveGrad" x1="0%" y1="0%" x2="100%" y2="0%">
              <stop offset="0%" stopColor={currentType.color} stopOpacity="0.6"/>
              <stop offset="50%" stopColor={currentType.color} stopOpacity="1"/>
              <stop offset="100%" stopColor={currentType.color} stopOpacity="0.6"/>
            </linearGradient>
          </defs>
          
          {/* Original waveform (faded) */}
          <path
            d="M 0 32 Q 32 8 64 32 Q 96 56 128 32"
            fill="none"
            stroke="hsl(var(--foreground))"
            strokeWidth="1"
            opacity="0.3"
          />
          
          {/* Clipped waveform */}
          <path
            d={
              type === 'soft' ? "M 0 32 Q 32 16 64 32 Q 96 48 128 32" :
              type === 'hard' ? "M 0 32 L 32 16 L 64 32 L 96 48 L 128 32" :
              type === 'tube' ? "M 0 32 Q 24 18 32 20 Q 40 22 64 32 Q 88 42 96 44 Q 104 46 128 32" :
              type === 'tape' ? "M 0 32 Q 30 14 34 18 Q 38 22 64 32 Q 90 42 94 46 Q 98 50 128 32" :
              "M 0 32 L 16 20 L 32 20 L 48 32 L 64 32 L 80 44 L 96 44 L 112 32 L 128 32"
            }
            fill="none"
            stroke="url(#waveGrad)"
            strokeWidth="2"
            opacity={0.7 + blend * 0.3}
            style={{
              filter: `drop-shadow(0 0 ${2 + blend * 4}px ${currentType.color})`
            }}
          />
        </svg>
      </div>
    </div>
  );
};