import React from 'react';
import { BTZPluginState } from './types';
import { cn } from '@/lib/utils';

interface ProcessingChainVisualizerProps {
  state: BTZPluginState;
  analysisData: {
    transientStrength: number;
    lowEndEnergy: number;
    loudnessScore: number;
    richness: number;
    spectralCentroid: number;
  };
}

const PROCESSING_STAGES = [
  { id: 'input', label: 'INPUT', icon: '🔊', description: 'Audio signal input' },
  { id: 'punch', label: 'PUNCH', icon: '⚡', description: '3-band transient + FET comp + gate' },
  { id: 'warmth', label: 'WARMTH', icon: '🔥', description: 'Even/odd sat + tape emulation' },
  { id: 'boom', label: 'BOOM', icon: '💥', description: 'Sub synth + dynamic low shelf' },
  { id: 'texture', label: 'TEXTURE', icon: '✨', description: 'Exciter + micro-IR + mod delay' },
  { id: 'drive', label: 'DRIVE', icon: '🚀', description: '3-band limiter + soft clip' },
  { id: 'console', label: 'CONSOLE', icon: '🎛️', description: 'Console emulation + glue' },
  { id: 'clipper', label: 'FL CLIPPER', icon: '📊', description: 'FL Studio style clipper' },
  { id: 'output', label: 'OUTPUT', icon: '🎵', description: 'Final output stage' },
];

export const ProcessingChainVisualizer: React.FC<ProcessingChainVisualizerProps> = ({
  state,
  analysisData
}) => {
  const getStageIntensity = (stageId: string): number => {
    switch (stageId) {
      case 'punch': return state.punch * analysisData.transientStrength;
      case 'warmth': return state.warmth * analysisData.richness;
      case 'boom': return state.boom * analysisData.lowEndEnergy;
      case 'texture': return state.texture ? 0.7 : 0;
      case 'drive': return state.drive * analysisData.loudnessScore;
      case 'console': return state.consoleGlue ? 0.5 : 0;
      case 'clipper': return state.clippingEnabled ? (state.clippingBlend || 0.5) : 0;
      default: return 0.5;
    }
  };

  const getStageColor = (stageId: string): string => {
    switch (stageId) {
      case 'punch': return 'hsl(var(--audio-primary))';
      case 'warmth': return 'hsl(var(--audio-secondary))';
      case 'boom': return 'hsl(var(--audio-tertiary))';
      case 'texture': return 'hsl(var(--audio-success))';
      case 'drive': return 'hsl(var(--audio-warning))';
      case 'console': return 'hsl(280, 60%, 60%)';
      case 'clipper': return 'hsl(var(--audio-error))';
      default: return 'hsl(var(--foreground))';
    }
  };

  return (
    <div className="bg-plugin-panel rounded-2xl p-6 border border-audio-primary/10">
      <div className="flex items-center justify-between mb-6">
        <h3 className="text-lg font-bold text-foreground tracking-wide">
          SIGNAL PROCESSING CHAIN
        </h3>
        <div className="text-xs text-foreground/60">
          ZDF + TPT + AI-Enhanced
        </div>
      </div>

      {/* Processing Chain Flow */}
      <div className="space-y-4">
        {PROCESSING_STAGES.map((stage, index) => {
          const intensity = getStageIntensity(stage.id);
          const color = getStageColor(stage.id);
          const isActive = stage.id === 'input' || stage.id === 'output' || 
                         (stage.id === 'texture' && state.texture) ||
                         (stage.id === 'console' && state.consoleGlue) ||
                         (stage.id === 'clipper' && state.clippingEnabled) ||
                         (!['input', 'output', 'texture', 'console', 'clipper'].includes(stage.id));

          return (
            <div key={stage.id} className="flex items-center gap-4">
              {/* Stage Icon */}
              <div 
                className={cn(
                  "w-12 h-12 rounded-full border-2 flex items-center justify-center text-lg font-bold transition-all duration-300",
                  isActive 
                    ? "border-opacity-100 shadow-lg" 
                    : "border-opacity-30 opacity-50"
                )}
                style={{
                  borderColor: color,
                  background: isActive ? `${color}20` : 'transparent',
                  boxShadow: isActive ? `0 0 15px ${color}40` : 'none'
                }}
              >
                <span>{stage.icon}</span>
              </div>

              {/* Stage Info */}
              <div className="flex-1 min-w-0">
                <div className="flex items-center justify-between mb-2">
                  <h4 className="text-sm font-bold text-foreground tracking-wide">
                    {stage.label}
                  </h4>
                  <span className="text-xs font-mono text-foreground/70">
                    {(intensity * 100).toFixed(0)}%
                  </span>
                </div>
                
                {/* Activity Bar */}
                <div className="w-full h-2 bg-plugin-raised rounded-full overflow-hidden">
                  <div 
                    className="h-full transition-all duration-300 rounded-full"
                    style={{ 
                      width: `${Math.max(5, intensity * 100)}%`,
                      background: `linear-gradient(90deg, ${color}80, ${color})`,
                      boxShadow: isActive ? `0 0 8px ${color}60` : 'none'
                    }}
                  />
                </div>
                
                <div className="text-xs text-foreground/60 mt-1">
                  {stage.description}
                </div>
              </div>

              {/* Connection Line to Next Stage */}
              {index < PROCESSING_STAGES.length - 1 && (
                <div className="w-4 h-0.5 bg-foreground/20" />
              )}
            </div>
          );
        })}
      </div>

      {/* Technical Stats */}
      <div className="mt-6 pt-4 border-t border-foreground/10">
        <div className="grid grid-cols-3 gap-4 text-center">
          <div className="bg-plugin-surface rounded-lg p-3">
            <div className="text-xs text-foreground/70 uppercase">Oversampling</div>
            <div className="text-lg font-bold text-audio-primary">
              {state.oversampling ? `${state.oversamplingRate || 4}x` : 'OFF'}
            </div>
          </div>
          <div className="bg-plugin-surface rounded-lg p-3">
            <div className="text-xs text-foreground/70 uppercase">ZDF Mode</div>
            <div className="text-lg font-bold text-audio-secondary">
              {state.oversampling ? 'HQ' : 'STD'}
            </div>
          </div>
          <div className="bg-plugin-surface rounded-lg p-3">
            <div className="text-xs text-foreground/70 uppercase">Latency</div>
            <div className="text-lg font-bold text-audio-tertiary">
              {state.oversampling ? '2ms' : '<1ms'}
            </div>
          </div>
        </div>
      </div>

      {/* AI Status */}
      <div className="mt-4 text-center">
        <div className="text-xs text-foreground/60">
          {state.aiAutomation ? (
            <span className="text-audio-primary font-medium">
              🤖 AI optimizing signal flow in real-time
            </span>
          ) : (
            <span>Manual control mode</span>
          )}
        </div>
      </div>
    </div>
  );
};