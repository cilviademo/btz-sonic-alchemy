import React, { useState, useEffect } from 'react';
import { cn } from '@/lib/utils';
import { BTZHeader } from './BTZHeader';
import { LevelsPanel } from './LevelsPanel';
import { MainControls } from './MainControls';
import { AIAnalysisPanel } from './AIAnalysisPanel';
import { ProcessingChain } from './ProcessingChain';
import { CompliancePanel } from './CompliancePanel';
import { BTZPluginState } from './types';

export const BTZPlugin: React.FC = () => {
  const [state, setState] = useState<BTZPluginState>({
    punch: 0.5,
    warmth: 0.3,
    boom: 0.2,
    mix: 1.0,
    texture: false,
    drive: 0.0,
    active: true,
    oversampling: false,
  });

  const [inputLevel, setInputLevel] = useState(0);
  const [outputLevel, setOutputLevel] = useState(0);
  const [isProcessing, setIsProcessing] = useState(false);
  const [lufsIntegrated, setLufsIntegrated] = useState(-18);
  const [truePeak, setTruePeak] = useState(-3);

  useEffect(() => {
    if (typeof window === 'undefined') return;
    let raf = 0;
    let last = 0;

    const ema = { in: 0, out: 0, lufs: -18 };

    const tick = (t: number) => {
      raf = requestAnimationFrame(tick);
      if (t - last < 1000 / 60) return;

      const baseLevel = 0.3 + Math.random() * 0.4;
      const processedLevel = baseLevel * (1 + state.drive * 0.08) * state.mix * (state.active ? 1 : 0.7);

      const inLv = baseLevel + (Math.random() - 0.5) * 0.1;
      const out = Math.min(0.98, processedLevel + (Math.random() - 0.5) * 0.05);

      ema.in = ema.in * 0.85 + inLv * 0.15;
      ema.out = ema.out * 0.85 + out * 0.15;

      setInputLevel(ema.in);
      setOutputLevel(ema.out);
      setIsProcessing(Math.random() > 0.7);

      const shortLUFS = -28 + out * 22; // ~[-28, -6]
      ema.lufs = ema.lufs * 0.96 + shortLUFS * 0.04;
      setLufsIntegrated(ema.lufs);

      const tp = -6 + out * 6; // ~[-6, 0]
      setTruePeak(tp);

      last = t;
    };

    raf = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(raf);
  }, [state.drive, state.mix, state.active]);

  const updateParameter = (key: keyof BTZPluginState, value: any) => {
    setState((prev) => ({ ...prev, [key]: value }));
  };

  return (
    <div className="w-full max-w-6xl mx-auto bg-plugin-surface border border-plugin-raised rounded-2xl shadow-[var(--shadow-panel)] overflow-hidden">
      <BTZHeader
        isProcessing={isProcessing}
        active={state.active}
        onToggleActive={() => updateParameter('active', !state.active)}
        oversampling={state.oversampling}
        onToggleOversampling={() => updateParameter('oversampling', !state.oversampling)}
        onApplyPreset={(p) => setState((prev) => ({ ...prev, ...p.state }))}
      />

      <main className="p-8">
        <div className="grid grid-cols-1 xl:grid-cols-12 gap-8">
          {/* Levels */}
          <div className="xl:col-span-2 space-y-8">
            <LevelsPanel inputLevel={inputLevel} outputLevel={outputLevel} />
            <CompliancePanel lufsIntegrated={lufsIntegrated} truePeak={truePeak} />
          </div>

          {/* Main Controls */}
          <div className="xl:col-span-8">
            <MainControls state={state} update={updateParameter} />
          </div>

          {/* AI Analysis */}
          <div className="xl:col-span-2">
            <AIAnalysisPanel punch={state.punch} warmth={state.warmth} boom={state.boom} />
          </div>
        </div>

        <ProcessingChain state={state} />
      </main>
    </div>
  );
};