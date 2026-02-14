import React, { useState } from 'react';
import { ModulationMatrix } from './ModulationMatrix';
import { EffectStages } from './EffectStages';
import { LFOPanel } from './LFOPanel';
import { EnvelopePanel } from './EnvelopePanel';
import { StepSequencer } from './StepSequencer';
import { MacroControls } from './MacroControls';
import { PresetMorpher } from './PresetMorpher';
import { BTZPluginState, EnhancedPreset } from './types';
import { cn } from '@/lib/utils';

interface AdvancedViewProps {
  state: BTZPluginState;
  updateParameter: (key: keyof BTZPluginState, value: any) => void;
  meters: any;
  onApplyPreset: (preset: EnhancedPreset) => void;
  presets: EnhancedPreset[];
}

export const AdvancedView: React.FC<AdvancedViewProps> = ({
  state,
  updateParameter,
  meters,
  onApplyPreset,
  presets
}) => {
  const [activeTab, setActiveTab] = useState<'stages' | 'modulation' | 'sequencing' | 'morphing'>('stages');

  const tabs = [
    { id: 'stages', label: 'EFFECT STAGES', icon: '⚡' },
    { id: 'modulation', label: 'MODULATION', icon: '〰️' },
    { id: 'sequencing', label: 'SEQUENCING', icon: '▣' },
    { id: 'morphing', label: 'MORPHING', icon: '∞' }
  ] as const;

  return (
    <div className="space-y-8">
      {/* Tab Navigation */}
      <div className="flex justify-center">
        <div className="flex bg-plugin-surface rounded-2xl p-2 border border-audio-primary/20">
          {tabs.map((tab) => (
            <button
              key={tab.id}
              onClick={() => setActiveTab(tab.id)}
              className={cn(
                "px-6 py-3 rounded-xl font-bold text-sm transition-all duration-300 flex items-center gap-2",
                activeTab === tab.id
                  ? "bg-audio-primary text-background shadow-lg transform scale-105"
                  : "text-foreground/70 hover:text-foreground hover:bg-plugin-raised/50"
              )}
              style={activeTab === tab.id ? {
                boxShadow: '0 0 20px hsl(var(--audio-primary) / 0.4)'
              } : {}}
            >
              <span>{tab.icon}</span>
              {tab.label}
            </button>
          ))}
        </div>
      </div>

      {/* Tab Content */}
      <div className="min-h-[600px]">
        {activeTab === 'stages' && (
          <div className="space-y-8">
            {/* Effect Stages */}
            <EffectStages 
              state={state}
              updateParameter={updateParameter}
              meters={meters}
            />
            
            {/* Macro Controls */}
            <MacroControls 
              state={state}
              updateParameter={updateParameter}
            />
          </div>
        )}

        {activeTab === 'modulation' && (
          <div className="grid grid-cols-1 xl:grid-cols-3 gap-8">
            {/* Modulation Matrix */}
            <div className="xl:col-span-2">
              <ModulationMatrix 
                state={state}
                updateParameter={updateParameter}
              />
            </div>
            
            {/* LFO Panel */}
            <div className="space-y-6">
              <LFOPanel />
              <EnvelopePanel />
            </div>
          </div>
        )}

        {activeTab === 'sequencing' && (
          <div className="space-y-8">
            {/* Step Sequencer */}
            <StepSequencer />
            
            {/* Parameter Automation */}
            <div className="bg-plugin-panel rounded-2xl p-6 border border-audio-primary/10">
              <h3 className="text-lg font-bold text-center text-foreground mb-6 tracking-wide">
                PARAMETER AUTOMATION
              </h3>
              <div className="grid grid-cols-2 lg:grid-cols-4 gap-4">
                {['punch', 'warmth', 'boom', 'drive'].map((param) => (
                  <div key={param} className="bg-plugin-surface rounded-lg p-4">
                    <div className="text-xs font-bold text-center text-foreground/80 mb-2 uppercase tracking-wide">
                      {param}
                    </div>
                    <div className="h-16 bg-plugin-raised rounded relative overflow-hidden">
                      {/* Mini automation curve */}
                      <svg className="w-full h-full">
                        <path
                          d="M 0 50 Q 25 20 50 40 Q 75 10 100 30"
                          fill="none"
                          stroke="hsl(var(--audio-primary))"
                          strokeWidth="2"
                          style={{
                            filter: 'drop-shadow(0 0 4px hsl(var(--audio-primary)))'
                          }}
                        />
                      </svg>
                    </div>
                  </div>
                ))}
              </div>
            </div>
          </div>
        )}

        {activeTab === 'morphing' && (
          <PresetMorpher 
            presets={presets}
            currentState={state}
            onApplyPreset={onApplyPreset}
            updateParameter={updateParameter}
          />
        )}
      </div>
    </div>
  );
};