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
}

export interface EnhancedPreset {
  id: string;
  label: string;
  state: BTZPluginState;
}
