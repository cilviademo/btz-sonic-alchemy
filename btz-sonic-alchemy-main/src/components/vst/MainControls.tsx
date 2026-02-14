import React from 'react';
import { Knob } from './Knob';
import { ToggleButton } from './ToggleButton';
import { BTZPluginState } from './types';

interface MainControlsProps {
  state: BTZPluginState;
  update: (key: keyof BTZPluginState, value: any) => void;
}

export const MainControls: React.FC<MainControlsProps> = ({ state, update }) => {
  return (
    <section className="bg-plugin-panel rounded-xl p-6 border border-plugin-raised">
      <h2 className="text-lg font-semibold text-foreground/80 mb-6 text-center">MAIN CONTROLS</h2>
      <div className="grid grid-cols-2 md:grid-cols-3 gap-8">
        <Knob value={state.punch} onChange={(v) => update('punch', v)} label="PUNCH" min={0} max={1} size="lg" />
        <Knob value={state.warmth} onChange={(v) => update('warmth', v)} label="WARMTH" min={0} max={1} size="lg" />
        <Knob value={state.boom} onChange={(v) => update('boom', v)} label="BOOM" min={0} max={1} size="lg" />
        <Knob value={state.mix} onChange={(v) => update('mix', v)} label="MIX" min={0} max={1} size="lg" />
        <Knob value={state.drive} onChange={(v) => update('drive', v)} label="DRIVE" min={0} max={12} unit="dB" size="lg" />
        <div className="flex flex-col items-center justify-center gap-4">
          <ToggleButton value={state.texture} onChange={(v) => update('texture', v)} label="TEXTURE" />
          <p className="text-xs text-foreground/50 text-center">Granular + Convolution</p>
        </div>
      </div>
    </section>
  );
};
