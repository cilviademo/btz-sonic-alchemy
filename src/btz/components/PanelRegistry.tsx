import React from 'react';
import type { BTZPluginState, PanelId } from '@/btz/types';

type Update = <K extends keyof BTZPluginState>(k:K, v:BTZPluginState[K])=>void;

export function renderPanel(id: PanelId, state: BTZPluginState, update: Update): React.ReactNode {
  switch (id) {
    case 'spark':
      return (
        <div className="space-y-3 text-sm">
          <div className="font-semibold uppercase tracking-widest">Spark</div>
          <label className="block">Target LUFS
            <input type="number" className="ml-2 bg-white/5 px-2 py-1"
              value={state.sparkLUFS}
              onChange={e=>update('sparkLUFS', Number(e.target.value))}
            />
          </label>
          <label className="block">Ceiling (dBTP)
            <input type="number" step="0.1" className="ml-2 bg-white/5 px-2 py-1"
              value={state.sparkCeiling}
              onChange={e=>update('sparkCeiling', Number(e.target.value))}
            />
          </label>
        </div>
      );
    case 'shine':
      return (
        <div className="space-y-3 text-sm">
          <div className="font-semibold uppercase tracking-widest">Shine</div>
          <label className="block">Freq (Hz)
            <input type="number" className="ml-2 bg-white/5 px-2 py-1"
              value={state.shineFreqHz}
              onChange={e=>update('shineFreqHz', Number(e.target.value))}
            />
          </label>
          <label className="block">Gain (dB)
            <input type="number" step="0.5" className="ml-2 bg-white/5 px-2 py-1"
              value={state.shineGainDb}
              onChange={e=>update('shineGainDb', Number(e.target.value))}
            />
          </label>
        </div>
      );
    case 'master':
      return (
        <div className="space-y-3 text-sm">
          <div className="font-semibold uppercase tracking-widest">Master</div>
          <label className="block">Blend
            <select className="ml-2 bg-white/5 px-2 py-1"
              value={state.masterBlend}
              onChange={e=>update('masterBlend', e.target.value as any)}
            >
              <option value="transparent">Transparent</option>
              <option value="glue">Glue</option>
              <option value="vintage">Vintage</option>
            </select>
          </label>
        </div>
      );
    case 'meters':
      return <div className="text-sm opacity-80">Detailed metering coming soon.</div>;
    case 'convolver':
      return <div className="text-sm opacity-80">Sub/Convolver settings.</div>;
    case 'deep':
      return <div className="text-sm opacity-80">Deep parameter view.</div>;
    default:
      return null;
  }
}