import React, { useState, useEffect } from 'react';
import { cn } from '@/lib/utils';

export const StepSequencer: React.FC = () => {
  const [steps, setSteps] = useState(Array(16).fill(0));
  const [currentStep, setCurrentStep] = useState(0);
  const [isPlaying, setIsPlaying] = useState(false);
  const [tempo, setTempo] = useState(120);

  // Auto-advance steps when playing
  useEffect(() => {
    if (!isPlaying) return;

    const interval = setInterval(() => {
      setCurrentStep(prev => (prev + 1) % 16);
    }, (60 / tempo) * 250); // 16th notes

    return () => clearInterval(interval);
  }, [isPlaying, tempo]);

  const toggleStep = (index: number) => {
    setSteps(prev => prev.map((step, i) => 
      i === index ? (step === 0 ? 1 : 0) : step
    ));
  };

  return (
    <div className="bg-plugin-panel rounded-2xl p-6 border border-audio-primary/10">
      <div className="flex items-center justify-between mb-6">
        <h3 className="text-lg font-bold text-foreground tracking-wide">
          STEP SEQUENCER
        </h3>
        
        <div className="flex items-center gap-4">
          <div className="flex items-center gap-2">
            <span className="text-sm text-foreground/70">BPM:</span>
            <input
              type="range"
              min="60"
              max="200"
              value={tempo}
              onChange={(e) => setTempo(parseInt(e.target.value))}
              className="w-20 accent-audio-primary"
            />
            <span className="text-sm font-bold text-audio-primary min-w-[40px]">
              {tempo}
            </span>
          </div>
          
          <button
            onClick={() => setIsPlaying(!isPlaying)}
            className={cn(
              "px-6 py-2 rounded-lg font-bold text-sm transition-all duration-300",
              isPlaying
                ? "bg-audio-warning text-background"
                : "bg-audio-primary text-background hover:bg-audio-primary/80"
            )}
          >
            {isPlaying ? 'STOP' : 'PLAY'}
          </button>
        </div>
      </div>
      
      {/* Step Grid */}
      <div className="grid grid-cols-16 gap-1 mb-4">
        {steps.map((step, index) => (
          <button
            key={index}
            onClick={() => toggleStep(index)}
            className={cn(
              "aspect-square rounded border-2 transition-all duration-150 text-xs font-bold",
              step > 0 
                ? "bg-audio-primary border-audio-primary text-background" 
                : "bg-plugin-surface border-plugin-raised hover:border-audio-primary/50 text-foreground/60",
              currentStep === index && "ring-2 ring-audio-secondary ring-offset-1 ring-offset-plugin-panel"
            )}
            style={currentStep === index ? {
              transform: 'scale(1.1)',
              boxShadow: currentStep === index ? '0 0 10px hsl(var(--audio-secondary))' : 'none'
            } : {}}
          >
            {index + 1}
          </button>
        ))}
      </div>

      {/* Pattern Controls */}
      <div className="flex justify-center gap-4">
        <button
          onClick={() => setSteps(Array(16).fill(1))}
          className="px-4 py-2 bg-audio-tertiary text-background rounded-lg text-sm font-bold hover:bg-audio-tertiary/80 transition-colors"
        >
          ALL ON
        </button>
        <button
          onClick={() => setSteps(Array(16).fill(0))}
          className="px-4 py-2 bg-plugin-raised text-foreground rounded-lg text-sm font-bold hover:bg-plugin-raised/80 transition-colors"
        >
          CLEAR
        </button>
        <button
          onClick={() => setSteps(prev => prev.map(() => Math.random() > 0.5 ? 1 : 0))}
          className="px-4 py-2 bg-audio-secondary text-background rounded-lg text-sm font-bold hover:bg-audio-secondary/80 transition-colors"
        >
          RANDOM
        </button>
      </div>
      
      {/* Current Step Indicator */}
      <div className="flex justify-center mt-4">
        <div className="text-sm text-foreground/70">
          Step: <span className="text-audio-primary font-bold">{currentStep + 1}/16</span>
        </div>
      </div>
    </div>
  );
};