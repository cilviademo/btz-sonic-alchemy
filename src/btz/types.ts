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

  // Spark (limiter/exciter)
  sparkEnabled: boolean;
  sparkLUFS: number;       // target loudness
  sparkCeiling: number;    // dBTP
  sparkMix: number;        // 0..1
  sparkOS: Oversample;
  sparkAutoOS: boolean;
  sparkMode: SparkMode;

  // Shine (air band)
  shineEnabled: boolean;
  shineFreqHz: number;
  shineGainDb: number;
  shineQ: number;
  shineMix: number;
  shineAutoOS: boolean;

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
