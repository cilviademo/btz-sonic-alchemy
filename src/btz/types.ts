export type Oversample = 1 | 2 | 4 | 8 | 16;
export type SparkMode = 'soft' | 'hard';
export type MasterBlend = 'transparent' | 'glue' | 'vintage';

export interface BTZPluginState {
  // core
  punch: number; warmth: number; boom: number; mix: number; drive: number;
  texture: boolean; active: boolean; oversampling: boolean;

  // I/O (UX improvement #11)
  inputGain: number;       // -12 to +12 dB
  outputGain: number;      // -12 to +12 dB
  autoGain: boolean;       // loudness matching

  // SPARK - Advanced Clipping & Limiting Engine
  // FL Studio Clipper + GoldClip + BigClipper + KClip + Acustica clippers
  // The magic of BTZ: brutal loudness with surgical transparency
  sparkEnabled: boolean;   // Default: true (globally activated for instant loudness)
  sparkLUFS: number;       // target integrated loudness: -14 to 0 LUFS (default: -5 for commercial competitiveness)
  sparkCeiling: number;    // true peak ceiling: -3 to 0 dBTP (default: -0.3 to prevent inter-sample peaks)
  sparkMix: number;        // wet/dry blend: 0..1 (default: 1.0 = 100% for full effect)
  sparkOS: Oversample;     // oversampling factor: 1, 2, 4, 8, or 16 (default: 8 for artifact-free processing)
  sparkAutoOS: boolean;    // auto-engage oversampling based on signal analysis (default: true)
  sparkMode: SparkMode;    // 'soft' (musical/warm) or 'hard' (aggressive/punchy)

  // SHINE - Ultra-High Frequency Air Enhancement
  // SSL Fusion Air + Maag EQ Air Band (10kHz-80kHz+ ultrasonic magic)
  // Ethereal highs, crystalline crispness, analog-inspired sheen
  shineEnabled: boolean;   // Default: false (user-activated for selective sweetening)
  shineFreqHz: number;     // center frequency: 10k-80kHz (default: 20kHz, extends beyond Nyquist via oversampling)
  shineGainDb: number;     // boost/cut: -12 to +12 dB (default: +3 for gentle, organic lift)
  shineQ: number;          // resonance/width: 0.1 (broad sheen) to 2.0 (focused crispness), default: 0.5
  shineMix: number;        // wet/dry blend: 0..1 (default: 0.5 = 50% for subtle layering)
  shineAutoOS: boolean;    // auto-engage oversampling for ultrasonic transparency (default: true)

  // Master macro
  masterEnabled: boolean;
  masterMacro: number;     // 0..1
  masterBlend: MasterBlend;
  masterMix: number;

  // other toggles
  transEnabled: boolean; eqEnabled: boolean; dynEnabled: boolean;
  subEnabled: boolean; consoleEnabled: boolean;

  // optional params used by sub panel
  subAmount?: number;

  // UX state (UX improvement #9)
  precisionMode: boolean;  // quantized, fine-grained adjustments
}

export type PanelId =
  | 'meters' | 'spark' | 'shine' | 'master' | 'convolver' | 'deep';

export type ParamId = keyof BTZPluginState;

// Utilities
export type Patch = Partial<BTZPluginState>;

export const DEFAULT_STATE: BTZPluginState = {
  punch: 0, warmth: 0, boom: 0, mix: 1, drive: 0,
  texture: false, active: true, oversampling: true,

  inputGain: 0, outputGain: 0, autoGain: false,

  sparkEnabled: true, sparkLUFS: -5, sparkCeiling: -0.3, sparkMix: 1,
  sparkOS: 8, sparkAutoOS: true, sparkMode: 'soft',

  shineEnabled: false, shineFreqHz: 20000, shineGainDb: 3, shineQ: 0.5, shineMix: 0.5, shineAutoOS: true,

  masterEnabled: false, masterMacro: 0.5, masterBlend: 'transparent', masterMix: 1,

  transEnabled: false, eqEnabled: false, dynEnabled: false, subEnabled: false, consoleEnabled: false,
  subAmount: 0.5,

  precisionMode: false
};
