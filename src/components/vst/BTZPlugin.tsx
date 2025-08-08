import React, { useState, useEffect } from 'react';
import { Knob } from './Knob';
import { ToggleButton } from './ToggleButton';
import { VUMeter } from './VUMeter';
import { cn } from '@/lib/utils';

interface BTZPluginState {
  punch: number;
  warmth: number;
  boom: number;
  mix: number;
  texture: boolean;
  drive: number;
}

export const BTZPlugin: React.FC = () => {
  const [state, setState] = useState<BTZPluginState>({
    punch: 0.5,
    warmth: 0.3,
    boom: 0.2,
    mix: 1.0,
    texture: false,
    drive: 0.0,
  });

  const [inputLevel, setInputLevel] = useState(0);
  const [outputLevel, setOutputLevel] = useState(0);
  const [isProcessing, setIsProcessing] = useState(false);

  // Simulate audio processing and metering
  useEffect(() => {
    const interval = setInterval(() => {
      const baseLevel = 0.3 + Math.random() * 0.4;
      const processedLevel = baseLevel * (1 + state.drive * 0.1) * state.mix;
      
      setInputLevel(baseLevel + (Math.random() - 0.5) * 0.1);
      setOutputLevel(Math.min(0.95, processedLevel + (Math.random() - 0.5) * 0.05));
      setIsProcessing(Math.random() > 0.7);
    }, 50);

    return () => clearInterval(interval);
  }, [state.drive, state.mix]);

  const updateParameter = (key: keyof BTZPluginState, value: any) => {
    setState(prev => ({ ...prev, [key]: value }));
  };

  return (
    <div className="w-full max-w-6xl mx-auto bg-plugin-surface border border-plugin-raised rounded-2xl shadow-[var(--shadow-panel)] overflow-hidden">
      {/* Header */}
      <div className="bg-gradient-to-r from-plugin-panel to-plugin-raised border-b border-plugin-raised p-4">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-4">
            <div className="text-2xl font-bold text-audio-primary font-['Inter']">BTZ</div>
            <div className="text-sm text-foreground/70">AI-Powered Audio Processing Suite</div>
          </div>
          
          <div className="flex items-center gap-4">
            {/* AI Processing Indicator */}
            <div className="flex items-center gap-2">
              <div className={cn(
                'w-2 h-2 rounded-full transition-all duration-200',
                isProcessing 
                  ? 'bg-audio-success shadow-[0_0_8px_var(--audio-success)]' 
                  : 'bg-foreground/30'
              )} />
              <span className="text-xs text-foreground/70">AI</span>
            </div>
            
            {/* Bypass/Active Toggle */}
            <ToggleButton
              value={true}
              onChange={() => {}}
              label="ACTIVE"
            />
          </div>
        </div>
      </div>

      <div className="p-8">
        <div className="grid grid-cols-1 lg:grid-cols-12 gap-8">
          
          {/* Input/Output Meters */}
          <div className="lg:col-span-2">
            <div className="bg-plugin-panel rounded-xl p-4 border border-plugin-raised">
              <h3 className="text-sm font-semibold text-foreground/80 mb-4 text-center">LEVELS</h3>
              <div className="flex justify-center gap-4">
                <VUMeter level={inputLevel} label="IN" size="md" />
                <VUMeter level={outputLevel} label="OUT" size="md" />
              </div>
            </div>
          </div>

          {/* Main Controls */}
          <div className="lg:col-span-8">
            <div className="bg-plugin-panel rounded-xl p-6 border border-plugin-raised">
              <h3 className="text-lg font-semibold text-foreground/80 mb-6 text-center">MAIN CONTROLS</h3>
              
              <div className="grid grid-cols-2 md:grid-cols-3 gap-8">
                <Knob
                  value={state.punch}
                  onChange={(value) => updateParameter('punch', value)}
                  label="PUNCH"
                  min={0}
                  max={1}
                  size="lg"
                />
                
                <Knob
                  value={state.warmth}
                  onChange={(value) => updateParameter('warmth', value)}
                  label="WARMTH"
                  min={0}
                  max={1}
                  size="lg"
                />
                
                <Knob
                  value={state.boom}
                  onChange={(value) => updateParameter('boom', value)}
                  label="BOOM"
                  min={0}
                  max={1}
                  size="lg"
                />
                
                <Knob
                  value={state.mix}
                  onChange={(value) => updateParameter('mix', value)}
                  label="MIX"
                  min={0}
                  max={1}
                  size="lg"
                />
                
                <Knob
                  value={state.drive}
                  onChange={(value) => updateParameter('drive', value)}
                  label="DRIVE"
                  min={0}
                  max={12}
                  unit="dB"
                  size="lg"
                />
                
                <div className="flex flex-col items-center justify-center gap-4">
                  <ToggleButton
                    value={state.texture}
                    onChange={(value) => updateParameter('texture', value)}
                    label="TEXTURE"
                  />
                  <div className="text-xs text-foreground/50 text-center">
                    Granular + Convolution
                  </div>
                </div>
              </div>
            </div>
          </div>

          {/* AI Analysis Panel */}
          <div className="lg:col-span-2">
            <div className="bg-plugin-panel rounded-xl p-4 border border-plugin-raised">
              <h3 className="text-sm font-semibold text-foreground/80 mb-4 text-center">AI ANALYSIS</h3>
              
              <div className="space-y-4">
                <div className="space-y-2">
                  <div className="flex justify-between text-xs">
                    <span className="text-foreground/70">Transients</span>
                    <span className="text-audio-primary">75%</span>
                  </div>
                  <div className="h-1 bg-plugin-surface rounded-full overflow-hidden">
                    <div className="h-full bg-audio-primary rounded-full transition-all duration-500" style={{ width: '75%' }} />
                  </div>
                </div>
                
                <div className="space-y-2">
                  <div className="flex justify-between text-xs">
                    <span className="text-foreground/70">Low End</span>
                    <span className="text-audio-success">60%</span>
                  </div>
                  <div className="h-1 bg-plugin-surface rounded-full overflow-hidden">
                    <div className="h-full bg-audio-success rounded-full transition-all duration-500" style={{ width: '60%' }} />
                  </div>
                </div>
                
                <div className="space-y-2">
                  <div className="flex justify-between text-xs">
                    <span className="text-foreground/70">Richness</span>
                    <span className="text-audio-secondary">45%</span>
                  </div>
                  <div className="h-1 bg-plugin-surface rounded-full overflow-hidden">
                    <div className="h-full bg-audio-secondary rounded-full transition-all duration-500" style={{ width: '45%' }} />
                  </div>
                </div>
                
                <div className="space-y-2">
                  <div className="flex justify-between text-xs">
                    <span className="text-foreground/70">Loudness</span>
                    <span className="text-audio-warning">82%</span>
                  </div>
                  <div className="h-1 bg-plugin-surface rounded-full overflow-hidden">
                    <div className="h-full bg-audio-warning rounded-full transition-all duration-500" style={{ width: '82%' }} />
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>

        {/* Processing Chain Visualization */}
        <div className="mt-8 bg-plugin-panel rounded-xl p-6 border border-plugin-raised">
          <h3 className="text-lg font-semibold text-foreground/80 mb-4 text-center">PROCESSING CHAIN</h3>
          
          <div className="flex items-center justify-center gap-3 flex-wrap">
            {[
              { name: 'Gate', active: state.punch > 0.3 },
              { name: 'Transient Shaper', active: state.punch > 0.1 },
              { name: 'Compressor', active: state.punch > 0.2 },
              { name: 'Wave Shaper', active: state.warmth > 0.1 },
              { name: 'Fuzz', active: state.warmth > 0.5 },
              { name: 'Tape Emulator', active: state.warmth > 0.2 },
              { name: 'Bass Enhancer', active: state.boom > 0.1 },
              { name: 'EQ', active: state.boom > 0.2 },
              { name: 'Convolution', active: state.texture },
              { name: 'Granular', active: state.texture },
              { name: 'Console Emulator', active: state.mix > 0.1 },
              { name: 'Limiter', active: state.drive > 0.1 },
            ].map((processor, index) => (
              <div key={processor.name} className="flex items-center gap-2">
                <div
                  className={cn(
                    'px-3 py-1 rounded-md text-xs font-medium transition-all duration-200',
                    processor.active
                      ? 'bg-audio-primary/20 text-audio-primary border border-audio-primary/30'
                      : 'bg-plugin-surface text-foreground/50 border border-plugin-raised'
                  )}
                >
                  {processor.name}
                </div>
                {index < 11 && <div className="text-foreground/30">→</div>}
              </div>
            ))}
          </div>
        </div>
      </div>
    </div>
  );
};