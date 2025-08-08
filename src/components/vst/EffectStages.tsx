import React from 'react';
import { ModernKnob } from './ModernKnob';
import { ToggleButton } from './ToggleButton';
import { BTZPluginState } from './types';
import { cn } from '@/lib/utils';

interface EffectStagesProps {
  state: BTZPluginState;
  updateParameter: (key: keyof BTZPluginState, value: any) => void;
  meters: any;
}

const EFFECT_STAGES = [
  {
    id: 'punch',
    label: 'TRANSIENT SHAPER',
    color: 'hsl(var(--audio-primary))',
    description: 'Attack and sustain control',
    parameters: ['punch']
  },
  {
    id: 'warmth',
    label: 'HARMONIC ENHANCER',
    color: 'hsl(var(--audio-secondary))',
    description: 'Midrange warmth and presence',
    parameters: ['warmth']
  },
  {
    id: 'boom',
    label: 'LOW END PROCESSOR',
    color: 'hsl(var(--audio-tertiary))',
    description: 'Sub-bass enhancement',
    parameters: ['boom']
  },
  {
    id: 'drive',
    label: 'SATURATION DRIVE',
    color: 'hsl(var(--audio-warning))',
    description: 'Harmonic distortion',
    parameters: ['drive']
  }
];

export const EffectStages: React.FC<EffectStagesProps> = ({
  state,
  updateParameter,
  meters
}) => {
  return (
    <div className="space-y-6">
      <h2 className="text-2xl font-bold text-center text-foreground tracking-wide">
        EFFECT STAGES
      </h2>
      
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        {EFFECT_STAGES.map((stage, index) => {
          const paramValue = state[stage.parameters[0] as keyof BTZPluginState] as number;
          const isActive = paramValue > 0.1;
          
          return (
            <div
              key={stage.id}
              className={cn(
                "relative bg-plugin-panel rounded-2xl p-6 border-2 transition-all duration-300",
                isActive 
                  ? "border-audio-primary/30" 
                  : "border-plugin-raised/30 hover:border-audio-primary/20"
              )}
              style={isActive ? {
                boxShadow: `0 0 20px ${stage.color}20`
              } : {}}
            >
              {/* Stage Header */}
              <div className="flex items-center justify-between mb-4">
                <div>
                  <h3 className="text-lg font-bold text-foreground tracking-wide">
                    {stage.label}
                  </h3>
                  <p className="text-xs text-foreground/60 mt-1">
                    {stage.description}
                  </p>
                </div>
                <div 
                  className={cn(
                    "w-4 h-4 rounded-full transition-all duration-300",
                    isActive ? "animate-pulse" : ""
                  )}
                  style={{ 
                    background: isActive ? stage.color : 'hsl(var(--plugin-raised))',
                    boxShadow: isActive ? `0 0 10px ${stage.color}` : 'none'
                  }}
                />
              </div>

              {/* Stage Visualization */}
              <div className="bg-plugin-surface rounded-lg p-4 mb-4 h-20 overflow-hidden">
                <svg className="w-full h-full" viewBox="0 0 200 60">
                  <defs>
                    <linearGradient id={`stageGrad${index}`} x1="0%" y1="0%" x2="100%" y2="0%">
                      <stop offset="0%" stopColor={stage.color} stopOpacity="0.3"/>
                      <stop offset="50%" stopColor={stage.color} stopOpacity="0.8"/>
                      <stop offset="100%" stopColor={stage.color} stopOpacity="0.3"/>
                    </linearGradient>
                  </defs>
                  
                  {/* Stage-specific visualization */}
                  {stage.id === 'punch' && (
                    <path
                      d="M 0 30 L 30 30 L 40 10 L 50 30 L 200 30"
                      fill="none"
                      stroke={`url(#stageGrad${index})`}
                      strokeWidth="3"
                      opacity={paramValue * 0.8 + 0.2}
                    />
                  )}
                  {stage.id === 'warmth' && (
                    <path
                      d="M 0 45 Q 50 20 100 30 Q 150 40 200 25"
                      fill="none"
                      stroke={`url(#stageGrad${index})`}
                      strokeWidth="3"
                      opacity={paramValue * 0.8 + 0.2}
                    />
                  )}
                  {stage.id === 'boom' && (
                    <path
                      d="M 0 30 Q 25 10 50 30 L 200 30"
                      fill="none"
                      stroke={`url(#stageGrad${index})`}
                      strokeWidth="4"
                      opacity={paramValue * 0.8 + 0.2}
                    />
                  )}
                  {stage.id === 'drive' && (
                    <path
                      d="M 0 30 L 50 30 L 60 15 L 70 45 L 80 15 L 90 45 L 100 30 L 200 30"
                      fill="none"
                      stroke={`url(#stageGrad${index})`}
                      strokeWidth="2"
                      opacity={paramValue * 0.8 + 0.2}
                    />
                  )}
                </svg>
              </div>

              {/* Stage Controls */}
              <div className="flex justify-center">
                <ModernKnob 
                  value={paramValue}
                  onChange={(v) => updateParameter(stage.parameters[0] as keyof BTZPluginState, v)}
                  label={stage.parameters[0].toUpperCase()}
                  min={0}
                  max={stage.id === 'drive' ? 2 : 1}
                  size="lg"
                  spectrum={meters.spectrumData?.slice(index * 8, (index + 1) * 8) || []}
                />
              </div>

              {/* Bypass Toggle */}
              <div className="flex justify-center mt-4">
                <ToggleButton
                  value={isActive}
                  onChange={(v) => updateParameter(stage.parameters[0] as keyof BTZPluginState, v ? 0.5 : 0)}
                  label="BYPASS"
                  className={cn(
                    "px-4 py-2 rounded-lg text-xs font-bold border transition-all duration-300",
                    isActive
                      ? "bg-plugin-raised border-plugin-raised text-foreground/70"
                      : "bg-audio-primary border-audio-primary text-background"
                  )}
                />
              </div>
            </div>
          );
        })}
      </div>
    </div>
  );
};