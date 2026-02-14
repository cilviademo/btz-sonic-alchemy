import React from 'react';
import { ToggleButton } from './ToggleButton';
import { BTZPluginState } from './types';
import { cn } from '@/lib/utils';

interface AIAutomationPanelProps {
  state: BTZPluginState;
  updateParameter: (key: keyof BTZPluginState, value: any) => void;
  analysisData: {
    transientStrength: number;
    lowEndEnergy: number;
    loudnessScore: number;
    richness: number;
    spectralCentroid: number;
  };
}

export const AIAutomationPanel: React.FC<AIAutomationPanelProps> = ({
  state,
  updateParameter,
  analysisData
}) => {
  return (
    <div className="bg-plugin-panel rounded-2xl p-6 border border-audio-primary/10">
      <div className="flex items-center justify-between mb-6">
        <h3 className="text-lg font-bold text-foreground tracking-wide">
          AI AUTOMATION
        </h3>
        <div 
          className={cn(
            "w-3 h-3 rounded-full transition-all duration-300",
            state.aiAutomation ? "bg-audio-primary animate-pulse" : "bg-plugin-raised"
          )}
          style={state.aiAutomation ? {
            boxShadow: '0 0 10px hsl(var(--audio-primary))'
          } : {}}
        />
      </div>

      {/* AI Toggle */}
      <div className="mb-6">
        <ToggleButton
          value={!!state.aiAutomation}
          onChange={(v) => updateParameter('aiAutomation', v)}
          label="AI AUTOMATION"
          className={cn(
            "w-full py-4 rounded-xl font-bold text-sm border-2 transition-all duration-300",
            state.aiAutomation
              ? "bg-audio-primary border-audio-primary text-background shadow-[0_0_20px_hsl(var(--audio-primary))]"
              : "bg-plugin-raised/50 border-plugin-raised hover:bg-plugin-raised text-foreground/70"
          )}
        />
      </div>

      {/* AI Analysis Display */}
      <div className="space-y-4">
        <h4 className="text-sm font-bold text-foreground/80 uppercase tracking-wide">
          Real-time Analysis
        </h4>
        
        <div className="grid grid-cols-2 gap-4">
          {/* Transient Strength */}
          <div className="bg-plugin-surface rounded-lg p-3">
            <div className="text-xs font-medium text-foreground/70 mb-2 uppercase">
              Transient
            </div>
            <div className="flex items-center justify-between">
              <div className="h-2 bg-plugin-raised rounded-full flex-1 mr-2 overflow-hidden">
                <div 
                  className="h-full bg-audio-primary transition-all duration-300"
                  style={{ 
                    width: `${analysisData.transientStrength * 100}%`,
                    boxShadow: '0 0 4px hsl(var(--audio-primary))'
                  }}
                />
              </div>
              <span className="text-xs font-bold text-audio-primary">
                {(analysisData.transientStrength * 100).toFixed(0)}%
              </span>
            </div>
          </div>

          {/* Low End Energy */}
          <div className="bg-plugin-surface rounded-lg p-3">
            <div className="text-xs font-medium text-foreground/70 mb-2 uppercase">
              Low End
            </div>
            <div className="flex items-center justify-between">
              <div className="h-2 bg-plugin-raised rounded-full flex-1 mr-2 overflow-hidden">
                <div 
                  className="h-full bg-audio-secondary transition-all duration-300"
                  style={{ 
                    width: `${analysisData.lowEndEnergy * 100}%`,
                    boxShadow: '0 0 4px hsl(var(--audio-secondary))'
                  }}
                />
              </div>
              <span className="text-xs font-bold text-audio-secondary">
                {(analysisData.lowEndEnergy * 100).toFixed(0)}%
              </span>
            </div>
          </div>

          {/* Loudness Score */}
          <div className="bg-plugin-surface rounded-lg p-3">
            <div className="text-xs font-medium text-foreground/70 mb-2 uppercase">
              Loudness
            </div>
            <div className="flex items-center justify-between">
              <div className="h-2 bg-plugin-raised rounded-full flex-1 mr-2 overflow-hidden">
                <div 
                  className="h-full bg-audio-warning transition-all duration-300"
                  style={{ 
                    width: `${analysisData.loudnessScore * 100}%`,
                    boxShadow: '0 0 4px hsl(var(--audio-warning))'
                  }}
                />
              </div>
              <span className="text-xs font-bold text-audio-warning">
                {(analysisData.loudnessScore * 100).toFixed(0)}%
              </span>
            </div>
          </div>

          {/* Richness */}
          <div className="bg-plugin-surface rounded-lg p-3">
            <div className="text-xs font-medium text-foreground/70 mb-2 uppercase">
              Richness
            </div>
            <div className="flex items-center justify-between">
              <div className="h-2 bg-plugin-raised rounded-full flex-1 mr-2 overflow-hidden">
                <div 
                  className="h-full bg-audio-tertiary transition-all duration-300"
                  style={{ 
                    width: `${analysisData.richness * 100}%`,
                    boxShadow: '0 0 4px hsl(var(--audio-tertiary))'
                  }}
                />
              </div>
              <span className="text-xs font-bold text-audio-tertiary">
                {(analysisData.richness * 100).toFixed(0)}%
              </span>
            </div>
          </div>
        </div>

        {/* AI Status */}
        <div className="text-center">
          <div className="text-xs text-foreground/60">
            {state.aiAutomation ? (
              <span className="text-audio-primary font-medium">
                🤖 AI adapting parameters in real-time
              </span>
            ) : (
              <span>AI automation disabled</span>
            )}
          </div>
        </div>
      </div>
    </div>
  );
};