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
}
