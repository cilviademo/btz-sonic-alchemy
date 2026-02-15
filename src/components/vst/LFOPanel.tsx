import React, { useState } from 'react';
import { ModernKnob } from './ModernKnob';
import { cn } from '@/lib/utils';

const LFO_WAVEFORMS = [
  { id: 'sine', label: 'Sine', path: 'M 0 30 Q 25 10 50 30 Q 75 50 100 30' },
  { id: 'saw', label: 'Saw', path: 'M 0 50 L 50 10 L 50 50 L 100 10' },
  { id: 'square', label: 'Square', path: 'M 0 10 L 25 10 L 25 50 L 50 50 L 50 10 L 75 10 L 75 50 L 100 50' },
  { id: 'random', label: 'Random', path: 'M 0 30 L 10 20 L 20 40 L 30 15 L 40 45 L 50 25 L 60 35 L 70 10 L 80 50 L 90 30 L 100 25' }
];

export const LFOPanel: React.FC = () => {
  const [lfo1, setLfo1] = useState({
    rate: 0.5,
    depth: 0.3,
    waveform: 'sine',
    sync: false
  });

  const [lfo2, setLfo2] = useState({
    rate: 0.8,
    depth: 0.2,
    waveform: 'saw',
    sync: true
  });

  const LFOControl = ({ 
    lfo, 
    setLfo, 
    label, 
    color 
  }: { 
    lfo: typeof lfo1; 
    setLfo: typeof setLfo1; 
    label: string;
    color: string;
  }) => (
    <div className="bg-plugin-surface rounded-xl p-4 border border-audio-primary/10">
      <div className="flex items-center justify-between mb-4">
        <h4 className="text-sm font-bold text-foreground tracking-wide">{label}</h4>
        <div 
          className="w-3 h-3 rounded-full animate-pulse"
          style={{ background: color }}
        />
      </div>
      
      {/* Waveform Display */}
      <div className="bg-plugin-panel rounded-lg p-3 mb-4 h-12 overflow-hidden">
        <svg className="w-full h-full" viewBox="0 0 100 60">
          <path
            d={LFO_WAVEFORMS.find(w => w.id === lfo.waveform)?.path}
            fill="none"
            stroke={color}
            strokeWidth="2"
            opacity={0.7 + lfo.depth * 0.3}
          />
        </svg>
      </div>

      {/* Waveform Selector */}
      <div className="grid grid-cols-2 gap-1 mb-4">
        {LFO_WAVEFORMS.map((waveform) => (
          <button
            key={waveform.id}
            onClick={() => setLfo(prev => ({ ...prev, waveform: waveform.id }))}
            className={cn(
              "px-2 py-1 rounded text-xs font-medium transition-all duration-200",
              lfo.waveform === waveform.id
                ? "text-background font-bold"
                : "text-foreground/60 hover:text-foreground/80"
            )}
            style={lfo.waveform === waveform.id ? { background: color } : {}}
          >
            {waveform.label}
          </button>
        ))}
      </div>

      {/* Controls */}
      <div className="grid grid-cols-2 gap-4">
        <ModernKnob 
          value={lfo.rate}
          onChange={(v) => setLfo(prev => ({ ...prev, rate: v }))}
          label="RATE"
          min={0}
          max={10}
          size="sm"
          spectrum={[]}
        />
        <ModernKnob 
          value={lfo.depth}
          onChange={(v) => setLfo(prev => ({ ...prev, depth: v }))}
          label="DEPTH"
          min={0}
          max={1}
          size="sm"
          spectrum={[]}
        />
      </div>
    </div>
  );

  return (
    <div className="bg-plugin-panel rounded-2xl p-6 border border-audio-primary/10">
      <h3 className="text-lg font-bold text-center text-foreground mb-6 tracking-wide">
        LFO GENERATORS
      </h3>
      
      <div className="space-y-4">
        <LFOControl 
          lfo={lfo1} 
          setLfo={setLfo1} 
          label="LFO 1" 
          color="hsl(var(--audio-primary))" 
        />
        <LFOControl 
          lfo={lfo2} 
          setLfo={setLfo2} 
          label="LFO 2" 
          color="hsl(var(--audio-secondary))" 
        />
      </div>
    </div>
  );
};