export type ClipType = 'soft' | 'hard' | 'tube' | 'tape' | 'digital';
export type GlueBlend = 'vintage' | 'digital' | 'transparent';

export interface BTZPluginState {
  // Existing core
  punch: number; warmth: number; boom: number; mix: number; drive: number;
  texture: boolean; active: boolean; oversampling: boolean;
  clippingType?: ClipType; clippingBlend?: number; clippingEnabled?: boolean;
  lufsTarget?: number; aiEnhance?: boolean; timbralTransfer?: boolean; aiAutomation?: boolean;
  gateThreshold?: number; transientAmount?: number; saturationAmount?: number;
  subHarmonics?: boolean; consoleGlue?: boolean; oversamplingRate?: number;

  // ==== SPARK (master clipper) ====
  sparkEnabled?: boolean;            // default true
  sparkLUFS?: number;                // -14..0 (default -5)
  sparkCeiling?: number;             // -3..0 dBTP (default -0.3)
  sparkMix?: number;                 // 0..1 (default 1)
  sparkOS?: 1 | 2 | 4 | 8 | 16;      // default 8
  sparkAutoOS?: boolean;             // default true
  sparkMode?: ClipType;              // soft/hard/tube/tape/digital
  sparkGR?: number;                  // meter (dB)

  // ==== SHINE (air band) ====
  shineEnabled?: boolean;            // default false
  shineFreqHz?: number;              // 10k..80k (default 20k)
  shineGainDb?: number;              // -12..+12 (default +3)
  shineQ?: number;                   // 0.1..2.0 (default 0.5)
  shineMix?: number;                 // 0..1 (default 0.5)
  shineAutoOS?: boolean;             // default true
  shineAB?: boolean;

  // ==== MASTER GLUE/MAX (Shadow Hills + L-style limiter) ====
  masterEnabled?: boolean;           // default false
  masterMacro?: number;              // 0..1
  masterBlend?: GlueBlend;           // vintage/digital/transparent
  masterMix?: number;                // 0..1
  masterCompAttack?: number;         // ms
  masterCompRelease?: number;        // ms
  masterCompRatio?: number;          // 1..10
  masterLimiterCeiling?: number;     // -1..0
  masterLimiterLookahead?: number;   // ms

  // ==== DEEP CONTROLS toggles ====
  transEnabled?: boolean; eqEnabled?: boolean; dynEnabled?: boolean;
  subEnabled?: boolean; consoleEnabled?: boolean;

  // Transient
  transAttack?: number;              // -100..+100 %
  transSustain?: number;             // -100..+100 %
  transDetect?: 'wide'|'mid'|'tight';

  // EQ
  eqLowGain?: number; eqLowFreq?: number;
  eqMidGain?: number; eqMidFreq?: number; eqMidQ?: number;
  eqHighGain?: number; eqHighFreq?: number;

  // Dynamics
  dynThreshold?: number; dynRatio?: number; dynAttack?: number;
  dynRelease?: number; dynKnee?: number;

  // Sub & Console
  subAmount?: number; subFreq?: number;
  consoleDrive?: number; consoleCrosstalk?: number;
}

export interface EnhancedPreset {
  id: string;
  label: string;
  state: BTZPluginState;
}
