import React, { useState, useEffect, useCallback } from 'react';
import { ModernKnobWithSpectrum } from './ModernKnobWithSpectrum';
import { ToggleButton } from './ToggleButton';
import { CentralVisualizer } from './CentralVisualizer';
import { ClippingControls } from './ClippingControls';
import { PresetBrowser } from './PresetBrowser';
import { AdvancedView } from './AdvancedView';
import { BTZPluginState, EnhancedPreset } from './types';
import { cn } from '@/lib/utils';

const DEFAULT_PRESET: EnhancedPreset = {
  id: 'default',
  label: 'Default',
  state: {
    punch: 0,
    warmth: 0,
    boom: 0,
    mix: 1.0,
    drive: 0,
    texture: false,
    active: true,
    oversampling: false,
    clippingType: 'soft',
    clippingBlend: 0.5,
    lufsTarget: -14,
    aiEnhance: false,
    timbralTransfer: false,
    oversamplingRate: 4
  }
};

const PERFORMANCE_PRESETS: EnhancedPreset[] = [
  DEFAULT_PRESET,
  {
    id: 'punchy-kick',
    label: 'Punchy Kick',
    state: { punch: 0.8, warmth: 0.35, boom: 0.6, mix: 0.9, drive: 0.4, texture: false, active: true, oversampling: false, clippingType: 'hard', clippingBlend: 0.3 },
  },
  {
    id: 'warm-vocal', 
    label: 'Warm Vocal',
    state: { punch: 0.4, warmth: 0.8, boom: 0.2, mix: 0.95, drive: 0.3, texture: true, active: true, oversampling: false, clippingType: 'tube', clippingBlend: 0.6 },
  },
  {
    id: 'modern-master',
    label: 'Modern Master', 
    state: { punch: 0.7, warmth: 0.5, boom: 0.4, mix: 0.9, drive: 0.6, texture: true, active: true, oversampling: false, clippingType: 'digital', clippingBlend: 0.4 },
  },
  {
    id: 'vintage-glue',
    label: 'Vintage Glue',
    state: { punch: 0.5, warmth: 0.9, boom: 0.7, mix: 0.85, drive: 0.8, texture: true, active: true, oversampling: false, clippingType: 'tape', clippingBlend: 0.8 },
  }
];

export const EnhancedBTZPlugin: React.FC = () => {
  const [state, setState] = useState<BTZPluginState>(DEFAULT_PRESET.state);
  const [viewMode, setViewMode] = useState<'primary' | 'advanced'>('primary');
  const [meters, setMeters] = useState({
    inputLevel: 0,
    outputLevel: 0,
    spectrumData: new Float32Array(64),
    waveformData: new Float32Array(128),
    lufsIntegrated: -14.2,
    truePeak: -2.1,
    isProcessing: false
  });

  // Generate live audio visualization data
  useEffect(() => {
    const interval = setInterval(() => {
      if (!state.active) {
        setMeters(prev => ({ ...prev, isProcessing: false }));
        return;
      }

      const baseLevel = 0.2 + Math.random() * 0.5;
      const processedLevel = baseLevel * (1 + state.drive * 0.8) * state.mix;
      
      // Generate realistic spectrum for each processing stage
      const spectrumData = new Float32Array(64);
      const waveformData = new Float32Array(128);
      
      for (let i = 0; i < 64; i++) {
        const freq = (i / 64) * 20000;
        let magnitude = Math.random() * 0.1;
        
        // Shape spectrum based on controls
        if (freq < 200) magnitude += state.boom * 0.6;
        if (freq > 80 && freq < 800) magnitude += state.punch * 0.5;
        if (freq > 1000 && freq < 8000) magnitude += state.warmth * 0.4;
        if (freq > 8000) magnitude += state.texture ? 0.3 : 0.1;
        
        spectrumData[i] = Math.max(0, Math.min(1, magnitude * (1 + state.drive * 0.3)));
      }
      
      // Generate waveform
      for (let i = 0; i < 128; i++) {
        const t = (i / 128) * Math.PI * 4;
        waveformData[i] = Math.sin(t) * processedLevel * (0.8 + Math.random() * 0.4);
      }

      setMeters({
        inputLevel: baseLevel,
        outputLevel: processedLevel,
        spectrumData,
        waveformData,
        lufsIntegrated: state.lufsTarget + (Math.random() - 0.5) * 1.5,
        truePeak: Math.max(-1.0, state.lufsTarget + 10 + (Math.random() - 0.5) * 2),
        isProcessing: processedLevel > 0.1
      });
    }, 60);

    return () => clearInterval(interval);
  }, [state]);

  const updateParameter = useCallback((key: keyof BTZPluginState, value: any) => {
    setState(prev => ({ ...prev, [key]: value }));
  }, []);

  const applyPreset = useCallback((preset: EnhancedPreset) => {
    setState(preset.state);
  }, []);

  return (
    <div className="w-full max-w-7xl mx-auto rounded-3xl border border-audio-primary/20 overflow-hidden"
         style={{ 
           background: 'var(--gradient-main)',
           boxShadow: 'var(--shadow-panel), 0 0 60px hsl(var(--audio-primary) / 0.1)'
         }}>
      
      {/* Header with view toggle */}
      <div className="relative border-b border-audio-primary/20 p-6 bg-gradient-to-r from-plugin-surface/50 to-plugin-panel/50">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-8">
            <div className="relative">
              <h1 className="text-6xl font-black text-transparent bg-gradient-to-r from-audio-primary to-audio-secondary bg-clip-text tracking-wider"
                  style={{ 
                    filter: 'drop-shadow(0 0 20px hsl(var(--audio-primary) / 0.5))',
                    fontFamily: 'var(--font-display)'
                  }}>
                BTZ
              </h1>
              <div className="absolute -top-2 -right-2 w-3 h-3 rounded-full bg-audio-primary animate-pulse"
                   style={{ boxShadow: '0 0 10px hsl(var(--audio-primary))' }}></div>
            </div>
            <div>
              <h2 className="text-2xl font-bold text-foreground tracking-wide">BOX TONE ZONE</h2>
              <h3 className="text-sm text-foreground/70 tracking-[0.3em] uppercase">Enhanced Audio Processor</h3>
            </div>
          </div>
          
          <div className="flex items-center gap-4">
            {/* View Mode Toggle */}
            <div className="flex bg-plugin-surface rounded-full p-1 border border-audio-primary/20">
              <button
                onClick={() => setViewMode('primary')}
                className={cn(
                  "px-6 py-2 rounded-full text-sm font-medium transition-all duration-300",
                  viewMode === 'primary'
                    ? "bg-audio-primary text-background shadow-lg"
                    : "text-foreground/70 hover:text-foreground"
                )}
              >
                PERFORMANCE
              </button>
              <button
                onClick={() => setViewMode('advanced')}
                className={cn(
                  "px-6 py-2 rounded-full text-sm font-medium transition-all duration-300",
                  viewMode === 'advanced'
                    ? "bg-audio-secondary text-background shadow-lg"
                    : "text-foreground/70 hover:text-foreground"
                )}
              >
                SOUND DESIGN
              </button>
            </div>
            
            {/* Power Button */}
            <ToggleButton 
              value={state.active} 
              onChange={(v) => updateParameter('active', v)} 
              label="POWER"
              className={cn(
                "px-8 py-3 rounded-full border-2 font-bold transition-all duration-300",
                state.active 
                  ? "bg-audio-primary border-audio-primary text-background shadow-[0_0_20px_hsl(var(--audio-primary))]" 
                  : "bg-plugin-raised/50 border-plugin-raised hover:bg-plugin-raised text-foreground/70"
              )}
            />
          </div>
        </div>
      </div>

      {/* Content Area */}
      <div className="p-8">
        {viewMode === 'primary' ? (
          // PRIMARY VIEW - Performance Mode
          <div className="space-y-12">
            {/* Central Visualizer */}
            <div className="flex justify-center">
              <CentralVisualizer 
                spectrumData={meters.spectrumData}
                waveformData={meters.waveformData}
                isProcessing={meters.isProcessing}
                level={meters.outputLevel}
              />
            </div>

            {/* Main Controls - 5 Knobs with live visualization */}
            <div className="grid grid-cols-5 gap-12 justify-items-center">
              <ModernKnobWithSpectrum 
                value={state.punch} 
                onChange={(v) => updateParameter('punch', v)} 
                label="PUNCH" 
                min={0} 
                max={1}
                size="xl"
                spectrumData={meters.spectrumData.slice(0, 16)}
                color="hsl(var(--audio-primary))"
              />
              <ModernKnobWithSpectrum 
                value={state.warmth} 
                onChange={(v) => updateParameter('warmth', v)} 
                label="WARMTH" 
                min={0} 
                max={1}
                size="xl"
                spectrumData={meters.spectrumData.slice(16, 32)}
                color="hsl(var(--audio-secondary))"
              />
              <ModernKnobWithSpectrum 
                value={state.boom} 
                onChange={(v) => updateParameter('boom', v)} 
                label="BOOM" 
                min={0} 
                max={1}
                size="xl"
                spectrumData={meters.spectrumData.slice(0, 8)}
                color="hsl(var(--audio-tertiary))"
              />
              <ModernKnobWithSpectrum 
                value={state.mix} 
                onChange={(v) => updateParameter('mix', v)} 
                label="MIX" 
                min={0} 
                max={1}
                size="xl"
                spectrumData={meters.spectrumData}
                color="hsl(var(--audio-success))"
              />
              <ModernKnobWithSpectrum 
              value={state.drive} 
              onChange={(v) => updateParameter('drive', v)} 
              label="DRIVE" 
              min={0} 
              max={1}
              size="xl"
              spectrumData={meters.spectrumData.slice(32, 48)}
              color="hsl(var(--audio-warning))"
              />
            </div>

            {/* Texture & Clipping Controls */}
            <div className="flex justify-center items-center gap-12">
              <ToggleButton 
                value={state.texture} 
                onChange={(v) => updateParameter('texture', v)} 
                label="TEXTURE"
                className={cn(
                  "px-12 py-4 rounded-2xl border-2 font-bold text-lg transition-all duration-300",
                  state.texture 
                    ? "bg-audio-tertiary border-audio-tertiary text-background shadow-[0_0_30px_hsl(var(--audio-tertiary))]" 
                    : "bg-plugin-raised/50 border-plugin-raised hover:bg-plugin-raised text-foreground/70"
                )}
              />
              
              <ClippingControls 
                type={state.clippingType || 'soft'}
                blend={state.clippingBlend || 0.5}
                onTypeChange={(type) => updateParameter('clippingType', type)}
                onBlendChange={(blend) => updateParameter('clippingBlend', blend)}
              />
            </div>

            {/* Presets */}
            <div className="flex justify-center">
              <PresetBrowser 
                presets={PERFORMANCE_PRESETS}
                onApplyPreset={applyPreset}
              />
            </div>

            {/* Output Metering */}
            <div className="bg-plugin-panel rounded-2xl p-6 border border-audio-primary/10">
              <div className="flex items-center justify-between mb-4">
                <h3 className="text-lg font-bold text-foreground tracking-wide">OUTPUT ANALYSIS</h3>
                <div className="flex gap-6 text-sm font-mono">
                  <span>LUFS: <span className="text-audio-primary font-bold">{meters.lufsIntegrated.toFixed(1)}</span></span>
                  <span>Peak: <span className="text-audio-secondary font-bold">{meters.truePeak > 0 ? '+' : ''}{meters.truePeak.toFixed(1)}dB</span></span>
                </div>
              </div>
              
              {/* Waveform Display */}
              <div className="h-20 bg-plugin-surface rounded-lg overflow-hidden relative">
                <svg className="w-full h-full">
                  <defs>
                    <linearGradient id="waveformGrad" x1="0%" y1="0%" x2="100%" y2="0%">
                      <stop offset="0%" stopColor="hsl(var(--audio-primary))" stopOpacity="0.8"/>
                      <stop offset="50%" stopColor="hsl(var(--audio-secondary))" stopOpacity="0.9"/>
                      <stop offset="100%" stopColor="hsl(var(--audio-tertiary))" stopOpacity="0.8"/>
                    </linearGradient>
                  </defs>
                  <polyline
                    points={meters.waveformData.map((value, i) => 
                      `${(i / meters.waveformData.length) * 100},${50 + value * 30}`
                    ).join(' ')}
                    fill="none"
                    stroke="url(#waveformGrad)"
                    strokeWidth="2"
                    style={{ filter: 'drop-shadow(0 0 4px hsl(var(--audio-primary) / 0.5))' }}
                  />
                </svg>
              </div>
            </div>
          </div>
        ) : (
          // ADVANCED VIEW - Sound Design Mode
          <AdvancedView 
            state={state}
            updateParameter={updateParameter}
            meters={meters}
            onApplyPreset={applyPreset}
            presets={PERFORMANCE_PRESETS}
          />
        )}
      </div>
    </div>
  );
};