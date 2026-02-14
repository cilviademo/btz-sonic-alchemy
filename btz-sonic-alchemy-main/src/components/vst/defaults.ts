import type { BTZPluginState } from './types';

export const DEFAULT_PRESET: { id: string; label: string; state: BTZPluginState } = {
  id: 'default',
  label: 'Default',
  state: {
    // Core
    punch: 0, warmth: 0, boom: 0, mix: 1, drive: 0,
    texture: false, active: true, oversampling: true,
    clippingType: 'soft', clippingBlend: 0.5, clippingEnabled: false,
    lufsTarget: -5, aiEnhance: false, timbralTransfer: false, aiAutomation: true,
    gateThreshold: -40, transientAmount: 0, saturationAmount: 0,
    subHarmonics: false, consoleGlue: true, oversamplingRate: 4,

    // SPARK
    sparkEnabled: true, sparkLUFS: -5, sparkCeiling: -0.3, sparkMix: 1,
    sparkOS: 8, sparkAutoOS: true, sparkMode: 'soft', sparkGR: 0,

    // SHINE
    shineEnabled: false, shineFreqHz: 20000, shineGainDb: 3, shineQ: 0.5, shineMix: 0.5,
    shineAutoOS: true, shineAB: false,

    // MASTER GLUE/MAX
    masterEnabled: false, masterMacro: 0.5, masterBlend: 'transparent', masterMix: 1,
    masterCompAttack: 10, masterCompRelease: 100, masterCompRatio: 2,
    masterLimiterCeiling: -0.3, masterLimiterLookahead: 0.8,

    // Deep toggles
    transEnabled: false, eqEnabled: false, dynEnabled: false, subEnabled: false, consoleEnabled: false,

    // Deep params
    transAttack: 0, transSustain: 0, transDetect: 'wide',
    eqLowGain: 0, eqLowFreq: 80, eqMidGain: 0, eqMidFreq: 1200, eqMidQ: 1,
    eqHighGain: 0, eqHighFreq: 8000,
    dynThreshold: 0, dynRatio: 2, dynAttack: 8, dynRelease: 120, dynKnee: 2,
    subAmount: 0, subFreq: 50,
    consoleDrive: 0.15, consoleCrosstalk: 0.1,
  } as BTZPluginState,
};
