import React from 'react';
import { ModernKnob } from './ModernKnob';
import { ToggleButton } from './ToggleButton';
import { BTZPluginState } from './types';
import { cn } from '@/lib/utils';

type ClippingType = 'soft' | 'hard' | 'tube' | 'tape' | 'digital';

interface EnhancedClippingControlsProps {
  state: BTZPluginState;
  updateParameter: (key: keyof BTZPluginState, value: any) => void;
}

const CLIPPING_TYPES: { id: ClippingType; label: string; color: string; icon: string; description: string }[] = [
  { id: 'soft', label: 'Soft', color: 'hsl(120, 60%, 50%)', icon: '∿', description: 'FL Studio style soft limiting' },
  { id: 'hard', label: 'Hard', color: 'hsl(0, 80%, 60%)', icon: '⫸', description: 'Aggressive hard clipping' },
  { id: 'tube', label: 'Tube', color: 'hsl(35, 90%, 60%)', icon: '○', description: 'Warm tube saturation' },
  { id: 'tape', label: 'Tape', color: 'hsl(15, 85%, 55%)', icon: '▢', description: 'Tape machine limiting' },
  { id: 'digital', label: 'Digital', color: 'hsl(240, 80%, 60%)', icon: '▣', description: 'Clean digital limiting' },
];

export const EnhancedClippingControls: React.FC<EnhancedClippingControlsProps> = ({
  state,
  updateParameter
}) => {
  const currentType = CLIPPING_TYPES.find(t => t.id === state.clippingType) || CLIPPING_TYPES[0];

  return (
    <div className="bg-plugin-panel rounded-2xl p-6 border border-audio-primary/10">
      <div className="flex items-center justify-between mb-6">
        <h3 className="text-lg font-bold text-foreground tracking-wide">
          FL STUDIO CLIPPER
        </h3>
        <div className="text-xs text-foreground/60">
          Billboard-level loudness without distortion
        </div>
      </div>

      {/* Clipping Enable Toggle */}
      <div className="mb-6">
        <ToggleButton
          value={!!state.clippingEnabled}
          onChange={(v) => updateParameter('clippingEnabled', v)}
          label="CLIPPING ENABLED"
          className={cn(
            "w-full py-4 rounded-xl font-bold text-sm border-2 transition-all duration-300",
            state.clippingEnabled
              ? "bg-audio-warning border-audio-warning text-background shadow-[0_0_20px_hsl(var(--audio-warning))]"
              : "bg-plugin-raised/50 border-plugin-raised hover:bg-plugin-raised text-foreground/70"
          )}
        />
      </div>

      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        {/* Clipping Type Selector */}
        <div className="space-y-4">
          <h4 className="text-sm font-bold text-foreground/80 uppercase tracking-wide text-center">
            Clipping Type
          </h4>
          <div className="grid grid-cols-3 gap-2">
            {CLIPPING_TYPES.map((clipType) => (
              <button
                key={clipType.id}
                onClick={() => updateParameter('clippingType', clipType.id)}
                className={cn(
                  "px-3 py-4 rounded-xl transition-all duration-300 text-xs font-bold flex flex-col items-center gap-2",
                  state.clippingType === clipType.id
                    ? "text-background shadow-lg transform scale-105"
                    : "text-foreground/60 hover:text-foreground/80 hover:bg-plugin-raised/50",
                  !state.clippingEnabled && "opacity-50"
                )}
                style={state.clippingType === clipType.id ? {
                  background: clipType.color,
                  boxShadow: `0 0 15px ${clipType.color}60`
                } : {}}
                disabled={!state.clippingEnabled}
                title={clipType.description}
              >
                <span className="text-lg">{clipType.icon}</span>
                <span>{clipType.label}</span>
              </button>
            ))}
          </div>
        </div>

        {/* Blend Control */}
        <div className="flex flex-col items-center space-y-4">
          <ModernKnob 
            value={state.clippingBlend || 0.5} 
            onChange={(v) => updateParameter('clippingBlend', v)} 
            label="BLEND" 
            min={0} 
            max={1}
            size="lg"
            disabled={!state.clippingEnabled}
            spectrum={new Array(16).fill(0).map(() => Math.random() * (state.clippingBlend || 0.5))}
          />
          <div className="text-xs text-center text-foreground/60 max-w-[120px]">
            Wet/Dry mix of clipped signal
          </div>
        </div>
      </div>

      {/* Clipping Visualization */}
      <div className="mt-6">
        <h4 className="text-sm font-bold text-foreground/80 uppercase tracking-wide text-center mb-3">
          Clipping Curve
        </h4>
        <div className="w-full h-20 bg-plugin-surface rounded-lg border border-foreground/10 overflow-hidden">
          <svg className="w-full h-full" viewBox="0 0 200 80">
            <defs>
              <linearGradient id="clippingWaveGrad" x1="0%" y1="0%" x2="100%" y2="0%">
                <stop offset="0%" stopColor={currentType.color} stopOpacity="0.6"/>
                <stop offset="50%" stopColor={currentType.color} stopOpacity="1"/>
                <stop offset="100%" stopColor={currentType.color} stopOpacity="0.6"/>
              </linearGradient>
            </defs>
            
            {/* Original waveform (faded) */}
            <path
              d="M 0 40 Q 40 10 80 40 Q 120 70 160 40 Q 180 25 200 40"
              fill="none"
              stroke="rgba(255,255,255,0.2)"
              strokeWidth="2"
            />
            
            {/* Clipped waveform based on type */}
            <path
              d={
                state.clippingType === 'soft' ? "M 0 40 Q 40 25 80 40 Q 120 55 160 40 Q 180 32 200 40" :
                state.clippingType === 'hard' ? "M 0 40 L 40 25 L 80 40 L 120 55 L 160 40 L 180 32 L 200 40" :
                state.clippingType === 'tube' ? "M 0 40 Q 35 28 40 30 Q 45 32 80 40 Q 115 48 120 50 Q 125 52 160 40 Q 175 34 180 36 Q 185 38 200 40" :
                state.clippingType === 'tape' ? "M 0 40 Q 38 26 42 28 Q 46 30 80 40 Q 118 52 122 54 Q 126 56 160 40 Q 178 33 182 35 Q 186 37 200 40" :
                "M 0 40 L 20 30 L 40 30 L 60 40 L 80 40 L 100 50 L 120 50 L 140 40 L 160 40 L 180 35 L 200 40"
              }
              fill="none"
              stroke="url(#clippingWaveGrad)"
              strokeWidth="3"
              opacity={state.clippingEnabled ? 0.8 + (state.clippingBlend || 0.5) * 0.2 : 0.3}
              style={{
                filter: state.clippingEnabled ? `drop-shadow(0 0 ${4 + (state.clippingBlend || 0.5) * 8}px ${currentType.color})` : 'none'
              }}
            />
            
            {/* Clipping threshold lines */}
            {state.clippingEnabled && (
              <>
                <line x1="0" y1="25" x2="200" y2="25" stroke={currentType.color} strokeWidth="1" opacity="0.3" strokeDasharray="4,4" />
                <line x1="0" y1="55" x2="200" y2="55" stroke={currentType.color} strokeWidth="1" opacity="0.3" strokeDasharray="4,4" />
              </>
            )}
          </svg>
        </div>
      </div>

      {/* LUFS Display */}
      {state.clippingEnabled && (
        <div className="mt-4 text-center">
          <div className="text-xs text-foreground/60">
            Target: <span className="text-audio-warning font-bold">
              {state.lufsTarget ? `${state.lufsTarget} LUFS` : '-6 LUFS (Billboard Level)'}
            </span>
          </div>
        </div>
      )}
    </div>
  );
};