import React, { useState } from 'react';
import { EnhancedPreset, BTZPluginState } from './types';
import { ModernKnob } from './ModernKnob';
import { cn } from '@/lib/utils';

interface PresetMorpherProps {
  presets: EnhancedPreset[];
  currentState: BTZPluginState;
  onApplyPreset: (preset: EnhancedPreset) => void;
  updateParameter: (key: keyof BTZPluginState, value: any) => void;
}

export const PresetMorpher: React.FC<PresetMorpherProps> = ({
  presets,
  currentState,
  onApplyPreset,
  updateParameter
}) => {
  const [presetA, setPresetA] = useState<EnhancedPreset | null>(presets[0]);
  const [presetB, setPresetB] = useState<EnhancedPreset | null>(presets[1]);
  const [morphAmount, setMorphAmount] = useState(0.5);
  const [lockedParams, setLockedParams] = useState<Set<keyof BTZPluginState>>(new Set());

  const handleMorph = () => {
    if (!presetA || !presetB) return;

    const morphedState: Record<string, any> = {};
    
    Object.keys(presetA.state).forEach(key => {
      const paramKey = key as keyof BTZPluginState;
      
      // Skip locked parameters
      if (lockedParams.has(paramKey)) return;
      
      const valueA = presetA.state[paramKey];
      const valueB = presetB.state[paramKey];
      
      if (typeof valueA === 'number' && typeof valueB === 'number') {
        morphedState[paramKey] = valueA + (valueB - valueA) * morphAmount;
      } else if (typeof valueA === 'boolean' && typeof valueB === 'boolean') {
        morphedState[paramKey] = morphAmount > 0.5 ? valueB : valueA;
      } else {
        morphedState[paramKey] = morphAmount > 0.5 ? valueB : valueA;
      }
    });

    Object.entries(morphedState).forEach(([key, value]) => {
      updateParameter(key as keyof BTZPluginState, value);
    });
  };

  const randomizePreset = () => {
    const randomState: Partial<BTZPluginState> = {};
    
    (['punch', 'warmth', 'boom', 'mix', 'drive'] as const).forEach(param => {
      if (!lockedParams.has(param)) {
        randomState[param] = Math.random();
      }
    });
    
    if (!lockedParams.has('texture')) {
      randomState.texture = Math.random() > 0.5;
    }

    Object.entries(randomState).forEach(([key, value]) => {
      updateParameter(key as keyof BTZPluginState, value);
    });
  };

  const toggleParameterLock = (param: keyof BTZPluginState) => {
    setLockedParams(prev => {
      const newSet = new Set(prev);
      if (newSet.has(param)) {
        newSet.delete(param);
      } else {
        newSet.add(param);
      }
      return newSet;
    });
  };

  return (
    <div className="space-y-8">
      {/* Preset Morphing */}
      <div className="bg-plugin-panel rounded-2xl p-6 border border-audio-primary/10">
        <h3 className="text-lg font-bold text-center text-foreground mb-6 tracking-wide">
          PRESET MORPHING
        </h3>
        
        <div className="grid grid-cols-1 lg:grid-cols-3 gap-6 items-center">
          {/* Preset A */}
          <div className="space-y-4">
            <h4 className="text-sm font-bold text-center text-audio-primary uppercase tracking-wide">
              PRESET A
            </h4>
            <select
              value={presetA?.id || ''}
              onChange={(e) => setPresetA(presets.find(p => p.id === e.target.value) || null)}
              className="w-full p-3 bg-plugin-surface border border-audio-primary/20 rounded-lg text-foreground"
            >
              <option value="">Select Preset A</option>
              {presets.map(preset => (
                <option key={preset.id} value={preset.id}>
                  {preset.label}
                </option>
              ))}
            </select>
            {presetA && (
              <button
                onClick={() => onApplyPreset(presetA)}
                className="w-full px-4 py-2 bg-audio-primary text-background rounded-lg font-bold hover:bg-audio-primary/80 transition-colors"
              >
                LOAD A
              </button>
            )}
          </div>

          {/* Morph Control */}
          <div className="flex flex-col items-center space-y-4">
            <ModernKnob 
              value={morphAmount}
              onChange={setMorphAmount}
              label="MORPH"
              min={0}
              max={1}
              size="lg"
              spectrum={[]}
            />
            <button
              onClick={handleMorph}
              disabled={!presetA || !presetB}
              className={cn(
                "px-6 py-3 rounded-lg font-bold text-sm transition-all duration-300",
                presetA && presetB
                  ? "bg-audio-secondary text-background hover:bg-audio-secondary/80"
                  : "bg-plugin-raised text-foreground/50 cursor-not-allowed"
              )}
            >
              APPLY MORPH
            </button>
          </div>

          {/* Preset B */}
          <div className="space-y-4">
            <h4 className="text-sm font-bold text-center text-audio-tertiary uppercase tracking-wide">
              PRESET B
            </h4>
            <select
              value={presetB?.id || ''}
              onChange={(e) => setPresetB(presets.find(p => p.id === e.target.value) || null)}
              className="w-full p-3 bg-plugin-surface border border-audio-tertiary/20 rounded-lg text-foreground"
            >
              <option value="">Select Preset B</option>
              {presets.map(preset => (
                <option key={preset.id} value={preset.id}>
                  {preset.label}
                </option>
              ))}
            </select>
            {presetB && (
              <button
                onClick={() => onApplyPreset(presetB)}
                className="w-full px-4 py-2 bg-audio-tertiary text-background rounded-lg font-bold hover:bg-audio-tertiary/80 transition-colors"
              >
                LOAD B
              </button>
            )}
          </div>
        </div>
      </div>

      {/* Parameter Locks */}
      <div className="bg-plugin-panel rounded-2xl p-6 border border-audio-primary/10">
        <h3 className="text-lg font-bold text-center text-foreground mb-6 tracking-wide">
          PARAMETER LOCKS
        </h3>
        
        <div className="grid grid-cols-3 lg:grid-cols-6 gap-4 mb-6">
          {(['punch', 'warmth', 'boom', 'mix', 'drive', 'texture'] as const).map(param => (
            <button
              key={param}
              onClick={() => toggleParameterLock(param)}
              className={cn(
                "px-4 py-3 rounded-lg font-bold text-xs transition-all duration-300 border-2",
                lockedParams.has(param)
                  ? "bg-audio-warning border-audio-warning text-background"
                  : "bg-plugin-surface border-plugin-raised hover:border-audio-primary/50 text-foreground/70"
              )}
            >
              🔒 {param.toUpperCase()}
            </button>
          ))}
        </div>
        
        <div className="flex justify-center gap-4">
          <button
            onClick={() => setLockedParams(new Set())}
            className="px-6 py-2 bg-plugin-raised text-foreground rounded-lg font-bold hover:bg-plugin-raised/80 transition-colors"
          >
            UNLOCK ALL
          </button>
          <button
            onClick={randomizePreset}
            className="px-6 py-2 bg-audio-success text-background rounded-lg font-bold hover:bg-audio-success/80 transition-colors"
          >
            RANDOMIZE
          </button>
        </div>
      </div>
    </div>
  );
};