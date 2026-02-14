import React, { useState } from 'react';
import { ModernKnob } from './ModernKnob';

export const EnvelopePanel: React.FC = () => {
  const [envelope, setEnvelope] = useState({
    attack: 0.1,
    decay: 0.3,
    sustain: 0.7,
    release: 0.5
  });

  const generateADSRPath = () => {
    const { attack, decay, sustain, release } = envelope;
    const attackX = attack * 25;
    const decayX = attackX + decay * 25;
    const sustainX = decayX + 30;
    const releaseX = sustainX + release * 25;
    
    return `M 0 50 L ${attackX} 10 L ${decayX} ${20 + (1 - sustain) * 25} L ${sustainX} ${20 + (1 - sustain) * 25} L ${releaseX} 50`;
  };

  return (
    <div className="bg-plugin-panel rounded-2xl p-6 border border-audio-primary/10">
      <h3 className="text-lg font-bold text-center text-foreground mb-6 tracking-wide">
        ENVELOPE
      </h3>
      
      {/* ADSR Visualization */}
      <div className="bg-plugin-surface rounded-lg p-4 mb-6 h-20 overflow-hidden">
        <svg className="w-full h-full" viewBox="0 0 120 60">
          <defs>
            <linearGradient id="adsrGrad" x1="0%" y1="0%" x2="100%" y2="0%">
              <stop offset="0%" stopColor="hsl(var(--audio-tertiary))" stopOpacity="0.6"/>
              <stop offset="25%" stopColor="hsl(var(--audio-primary))" stopOpacity="0.8"/>
              <stop offset="50%" stopColor="hsl(var(--audio-secondary))" stopOpacity="0.8"/>
              <stop offset="100%" stopColor="hsl(var(--audio-tertiary))" stopOpacity="0.6"/>
            </linearGradient>
          </defs>
          
          <path
            d={generateADSRPath()}
            fill="none"
            stroke="url(#adsrGrad)"
            strokeWidth="3"
            strokeLinecap="round"
          />
          
          {/* Stage Labels */}
          <text x="12" y="55" className="text-xs fill-foreground/60" fontSize="8">A</text>
          <text x="35" y="55" className="text-xs fill-foreground/60" fontSize="8">D</text>
          <text x="65" y="55" className="text-xs fill-foreground/60" fontSize="8">S</text>
          <text x="95" y="55" className="text-xs fill-foreground/60" fontSize="8">R</text>
        </svg>
      </div>

      {/* ADSR Controls */}
      <div className="grid grid-cols-2 gap-4">
        <ModernKnob 
          value={envelope.attack}
          onChange={(v) => setEnvelope(prev => ({ ...prev, attack: v }))}
          label="ATTACK"
          min={0}
          max={2}
          size="sm"
          spectrum={[]}
        />
        <ModernKnob 
          value={envelope.decay}
          onChange={(v) => setEnvelope(prev => ({ ...prev, decay: v }))}
          label="DECAY"
          min={0}
          max={2}
          size="sm"
          spectrum={[]}
        />
        <ModernKnob 
          value={envelope.sustain}
          onChange={(v) => setEnvelope(prev => ({ ...prev, sustain: v }))}
          label="SUSTAIN"
          min={0}
          max={1}
          size="sm"
          spectrum={[]}
        />
        <ModernKnob 
          value={envelope.release}
          onChange={(v) => setEnvelope(prev => ({ ...prev, release: v }))}
          label="RELEASE"
          min={0}
          max={3}
          size="sm"
          spectrum={[]}
        />
      </div>
    </div>
  );
};