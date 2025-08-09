export interface BTZPluginState {
  punch: number;
  warmth: number;
  boom: number;
  mix: number;
  texture: boolean;
  drive: number;
  active: boolean;
  oversampling: boolean;
  lufsTarget?: number;
  aiEnhance?: boolean;
  timbralTransfer?: boolean;
  oversamplingRate?: number;
  clippingType?: 'soft' | 'hard' | 'tube' | 'tape' | 'digital';
  clippingBlend?: number;
  clippingEnabled?: boolean;
  aiAutomation?: boolean;
  gateThreshold?: number;
  transientAmount?: number;
  saturationAmount?: number;
  subHarmonics?: boolean;
  consoleGlue?: boolean;

  // SPARK: advanced clipping engine
  sparkEnabled?: boolean;            // global enable
  sparkLufsTarget?: number;          // -14..0 (default -5)
  sparkCeilingDb?: number;           // -3..0 (default -0.3)
  sparkMix?: number;                 // 0..1 (default 1)
  sparkOversampling?: 1 | 2 | 4 | 8 | 16 | 'auto';

  // SHINE: air enhancement
  shineEnabled?: boolean;            // enable air band
  shineFreqHz?: number;              // 10k..80k (default 20k)
  shineGainDb?: number;              // -12..+12 (default +3)
  shineQ?: number;                   // 0.1..2.0 (default 0.5)
  shineMix?: number;                 // 0..1 (default 0.5)
  shineAB?: boolean;                 // A/B toggle
}

export interface EnhancedPreset {
  id: string;
  label: string;
  state: BTZPluginState;
}
