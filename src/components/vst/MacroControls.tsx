import React, { useState } from 'react';
import { ModernKnob } from './ModernKnob';
import { BTZPluginState } from './types';
import { cn } from '@/lib/utils';

interface MacroControlsProps {
  state: BTZPluginState;
  updateParameter: (key: keyof BTZPluginState, value: any) => void;
}

export const MacroControls: React.FC<MacroControlsProps> = ({
  state,
  updateParameter
}) => {
  const [macros, setMacros] = useState({
    macro1: 0.5,
    macro2: 0.3,
    macro3: 0.7,
    macro4: 0.2
  });

  const [assignments, setAssignments] = useState({
    macro1: ['punch', 'drive'],
    macro2: ['warmth', 'boom'],
    macro3: ['mix', 'texture'],
    macro4: ['drive', 'boom']
  });

  const handleMacroChange = (macroId: keyof typeof macros, value: number) => {
    setMacros(prev => ({ ...prev, [macroId]: value }));
    
    // Apply macro to assigned parameters
    const assigned = assignments[macroId];
    assigned.forEach(param => {
      if (param === 'texture') {
        updateParameter('texture', value > 0.5);
      } else {
        const currentValue = state[param as keyof BTZPluginState] as number;
        const modulated = currentValue + (value - 0.5) * 0.3;
        updateParameter(param as keyof BTZPluginState, Math.max(0, Math.min(1, modulated)));
      }
    });
  };

  const MACRO_COLORS = [
    'hsl(var(--audio-primary))',
    'hsl(var(--audio-secondary))',
    'hsl(var(--audio-tertiary))',
    'hsl(var(--audio-success))'
  ];

  return (
    <div className="bg-plugin-panel rounded-2xl p-6 border border-audio-primary/10">
      <h3 className="text-lg font-bold text-center text-foreground mb-6 tracking-wide">
        MACRO CONTROLS
      </h3>
      
      {/* Macro Knobs */}
      <div className="grid grid-cols-2 lg:grid-cols-4 gap-6 mb-8">
        {Object.entries(macros).map(([macroId, value], index) => (
          <div key={macroId} className="flex flex-col items-center">
            <ModernKnob 
              value={value}
              onChange={(v) => handleMacroChange(macroId as keyof typeof macros, v)}
              label={`MACRO ${index + 1}`}
              min={0}
              max={1}
              size="lg"
              spectrum={[]}
            />
            
            {/* Assignment Display */}
            <div className="mt-3 space-y-1">
              {assignments[macroId as keyof typeof assignments].map((param) => (
                <div 
                  key={param}
                  className="px-2 py-1 bg-plugin-surface rounded text-xs text-center font-medium"
                  style={{ 
                    color: MACRO_COLORS[index],
                    border: `1px solid ${MACRO_COLORS[index]}40`
                  }}
                >
                  {param.toUpperCase()}
                </div>
              ))}
            </div>
          </div>
        ))}
      </div>

      {/* Macro Assignment Matrix */}
      <div className="space-y-4">
        <h4 className="text-sm font-bold text-foreground/80 uppercase tracking-wide text-center">
          Assignment Matrix
        </h4>
        
        <div className="grid grid-cols-4 gap-4">
          {Object.keys(macros).map((macroId, index) => (
            <div key={macroId} className="bg-plugin-surface rounded-lg p-4">
              <div 
                className="text-sm font-bold text-center mb-3"
                style={{ color: MACRO_COLORS[index] }}
              >
                {macroId.toUpperCase()}
              </div>
              
              <div className="space-y-2">
                {['punch', 'warmth', 'boom', 'mix', 'drive', 'texture'].map((param) => {
                  const isAssigned = assignments[macroId as keyof typeof assignments].includes(param);
                  
                  return (
                    <label key={param} className="flex items-center gap-2">
                      <input
                        type="checkbox"
                        checked={isAssigned}
                        onChange={(e) => {
                          setAssignments(prev => ({
                            ...prev,
                            [macroId]: e.target.checked
                              ? [...prev[macroId as keyof typeof assignments], param]
                              : prev[macroId as keyof typeof assignments].filter(p => p !== param)
                          }));
                        }}
                        className="accent-audio-primary"
                      />
                      <span className="text-xs text-foreground/70 uppercase">
                        {param}
                      </span>
                    </label>
                  );
                })}
              </div>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
};