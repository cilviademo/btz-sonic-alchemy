import React from 'react';
import { BTZPluginState } from './types';

// reuse your existing panels:
import { SparkPanel } from './SparkPanel';
import { ShinePanel } from './ShinePanel';
import { MasterGluePanel } from './MasterGluePanel';
import { DeepControlsPanel } from './DeepControlsPanel';

export type PanelId = 'spark' | 'shine' | 'master' | 'deep' | 'convolver' | 'meters';

export function renderPanel(
  id: PanelId,
  state: BTZPluginState,
  update: <K extends keyof BTZPluginState>(k: K, v: BTZPluginState[K]) => void
) {
  switch (id) {
    case 'spark':
      return <SparkPanel state={state} update={(k, v)=>update(k as any, v as any)} />;
    case 'shine':
      return <ShinePanel state={state} updateParameter={(k, v)=>update(k as any, v as any)} />;
    case 'master':
      return <MasterGluePanel state={state} update={update} />;
    case 'deep':
      return <DeepControlsPanel state={state} update={update} />;
    case 'convolver':
      return (
        <div className="text-sm opacity-80">
          <div className="text-[11px] tracking-[.22em] opacity-70 mb-3">CONVOLUTION REVERB</div>
          <p>Hook your IRConvolverPanel here (kept minimal for now).</p>
        </div>
      );
    case 'meters':
      return (
        <div className="text-sm opacity-80">
          <div className="text-[11px] tracking-[.22em] opacity-70 mb-3">PROFESSIONAL METERING</div>
          <p>Mount your LUFS / True Peak / GR scope here.</p>
        </div>
      );
    default:
      return null;
  }
}
