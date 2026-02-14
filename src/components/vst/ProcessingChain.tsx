import React from 'react';
import { cn } from '@/lib/utils';
import { BTZPluginState } from './types';

interface ProcessingChainProps {
  state: Pick<BTZPluginState, 'punch' | 'warmth' | 'boom' | 'mix' | 'texture' | 'drive'>;
}

export const ProcessingChain: React.FC<ProcessingChainProps> = ({ state }) => {
  const items = [
    { name: 'Gate', active: state.punch > 0.3 },
    { name: 'Transient Shaper', active: state.punch > 0.1 },
    { name: 'Compressor', active: state.punch > 0.2 },
    { name: 'Wave Shaper', active: state.warmth > 0.1 },
    { name: 'Fuzz', active: state.warmth > 0.5 },
    { name: 'Tape Emulator', active: state.warmth > 0.2 },
    { name: 'Bass Enhancer', active: state.boom > 0.1 },
    { name: 'EQ', active: state.boom > 0.2 },
    { name: 'Convolution', active: state.texture },
    { name: 'Granular', active: state.texture },
    { name: 'Console Emulator', active: state.mix > 0.1 },
    { name: 'Limiter', active: state.drive > 0.1 },
  ];
  return (
    <section className="mt-8 bg-plugin-panel rounded-xl p-6 border border-plugin-raised">
      <h2 className="text-lg font-semibold text-foreground/80 mb-4 text-center">PROCESSING CHAIN</h2>
      <div className="flex items-center justify-center gap-3 flex-wrap">
        {items.map((processor, index) => (
          <div key={processor.name} className="flex items-center gap-2">
            <div
              className={cn(
                'px-3 py-1 rounded-md text-xs font-medium transition-all duration-200',
                processor.active
                  ? 'bg-audio-primary/20 text-audio-primary border border-audio-primary/30'
                  : 'bg-plugin-surface text-foreground/50 border border-plugin-raised'
              )}
            >
              {processor.name}
            </div>
            {index < items.length - 1 && <div className="text-foreground/30">→</div>}
          </div>
        ))}
      </div>
    </section>
  );
};
