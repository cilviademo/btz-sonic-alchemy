import { PresetOption } from './PresetsSelect';
import { BTZPluginState } from './types';

// Pro Style Presets inspired by top drum enhancement plugins (2023–2025)
// Each preset tweaks core BTZ controls to evoke the referenced tool's behavior.
export const PRO_STYLE_PRESETS: PresetOption[] = [
  { id: 'default', label: 'Default', state: { punch: 0, warmth: 0, boom: 0, mix: 1, drive: 0, clippingEnabled: true, clippingType: 'soft', clippingBlend: 1, lufsTarget: -5, sparkEnabled: true, sparkMix: 1, sparkLufsTarget: -5, sparkCeilingDb: -0.3, sparkOversampling: 'auto' as any } },
  { id: 'dynamic-design', label: 'Dynamic-Design', state: { punch: 0.85, transientAmount: 0.8, mix: 0.95 } },
  { id: 'punch-smack', label: 'Punch-Smack', state: { punch: 0.9, transientAmount: 0.9, gateThreshold: -35, mix: 1 } },
  { id: 'precision-q', label: 'Precision-Q', state: { boom: 0.35, warmth: 0.4, mix: 1 } },
  { id: 'analog-reel', label: 'Analog-Reel', state: { warmth: 0.75, saturationAmount: 0.6, consoleGlue: true, mix: 0.95 } },
  { id: 'vintage-tape', label: 'Vintage-Tape', state: { warmth: 0.8, saturationAmount: 0.7, drive: 0.5, mix: 0.95 } },
  { id: 'sonic-vital', label: 'Sonic-Vital', state: { warmth: 0.6, texture: true, saturationAmount: 0.5 } },
  { id: 'classic-76', label: 'Classic-76', state: { punch: 0.7, drive: 0.6, clippingEnabled: true, clippingType: 'hard', clippingBlend: 0.4 } },
  { id: 'saturation-decap', label: 'Saturation-Decap', state: { warmth: 0.85, drive: 0.7, saturationAmount: 0.8, mix: 0.9 } },
  { id: 'drum-sculpt', label: 'Drum-Sculpt', state: { punch: 0.85, transientAmount: 0.85, mix: 1 } },
  { id: 'transient-neutron', label: 'Transient-Neutron', state: { punch: 0.8, transientAmount: 0.8, oversampling: true } },
  { id: 'bass-synth', label: 'Bass-Synth', state: { boom: 0.9, subHarmonics: true, mix: 0.95 } },
  { id: 'clarity-air', label: 'Clarity-Air', state: { texture: true, warmth: 0.4, saturationAmount: 0.3 } },
  { id: 'split-physic', label: 'Split-Physic', state: { punch: 0.75, texture: true, transientAmount: 0.7 } },
  { id: 'retro-goeq', label: 'Retro-GoEQ', state: { boom: 0.5, warmth: 0.55, mix: 0.95 } },
  { id: 'intelligent-eq', label: 'Intelligent-EQ', state: { aiAutomation: true, boom: 0.45, warmth: 0.45 } },
  { id: 'tone-char', label: 'Tone-Char', state: { aiAutomation: true, punch: 0.65, warmth: 0.55 } },
  { id: 'tube-cascade', label: 'Tube-Cascade', state: { warmth: 0.9, saturationAmount: 0.85, drive: 0.6, mix: 0.9 } },
  { id: 'low-mint', label: 'Low-Mint', state: { boom: 0.85, subHarmonics: true, mix: 0.9 } },
  { id: 'stereo-vital', label: 'Stereo-Vital', state: { texture: true, warmth: 0.5, mix: 1 } },
  { id: 'vintage-76', label: 'Vintage-76', state: { punch: 0.75, drive: 0.6, clippingEnabled: true, clippingType: 'soft', clippingBlend: 0.6 } },
  { id: 'dynamic-intense', label: 'Dynamic-Intense', state: { drive: 0.8, aiAutomation: true, lufsTarget: -8 } },
  { id: 'harmonic-spark', label: 'Harmonic-Spark', state: { warmth: 0.8, saturationAmount: 0.7, mix: 0.95 } },
  { id: 'multiband-saturn', label: 'Multiband-Saturn', state: { warmth: 0.75, saturationAmount: 0.8, texture: true } },
  { id: 'mix-vault', label: 'Mix-Vault', state: { punch: 0.6, warmth: 0.6, boom: 0.5, mix: 1 } },
  { id: 'drum-trigger', label: 'Drum-Trigger', state: { punch: 0.9, gateThreshold: -30, transientAmount: 0.9, mix: 1 } },
];
