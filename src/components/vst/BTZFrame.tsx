import React from 'react';
import './btz-layout.css';
import { SparkPanel } from './SparkPanel';
import { ShinePanel } from './ShinePanel';
import { MasterGluePanel } from './MasterGluePanel';
import { DeepControlsPanel } from './DeepControlsPanel';
import { OutputScope } from '@/components/OutputScope';
import { CentralVisualizerCanvas } from '@/components/CentralVisualizerCanvas';
import { ThermalKnob } from '@/components/ThermalKnob';
import { PresetChipRail } from './PresetChipRail';
import { BTZPluginState, EnhancedPreset } from './types';

export const BTZFrame: React.FC<{
  state: BTZPluginState;
  presets: EnhancedPreset[];
  onApplyPreset: (p: EnhancedPreset) => void;
  update: <K extends keyof BTZPluginState>(k: K, v: BTZPluginState[K]) => void;
  meters: { waveformData: Float32Array; lufsIntegrated: number; truePeak: number; };
  spectrum: Float32Array;
  scale?: number; // 0.75..1.5
}> = ({ state, presets, onApplyPreset, update, meters, spectrum, scale = 1 }) => {
  const set = <K extends keyof BTZPluginState>(k: K) => (v: BTZPluginState[K]) => update(k, v);

  return (
    <div className="btz-scale-wrap" style={{ transform: `scale(${scale})` }}>
      <div className="btz-frame">
        {/* Header */}
        <Header className="btz-header" />

        {/* Preset strip */}
        <div className="btz-preset btz-card">
          <PresetChipRail presets={presets} currentState={state} onClick={onApplyPreset} />
        </div>

        {/* Central visualizer */}
        <div className="btz-visual btz-card" style={{ display:'grid', placeItems:'center' }}>
          <CentralVisualizerCanvas
            spectrumData={spectrum}
            waveformData={meters.waveformData}
            isProcessing={true}
            level={Math.max(0, Math.min(1, (state.drive ?? 0) * 0.9 + (state.mix ?? 1) * 0.1))}
          />
        </div>

        {/* Knobs Row */}
        <div className="btz-knobs btz-card">
          <div className="btz-knob-row">
            <ThermalKnob label="PUNCH"  value={state.punch}  onChange={set('punch')}  spectrumData={spectrum} />
            <ThermalKnob label="WARMTH" value={state.warmth} onChange={set('warmth')} spectrumData={spectrum} />
            <ThermalKnob label="BOOM"   value={state.boom}   onChange={set('boom')}   spectrumData={spectrum} />
            <ThermalKnob label="MIX"    value={state.mix}    onChange={set('mix')}    spectrumData={spectrum} />
            <ThermalKnob label="DRIVE"  value={state.drive}  onChange={set('drive')}  spectrumData={spectrum} />
          </div>
        </div>

        {/* Deep controls (numeric / EQ-style) */}
        <div className="btz-deep btz-card">
          <DeepControlsPanel state={state} update={update} />
        </div>

        {/* Right master strip */}
        <aside className="btz-master">
          <div className="btz-master-stack">
            <div className="btz-card"><SparkPanel state={state} update={update} /></div>
            <div className="btz-card"><MasterGluePanel state={state} update={update} /></div>
            <div className="btz-card"><ShinePanel state={state} updateParameter={update as any} /></div>
          </div>
        </aside>

        {/* Bottom meters */}
        <footer className="btz-meters">
          <div className="btz-meters-grid">
            <LufsCard value={meters.lufsIntegrated} />
            <TruePeakCard value={meters.truePeak} />
            <GainReductionCard value={state.sparkGR ?? 0} />
            <div className="btz-card" style={{ padding:12 }}>
              <OutputScope data={meters.waveformData} lufs={meters.lufsIntegrated} peak={meters.truePeak} />
            </div>
          </div>
        </footer>

        {/* Optional status chips (oversampling/latency) */}
        <div className="btz-right">
          <Chip label="OS" value={`${state.sparkOS ?? 8}×`} />
          <Chip label="Latency" value="2 ms" />
        </div>
      </div>
    </div>
  );
};

/* tiny helpers */
const Header: React.FC<React.HTMLAttributes<HTMLDivElement>> = ({ className }) => (
  <div className={`${className} btz-card`} style={{ display:'flex', alignItems:'center', padding:'0 14px', gap:12 }}>
    <div style={{ fontWeight:900, letterSpacing:'.08em' }}>BTZ • BOX TONE ZONE</div>
    <div style={{ marginLeft:'auto', display:'flex', gap:8 }}>
      {/* Your view/skin toggles can live here if you still want them */}
    </div>
  </div>
);

const LufsCard: React.FC<{ value:number }> = ({ value }) => (
  <div className="btz-card" style={{ display:'grid', placeItems:'center' }}>
    <Metric label="LUFS" value={value.toFixed(1)} accent="var(--accent,#ff6a3d)" />
  </div>
);
const TruePeakCard: React.FC<{ value:number }> = ({ value }) => (
  <div className="btz-card" style={{ display:'grid', placeItems:'center' }}>
    <Metric label="TRUE PEAK" value={`${value.toFixed(1)} dBTP`} />
  </div>
);
const GainReductionCard: React.FC<{ value:number }> = ({ value }) => (
  <div className="btz-card" style={{ display:'grid', placeItems:'center' }}>
    <Metric label="GR" value={`${value.toFixed(1)} dB`} />
  </div>
);
const Metric: React.FC<{label:string; value:string; accent?:string}> = ({ label, value, accent }) => (
  <div style={{ textAlign:'center' }}>
    <div style={{ fontSize:12, opacity:.7, letterSpacing:'.18em' }}>{label}</div>
    <div style={{ fontFamily:'var(--mono, ui-monospace, SFMono-Regular)', fontSize:28, fontWeight:800, color:accent }}>{value}</div>
  </div>
);

const Chip: React.FC<{label:string; value:string}> = ({ label, value }) => (
  <div className="btz-card" style={{ padding:'8px 10px' }}>
    <div style={{ fontSize:10, letterSpacing:'.16em', opacity:.7 }}>{label}</div>
    <div style={{ fontWeight:800 }}>{value}</div>
  </div>
);
