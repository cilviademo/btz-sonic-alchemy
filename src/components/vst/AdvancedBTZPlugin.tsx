import React, { useState, useEffect, useCallback } from 'react';
import { ModernKnob } from './ModernKnob';
import { ToggleButton } from './ToggleButton';
import { SpectrumAnalyzer } from './SpectrumAnalyzer';
import { ModernVUMeter } from './ModernVUMeter';
import { BTZPluginState } from './types';
import { PresetsSelect, PresetOption } from './PresetsSelect';
import { cn } from '@/lib/utils';

const STREAMING_PRESETS: PresetOption[] = [
  {
    id: 'streaming-safe',
    label: 'Streaming Safe (-14 LUFS)',
    state: { punch: 0.6, warmth: 0.4, boom: 0.3, mix: 0.8, drive: 0.8, texture: true, lufsTarget: -14 }
  },
  {
    id: 'loud-clean',  
    label: 'Loud & Clean (-8 LUFS)',
    state: { punch: 0.7, warmth: 0.5, boom: 0.4, mix: 0.9, drive: 1.2, texture: false, lufsTarget: -8 }
  },
  {
    id: 'neural-enhance',
    label: 'AI Neural Enhance',
    state: { punch: 0.5, warmth: 0.6, boom: 0.35, mix: 0.85, drive: 0.9, texture: true, aiEnhance: true }
  },
  {
    id: 'timbral-sculpt',
    label: 'Timbral Sculpt',
    state: { punch: 0.65, warmth: 0.7, boom: 0.5, mix: 0.95, drive: 1.0, texture: true, timbralTransfer: true }
  }
];

export const AdvancedBTZPlugin: React.FC = () => {
  const [state, setState] = useState<BTZPluginState & {
    lufsTarget?: number;
    aiEnhance?: boolean;
    timbralTransfer?: boolean;
    oversamplingRate?: number;
    spectrumData?: Float32Array;
  }>({
    punch: 0.5,
    warmth: 0.4,
    boom: 0.3,
    mix: 0.8,
    drive: 0.5,
    texture: false,
    active: true,
    oversampling: true,
    lufsTarget: -8,
    aiEnhance: false,
    timbralTransfer: false,
    oversamplingRate: 8
  });

  const [meters, setMeters] = useState({
    inputLevel: 0,
    outputLevel: 0,
    inputPeak: 0,
    outputPeak: 0,
    gainReduction: 0,
    lufsIntegrated: -12.5,
    truePeak: -2.1,
    isProcessing: false,
    spectrumData: new Float32Array(512)
  });

  // Simulate advanced metering with realistic behavior
  useEffect(() => {
    const interval = setInterval(() => {
      if (!state.active) {
        setMeters(prev => ({ ...prev, isProcessing: false }));
        return;
      }

      const baseInput = 0.3 + Math.random() * 0.4;
      const inputLevel = baseInput * (1 + state.drive * 0.3);
      const outputLevel = inputLevel * state.mix * (state.active ? 1 : 0);
      
      // Simulate gain reduction from processing
      const grAmount = (state.punch * 0.8 + state.warmth * 0.4) * Math.random() * 15;
      
      // Simulate LUFS and true peak with realistic values
      const targetLufs = state.lufsTarget || -8;
      const lufsIntegrated = targetLufs + (Math.random() - 0.5) * 2;
      const truePeak = Math.max(-1.0, lufsIntegrated + 8 + (Math.random() - 0.5) * 3);

      // Generate realistic spectrum data
      const spectrumData = new Float32Array(512);
      for (let i = 0; i < 512; i++) {
        const freq = (i / 512) * 24000;
        let magnitude = Math.random() * 0.1;
        
        // Add frequency content based on controls
        if (freq < 200) magnitude += state.boom * 0.4 + Math.random() * 0.2;
        if (freq > 60 && freq < 500) magnitude += state.punch * 0.3;
        if (freq > 1000 && freq < 5000) magnitude += state.warmth * 0.35;
        if (freq > 8000) magnitude += state.texture ? 0.25 : 0.1;
        
        spectrumData[i] = Math.max(0, Math.min(1, magnitude));
      }

      setMeters({
        inputLevel,
        outputLevel,
        inputPeak: Math.max(inputLevel, meters.inputPeak * 0.95),
        outputPeak: Math.max(outputLevel, meters.outputPeak * 0.95),
        gainReduction: grAmount,
        lufsIntegrated,
        truePeak,
        isProcessing: inputLevel > 0.1,
        spectrumData
      });
    }, 50);

    return () => clearInterval(interval);
  }, [state]);

  const updateParameter = useCallback((key: keyof typeof state, value: any) => {
    setState(prev => ({ ...prev, [key]: value }));
  }, []);

  const applyPreset = useCallback((preset: PresetOption) => {
    setState(prev => ({ ...prev, ...preset.state }));
  }, []);

  return (
    <div className="min-h-screen bg-gradient-to-br from-plugin-surface via-plugin-panel to-plugin-surface">
      {/* Modern Header */}
      <header className="bg-gradient-to-r from-plugin-panel to-plugin-raised border-b border-plugin-highlight p-4 shadow-[var(--shadow-depth)]">
        <div className="flex items-center justify-between flex-wrap gap-4">
          <div className="flex items-center gap-6">
            <div className="flex flex-col">
              <h1 className="text-2xl font-bold bg-gradient-to-r from-audio-primary to-audio-primary-glow bg-clip-text text-transparent font-display">
                BTZ — Box Tone Zone Enhancer
              </h1>
              <p className="text-sm text-foreground/60">
                Neural-Enhanced Drum Tone Sculptor • v2.0 Pro
              </p>
            </div>
            
            {/* AI Status */}
            <div className="flex items-center gap-3 bg-plugin-surface/50 rounded-lg px-3 py-2 border border-plugin-raised">
              <div className={cn(
                'w-2 h-2 rounded-full transition-all duration-200',
                meters.isProcessing 
                  ? 'bg-audio-success shadow-[var(--glow-accent)]' 
                  : 'bg-foreground/30'
              )} />
              <div className="text-xs">
                <div className="text-foreground/70">AI Engine</div>
                <div className="text-audio-success font-medium">
                  {state.aiEnhance ? 'Neural' : 'Classic'} • {state.oversamplingRate}x OS
                </div>
              </div>
            </div>
          </div>

          <div className="flex items-center gap-4">
            <ToggleButton 
              value={state.active} 
              onChange={(v) => updateParameter('active', v)} 
              label={state.active ? 'ACTIVE' : 'BYPASS'} 
            />
            <ToggleButton 
              value={state.oversampling} 
              onChange={(v) => updateParameter('oversampling', v)} 
              label={state.oversampling ? `HQ ${state.oversamplingRate}x` : 'HQ OFF'} 
            />
            <PresetsSelect onApply={applyPreset} presets={STREAMING_PRESETS} />
          </div>
        </div>
      </header>

      {/* Main Content */}
      <div className="p-6 space-y-6">
        {/* Top Row - Meters & Spectrum */}
        <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
          {/* Input/Output Meters */}
          <div className="bg-gradient-to-br from-plugin-panel to-plugin-raised rounded-xl p-4 border border-plugin-highlight shadow-[var(--shadow-depth)]">
            <h3 className="text-sm font-semibold text-foreground/80 mb-4 text-center">I/O LEVELS</h3>
            <div className="flex justify-center gap-8">
              <ModernVUMeter 
                level={meters.inputLevel} 
                peak={meters.inputPeak}
                label="INPUT" 
                size="md"
                showPeakHold
              />
              <ModernVUMeter 
                level={meters.outputLevel} 
                peak={meters.outputPeak}
                label="OUTPUT" 
                size="md"
                showPeakHold
              />
            </div>
          </div>

          {/* Spectrum Analyzer */}
          <div className="lg:col-span-2">
            <div className="bg-gradient-to-br from-plugin-panel to-plugin-raised rounded-xl p-4 border border-plugin-highlight shadow-[var(--shadow-depth)]">
              <div className="flex justify-between items-center mb-4">
                <h3 className="text-sm font-semibold text-foreground/80">REAL-TIME SPECTRUM</h3>
                <div className="flex gap-2">
                  <button className="px-2 py-1 text-xs bg-plugin-surface/80 text-foreground/70 rounded border border-plugin-raised hover:bg-plugin-raised transition-colors">
                    PRE
                  </button>
                  <button className="px-2 py-1 text-xs bg-audio-primary/20 text-audio-primary rounded border border-audio-primary/30">
                    POST
                  </button>
                </div>
              </div>
              <SpectrumAnalyzer 
                fftData={meters.spectrumData}
                width={500}
                height={150}
                colorMode="rainbow"
                showFreqLabels
                showGrid
              />
            </div>
          </div>
        </div>

        {/* Main Controls */}
        <div className="bg-gradient-to-br from-plugin-panel to-plugin-raised rounded-xl p-6 border border-plugin-highlight shadow-[var(--shadow-depth)]">
          <h2 className="text-lg font-semibold text-foreground/80 mb-6 text-center">TONE SCULPTING CONTROLS</h2>
          <div className="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-6 gap-8">
            <ModernKnob 
              value={state.punch} 
              onChange={(v) => updateParameter('punch', v)} 
              min={0} max={1} 
              label="PUNCH" 
              size="lg"
              spectrum={meters.spectrumData ? Array.from(meters.spectrumData.slice(0, 32)) : []}
            />
            <ModernKnob 
              value={state.warmth} 
              onChange={(v) => updateParameter('warmth', v)} 
              min={0} max={1} 
              label="WARMTH" 
              size="lg"
            />
            <ModernKnob 
              value={state.boom} 
              onChange={(v) => updateParameter('boom', v)} 
              min={0} max={1} 
              label="BOOM" 
              size="lg"
            />
            <ModernKnob 
              value={state.mix} 
              onChange={(v) => updateParameter('mix', v)} 
              min={0} max={1} 
              label="MIX" 
              size="lg"
            />
            <ModernKnob 
              value={state.drive} 
              onChange={(v) => updateParameter('drive', v)} 
              min={0} max={12} 
              label="DRIVE" 
              unit="dB"
              size="lg"
            />
            <div className="flex flex-col items-center justify-center gap-4">
              <ToggleButton 
                value={state.texture} 
                onChange={(v) => updateParameter('texture', v)} 
                label="TEXTURE" 
              />
              <p className="text-xs text-foreground/50 text-center">Air + Micro Room</p>
            </div>
          </div>
        </div>

        {/* Bottom Row - Advanced Controls & Metering */}
        <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
          {/* AI Controls */}
          <div className="bg-gradient-to-br from-plugin-panel to-plugin-raised rounded-xl p-4 border border-plugin-highlight shadow-[var(--shadow-depth)]">
            <h3 className="text-sm font-semibold text-foreground/80 mb-4 text-center">AI ENHANCEMENT</h3>
            <div className="space-y-3">
              <ToggleButton 
                value={state.aiEnhance || false} 
                onChange={(v) => updateParameter('aiEnhance', v)} 
                label={state.aiEnhance ? 'NEURAL ON' : 'NEURAL OFF'} 
              />
              <ToggleButton 
                value={state.timbralTransfer || false} 
                onChange={(v) => updateParameter('timbralTransfer', v)} 
                label={state.timbralTransfer ? 'TIMBRAL ON' : 'TIMBRAL OFF'} 
              />
            </div>
          </div>

          {/* Gain Reduction */}
          <div className="bg-gradient-to-br from-plugin-panel to-plugin-raised rounded-xl p-4 border border-plugin-highlight shadow-[var(--shadow-depth)]">
            <h3 className="text-sm font-semibold text-foreground/80 mb-4 text-center">GAIN REDUCTION</h3>
            <div className="flex justify-center">
              <ModernVUMeter 
                level={meters.gainReduction / 20} 
                label="GR" 
                orientation="horizontal"
                size="md"
                showNumeric={false}
              />
            </div>
            <div className="text-center mt-2 text-sm font-mono text-audio-warning">
              -{meters.gainReduction.toFixed(1)} dB
            </div>
          </div>

          {/* LUFS & True Peak */}
          <div className="bg-gradient-to-br from-plugin-panel to-plugin-raised rounded-xl p-4 border border-plugin-highlight shadow-[var(--shadow-depth)]">
            <h3 className="text-sm font-semibold text-foreground/80 mb-4 text-center">LOUDNESS COMPLIANCE</h3>
            <div className="space-y-3">
              <div className="flex justify-between items-center">
                <span className="text-sm text-foreground/70">LUFS Integrated:</span>
                <span className={cn(
                  'text-sm font-mono font-medium',
                  meters.lufsIntegrated > -8 ? 'text-audio-warning' : 'text-audio-success'
                )}>
                  {meters.lufsIntegrated.toFixed(1)}
                </span>
              </div>
              <div className="flex justify-between items-center">
                <span className="text-sm text-foreground/70">True Peak:</span>
                <span className={cn(
                  'text-sm font-mono font-medium',
                  meters.truePeak > -1.0 ? 'text-audio-danger' : 'text-audio-success'
                )}>
                  {meters.truePeak.toFixed(1)} dBTP
                </span>
              </div>
              <div className="text-xs text-foreground/50 text-center mt-2">
                Target: {state.lufsTarget} LUFS • Max: -1.0 dBTP
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};