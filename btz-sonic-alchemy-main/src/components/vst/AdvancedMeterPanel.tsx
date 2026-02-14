import React from 'react';
import { BTZPluginState } from './types';
import { cn } from '@/lib/utils';

interface AdvancedMeterPanelProps {
  state: BTZPluginState;
  meters: {
    inputLevel: number;
    outputLevel: number;
    lufsIntegrated: number;
    truePeak: number;
    isProcessing: boolean;
    analysisData: {
      transientStrength: number;
      lowEndEnergy: number;
      loudnessScore: number;
      richness: number;
      spectralCentroid: number;
    };
  };
}

export const AdvancedMeterPanel: React.FC<AdvancedMeterPanelProps> = ({
  state,
  meters
}) => {
  const getLevelColor = (level: number, type: 'lufs' | 'peak'): string => {
    if (type === 'lufs') {
      if (level > -6) return 'hsl(var(--audio-error))';
      if (level > -10) return 'hsl(var(--audio-warning))';
      if (level > -16) return 'hsl(var(--audio-success))';
      return 'hsl(var(--audio-primary))';
    } else {
      if (level > -0.1) return 'hsl(var(--audio-error))';
      if (level > -3) return 'hsl(var(--audio-warning))';
      if (level > -12) return 'hsl(var(--audio-success))';
      return 'hsl(var(--audio-primary))';
    }
  };

  const getLufsTargetStatus = (): { text: string; color: string } => {
    const target = state.lufsTarget || -14;
    const current = meters.lufsIntegrated;
    const diff = current - target;
    
    if (Math.abs(diff) < 1) {
      return { text: 'ON TARGET', color: 'hsl(var(--audio-success))' };
    } else if (diff > 1) {
      return { text: 'TOO LOUD', color: 'hsl(var(--audio-error))' };
    } else {
      return { text: 'TOO QUIET', color: 'hsl(var(--audio-warning))' };
    }
  };

  const targetStatus = getLufsTargetStatus();

  return (
    <div className="bg-plugin-panel rounded-2xl p-6 border border-audio-primary/10">
      <div className="flex items-center justify-between mb-6">
        <h3 className="text-lg font-bold text-foreground tracking-wide">
          PROFESSIONAL METERING
        </h3>
        <div className="text-xs text-foreground/60">
          EBU R128 • ITU-R BS.1770
        </div>
      </div>

      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        {/* LUFS Integrated Display */}
        <div className="space-y-4">
          <div className="text-center">
            <h4 className="text-sm font-bold text-foreground/80 uppercase tracking-wide mb-2">
              LUFS Integrated
            </h4>
            <div 
              className="text-6xl font-mono font-black mb-2"
              style={{ 
                color: getLevelColor(meters.lufsIntegrated, 'lufs'),
                textShadow: `0 0 20px ${getLevelColor(meters.lufsIntegrated, 'lufs')}60`
              }}
            >
              {meters.lufsIntegrated.toFixed(1)}
            </div>
            <div className="text-sm text-foreground/70">LUFS</div>
          </div>

          {/* LUFS Target Comparison */}
          <div className="bg-plugin-surface rounded-lg p-4">
            <div className="flex items-center justify-between mb-3">
              <span className="text-xs text-foreground/70 uppercase">Target</span>
              <span className="text-xs font-mono">
                {state.lufsTarget || -14} LUFS
              </span>
            </div>
            
            <div className="relative h-3 bg-plugin-raised rounded-full overflow-hidden">
              {/* Target marker */}
              <div 
                className="absolute top-0 w-1 h-full bg-foreground/60"
                style={{ left: '50%' }}
              />
              
              {/* Current level indicator */}
              <div 
                className="absolute top-0 w-2 h-full rounded-full transition-all duration-300"
                style={{ 
                  left: `${Math.min(100, Math.max(0, 50 + ((meters.lufsIntegrated - (state.lufsTarget || -14)) * 2)))}%`,
                  backgroundColor: targetStatus.color,
                  boxShadow: `0 0 8px ${targetStatus.color}`
                }}
              />
            </div>
            
            <div className="flex items-center justify-between mt-2">
              <span className="text-xs text-foreground/50">-20</span>
              <span 
                className="text-xs font-bold"
                style={{ color: targetStatus.color }}
              >
                {targetStatus.text}
              </span>
              <span className="text-xs text-foreground/50">-6</span>
            </div>
          </div>
        </div>

        {/* True Peak Display */}
        <div className="space-y-4">
          <div className="text-center">
            <h4 className="text-sm font-bold text-foreground/80 uppercase tracking-wide mb-2">
              True Peak
            </h4>
            <div 
              className="text-6xl font-mono font-black mb-2"
              style={{ 
                color: getLevelColor(meters.truePeak, 'peak'),
                textShadow: `0 0 20px ${getLevelColor(meters.truePeak, 'peak')}60`
              }}
            >
              {meters.truePeak > 0 ? '+' : ''}{meters.truePeak.toFixed(1)}
            </div>
            <div className="text-sm text-foreground/70">dBTP</div>
          </div>

          {/* Peak Level Bar */}
          <div className="bg-plugin-surface rounded-lg p-4">
            <div className="flex items-center justify-between mb-3">
              <span className="text-xs text-foreground/70 uppercase">Peak Level</span>
              <span className="text-xs font-mono">
                Ceiling: -1.0 dBTP
              </span>
            </div>
            
            <div className="relative h-3 bg-plugin-raised rounded-full overflow-hidden">
              {/* Safe zone (green) */}
              <div className="absolute left-0 top-0 w-3/5 h-full bg-audio-success/30 rounded-l-full" />
              
              {/* Warning zone (yellow) */}
              <div className="absolute left-3/5 top-0 w-1/5 h-full bg-audio-warning/30" />
              
              {/* Danger zone (red) */}
              <div className="absolute right-0 top-0 w-1/5 h-full bg-audio-error/30 rounded-r-full" />
              
              {/* Current level */}
              <div 
                className="absolute top-0 h-full rounded-full transition-all duration-100"
                style={{ 
                  width: `${Math.min(100, Math.max(0, ((meters.truePeak + 40) / 40) * 100))}%`,
                  backgroundColor: getLevelColor(meters.truePeak, 'peak'),
                  boxShadow: `0 0 8px ${getLevelColor(meters.truePeak, 'peak')}`
                }}
              />
              
              {/* -1.0 dBTP ceiling marker */}
              <div 
                className="absolute top-0 w-0.5 h-full bg-foreground"
                style={{ left: '95%' }}
              />
            </div>
            
            <div className="flex items-center justify-between mt-2">
              <span className="text-xs text-foreground/50">-40</span>
              <span className="text-xs text-foreground/50">-1.0</span>
              <span className="text-xs text-foreground/50">0</span>
            </div>
          </div>
        </div>
      </div>

      {/* Processing Stats */}
      <div className="mt-6 pt-4 border-t border-foreground/10">
        <div className="grid grid-cols-4 gap-4">
          <div className="text-center">
            <div className="text-xs text-foreground/70 uppercase mb-1">Headroom</div>
            <div className="text-lg font-bold text-audio-primary">
              {(-1.0 - meters.truePeak).toFixed(1)} dB
            </div>
          </div>
          <div className="text-center">
            <div className="text-xs text-foreground/70 uppercase mb-1">Dynamic Range</div>
            <div className="text-lg font-bold text-audio-secondary">
              {(meters.truePeak - meters.lufsIntegrated).toFixed(1)} LU
            </div>
          </div>
          <div className="text-center">
            <div className="text-xs text-foreground/70 uppercase mb-1">Gain Reduction</div>
            <div className="text-lg font-bold text-audio-warning">
              -{(state.drive * 6).toFixed(1)} dB
            </div>
          </div>
          <div className="text-center">
            <div className="text-xs text-foreground/70 uppercase mb-1">Processing</div>
            <div 
              className={cn(
                "text-lg font-bold",
                meters.isProcessing ? "text-audio-success animate-pulse" : "text-foreground/50"
              )}
            >
              {meters.isProcessing ? 'ACTIVE' : 'BYPASS'}
            </div>
          </div>
        </div>
      </div>

      {/* Compliance Indicators */}
      <div className="mt-4 flex justify-center gap-6">
        <div className={cn(
          "px-4 py-2 rounded-full text-xs font-bold border",
          meters.lufsIntegrated <= -14 
            ? "bg-audio-success/20 border-audio-success text-audio-success" 
            : "bg-plugin-raised/20 border-plugin-raised text-foreground/50"
        )}>
          SPOTIFY READY
        </div>
        <div className={cn(
          "px-4 py-2 rounded-full text-xs font-bold border",
          meters.truePeak <= -1.0 
            ? "bg-audio-success/20 border-audio-success text-audio-success" 
            : "bg-audio-error/20 border-audio-error text-audio-error"
        )}>
          NO CLIPPING
        </div>
        <div className={cn(
          "px-4 py-2 rounded-full text-xs font-bold border",
          meters.lufsIntegrated >= -16 && meters.lufsIntegrated <= -8 
            ? "bg-audio-success/20 border-audio-success text-audio-success" 
            : "bg-plugin-raised/20 border-plugin-raised text-foreground/50"
        )}>
          COMMERCIAL READY
        </div>
      </div>
    </div>
  );
};