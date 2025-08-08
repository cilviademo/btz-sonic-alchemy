import { PresetOption } from './PresetsSelect';
import { BTZPluginState } from './types';

// Pro Style Presets inspired by top drum enhancement plugins (2023–2025)
// Each preset tweaks core BTZ controls to evoke the referenced tool's behavior.
export const PRO_STYLE_PRESETS: PresetOption[] = [
  { id: 'spl-transient-designer', label: 'SPL Transient Designer', state: { punch: 0.85, transientAmount: 0.8, mix: 0.95 } },
  { id: 'waves-smack-attack', label: 'Waves Smack Attack', state: { punch: 0.9, transientAmount: 0.9, gateThreshold: -35, mix: 1 } },
  { id: 'fabfilter-proq3', label: 'FabFilter Pro-Q 3', state: { boom: 0.35, warmth: 0.4, mix: 1 } },
  { id: 'softube-tape', label: 'Softube Tape', state: { warmth: 0.75, saturationAmount: 0.6, consoleGlue: true, mix: 0.95 } },
  { id: 'baby-taip', label: 'Baby Audio TAIP', state: { warmth: 0.8, saturationAmount: 0.7, drive: 0.5, mix: 0.95 } },
  { id: 'waves-vitamin', label: 'Waves Vitamin', state: { warmth: 0.6, texture: true, saturationAmount: 0.5 } },
  { id: 'uad-1176', label: 'UAD 1176', state: { punch: 0.7, drive: 0.6, clippingEnabled: true, clippingType: 'hard', clippingBlend: 0.4 } },
  { id: 'soundtoys-decapitator', label: 'Decapitator', state: { warmth: 0.85, drive: 0.7, saturationAmount: 0.8, mix: 0.9 } },
  { id: 'xln-ds10', label: 'XLN DS-10', state: { punch: 0.85, transientAmount: 0.85, mix: 1 } },
  { id: 'neutron-transient', label: 'Neutron Transient', state: { punch: 0.8, transientAmount: 0.8, oversampling: true } },
  { id: 'bx-subsynth', label: 'bx_subsynth', state: { boom: 0.9, subHarmonics: true, mix: 0.95 } },
  { id: 'fresh-air', label: 'Fresh Air', state: { texture: true, warmth: 0.4, saturationAmount: 0.3 } },
  { id: 'eventide-physion', label: 'Eventide Physion', state: { punch: 0.75, texture: true, transientAmount: 0.7 } },
  { id: 'klevgrand-gotoeq', label: 'Klevgrand GotoEQ', state: { boom: 0.5, warmth: 0.55, mix: 0.95 } },
  { id: 'sonible-smarteq3', label: 'smart:EQ 3', state: { aiAutomation: true, boom: 0.45, warmth: 0.45 } },
  { id: 'noveltech-character', label: 'Noveltech CHARACTER', state: { aiAutomation: true, punch: 0.65, warmth: 0.55 } },
  { id: 'elysia-phils-cascade', label: "elysia Phil's Cascade", state: { warmth: 0.9, saturationAmount: 0.85, drive: 0.6, mix: 0.9 } },
  { id: 'bass-mint', label: 'Unfiltered Bass-Mint', state: { boom: 0.85, subHarmonics: true, mix: 0.9 } },
  { id: 'spl-vitalizer', label: 'SPL Vitalizer', state: { texture: true, warmth: 0.5, mix: 1 } },
  { id: 'waves-cla76', label: 'Waves CLA-76', state: { punch: 0.75, drive: 0.6, clippingEnabled: true, clippingType: 'soft', clippingBlend: 0.6 } },
  { id: 'zynaptiq-intensity', label: 'Zynaptiq Intensity', state: { drive: 0.8, aiAutomation: true, lufsTarget: -8 } },
  { id: 'softube-harmonics', label: 'Softube Harmonics', state: { warmth: 0.8, saturationAmount: 0.7, mix: 0.95 } },
  { id: 'fabfilter-saturn2', label: 'FabFilter Saturn 2', state: { warmth: 0.75, saturationAmount: 0.8, texture: true } },
  { id: 'ik-mixbox', label: 'IK MixBox', state: { punch: 0.6, warmth: 0.6, boom: 0.5, mix: 1 } },
  { id: 'addictive-trigger', label: 'Addictive Trigger', state: { punch: 0.9, gateThreshold: -30, transientAmount: 0.9, mix: 1 } },
];
