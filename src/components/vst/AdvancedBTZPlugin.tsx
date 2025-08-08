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

  const [showPresets, setShowPresets] = useState(false);

  return (
    <div className="w-full max-w-5xl mx-auto rounded-3xl border border-plugin-raised/50 overflow-hidden backdrop-blur-sm"
         style={{ 
           background: 'var(--gradient-main)',
           boxShadow: 'var(--shadow-panel)'
         }}>
      
      {/* Header */}
      <div className="relative border-b border-plugin-raised/30 p-6">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-6">
            <h1 className="text-5xl font-bold text-foreground tracking-wider" 
                style={{ textShadow: '0 0 20px hsl(var(--audio-primary) / 0.3)' }}>
              BTZ
            </h1>
            <div className="text-right">
              <h2 className="text-xl font-medium text-foreground/90 tracking-wide">BOX TONE ZONE</h2>
              <h3 className="text-sm text-foreground/70 tracking-widest">ENHANCER</h3>
            </div>
          </div>
          
          {/* Status Controls */}
          <div className="flex items-center gap-3">
            <ToggleButton 
              value={state.active} 
              onChange={(v) => updateParameter('active', v)} 
              label="POWER"
              className={cn(
                "text-xs px-6 py-3 rounded-lg border transition-all duration-300",
                state.active 
                  ? "bg-audio-primary border-audio-primary text-background font-bold" 
                  : "bg-plugin-raised/50 border-plugin-raised hover:bg-plugin-raised text-foreground/70"
              )}
            />
          </div>
        </div>
        
        {/* Gradient separator */}
        <div className="absolute bottom-0 left-6 right-6 h-px bg-gradient-to-r from-transparent via-audio-primary/30 to-transparent"></div>
      </div>

      {/* Main Content */}
      <div className="p-8 space-y-12">
        
        {/* Main Controls - Clean 5-knob layout matching reference */}
        <section className="space-y-12">
          {/* Top Row - 5 Main Knobs */}
          <div className="grid grid-cols-5 gap-8 justify-items-center">
            <ModernKnob 
              value={state.punch} 
              onChange={(v) => updateParameter('punch', v)} 
              label="PUNCH" 
              min={0} 
              max={1} 
              size="xl" 
            />
            <ModernKnob 
              value={state.warmth} 
              onChange={(v) => updateParameter('warmth', v)} 
              label="WARMTH" 
              min={0} 
              max={1} 
              size="xl" 
            />
            <ModernKnob 
              value={state.boom} 
              onChange={(v) => updateParameter('boom', v)} 
              label="BOOM" 
              min={0} 
              max={1} 
              size="xl" 
            />
            <ModernKnob 
              value={state.mix} 
              onChange={(v) => updateParameter('mix', v)} 
              label="MIX" 
              min={0} 
              max={1} 
              size="xl" 
            />
            <ModernKnob 
              value={state.drive} 
              onChange={(v) => updateParameter('drive', v)} 
              label="DRIVE" 
              min={0} 
              max={12} 
              unit="dB" 
              size="xl" 
            />
          </div>
          
          {/* Second Row - Drive & Texture (matching reference layout) */}
          <div className="grid grid-cols-5 gap-8 justify-items-center items-center">
            <div className="col-start-3 flex flex-col items-center">
              <ModernKnob 
                value={state.drive * 0.8} 
                onChange={(v) => updateParameter('drive', v / 0.8)} 
                label="DRIVE" 
                min={0} 
                max={15} 
                unit="dB" 
                size="lg" 
              />
            </div>
            <div className="col-start-5 flex flex-col items-center">
              <ToggleButton 
                value={state.texture} 
                onChange={(v) => updateParameter('texture', v)} 
                label="TEXTURE"
                className={cn(
                  "text-sm px-10 py-4 rounded-lg border-2 transition-all duration-300 font-bold tracking-wide",
                  state.texture 
                    ? "bg-audio-tertiary border-audio-tertiary text-background" 
                    : "bg-plugin-raised/50 border-plugin-raised hover:bg-plugin-raised text-foreground/70"
                )}
              />
            </div>
          </div>
        </section>

        {/* Separator Line with gradient glow */}
        <div className="h-px bg-gradient-to-r from-transparent via-audio-primary/50 to-transparent"
             style={{ boxShadow: '0 0 10px hsl(var(--audio-primary) / 0.3)' }}>
        </div>

        {/* Output Meter Section - Clean horizontal layout */}
        <section className="space-y-4">
          <div className="flex items-center justify-between">
            <h3 className="text-lg font-bold text-foreground/90 tracking-wide">OUTPUT</h3>
            <div className="text-sm text-foreground/60 font-mono">
              LUFS: <span className="text-audio-primary">{meters.lufsIntegrated.toFixed(1)}</span> | 
              Peak: <span className="text-audio-secondary">{meters.truePeak > 0 ? '+' : ''}{meters.truePeak.toFixed(1)}dB</span>
            </div>
          </div>
          
          {/* Clean Horizontal VU Meter */}
          <div className="rounded-xl p-6 border border-plugin-raised/30"
               style={{ 
                 background: 'var(--gradient-panel)',
                 boxShadow: 'inset 0 2px 8px hsl(var(--plugin-surface))'
               }}>
            <div className="h-4 bg-plugin-surface rounded-full overflow-hidden relative">
              {/* VU Meter bars simulation */}
              {Array.from({ length: 32 }).map((_, i) => (
                <div
                  key={i}
                  className="absolute top-0 h-full transition-all duration-100"
                  style={{
                    left: `${(i / 32) * 100}%`,
                    width: '2.5%',
                    background: meters.outputLevel * 32 > i 
                      ? `hsl(${120 - (i / 32) * 120}, 80%, 50%)` 
                      : 'hsl(var(--plugin-surface))',
                    marginRight: '0.5%',
                    borderRadius: '2px',
                    boxShadow: meters.outputLevel * 32 > i 
                      ? `0 0 4px hsl(${120 - (i / 32) * 120}, 80%, 50%)` 
                      : 'none'
                  }}
                />
              ))}
            </div>
          </div>
        </section>

        {/* Advanced Toggle */}
        <div className="flex justify-center">
          <button
            onClick={() => setShowPresets(!showPresets)}
            className="text-xs text-foreground/50 hover:text-audio-primary transition-colors tracking-widest uppercase font-medium"
          >
            {showPresets ? 'HIDE ADVANCED' : 'SHOW ADVANCED'}
          </button>
        </div>

        {/* Advanced Controls - Collapsible */}
        {showPresets && (
          <section className="rounded-xl p-6 border border-plugin-raised/30 backdrop-blur-sm animate-fade-in"
                   style={{ 
                     background: 'var(--gradient-panel)',
                     boxShadow: 'var(--shadow-panel)'
                   }}>
            <div className="grid grid-cols-1 lg:grid-cols-2 gap-8">
              {/* AI Controls */}
              <div className="space-y-6">
                <h3 className="text-lg font-bold text-center text-foreground/90 tracking-wide">AI ENGINE</h3>
                <div className="space-y-4">
                  <ToggleButton 
                    value={!!state.aiEnhance} 
                    onChange={(v) => updateParameter('aiEnhance', v)} 
                    label="Neural Enhancement"
                    className={cn(
                      "w-full text-sm py-3 rounded-lg border transition-all duration-300",
                      state.aiEnhance 
                        ? "bg-audio-secondary border-audio-secondary text-background font-bold" 
                        : "bg-plugin-raised/50 border-plugin-raised hover:bg-plugin-raised text-foreground/70"
                    )}
                  />
                  <ToggleButton 
                    value={!!state.timbralTransfer} 
                    onChange={(v) => updateParameter('timbralTransfer', v)} 
                    label="Timbral Transfer"
                    className={cn(
                      "w-full text-sm py-3 rounded-lg border transition-all duration-300",
                      state.timbralTransfer 
                        ? "bg-audio-tertiary border-audio-tertiary text-background font-bold" 
                        : "bg-plugin-raised/50 border-plugin-raised hover:bg-plugin-raised text-foreground/70"
                    )}
                  />
                  <ToggleButton 
                    value={state.oversampling} 
                    onChange={(v) => updateParameter('oversampling', v)} 
                    label={`Oversampling ${state.oversampling ? '8x' : 'OFF'}`}
                    className={cn(
                      "w-full text-sm py-3 rounded-lg border transition-all duration-300",
                      state.oversampling 
                        ? "bg-audio-success border-audio-success text-background font-bold" 
                        : "bg-plugin-raised/50 border-plugin-raised hover:bg-plugin-raised text-foreground/70"
                    )}
                  />
                </div>
              </div>
              
              {/* Spectrum Analyzer */}
              <div className="space-y-4">
                <h3 className="text-lg font-bold text-center text-foreground/90 tracking-wide">SPECTRUM</h3>
                <div className="h-32 rounded-lg overflow-hidden"
                     style={{ 
                       background: 'var(--gradient-panel)',
                       border: '1px solid hsl(var(--plugin-raised))'
                     }}>
                  <SpectrumAnalyzer 
                    fftData={meters.spectrumData}
                    height={120}
                    showGrid={false}
                  />
                </div>
              </div>
            </div>
          </section>
        )}
      </div>
    </div>
  );
};