import React from 'react';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';
import { BTZPluginState } from './types';

export interface PresetOption {
  id: string;
  label: string;
  state: Partial<BTZPluginState>;
}

const PRESETS: PresetOption[] = [
  {
    id: 'punchy-kick',
    label: 'Punchy Kick',
    state: { punch: 0.8, warmth: 0.35, boom: 0.6, mix: 0.9, drive: 2.5, texture: false },
  },
  {
    id: 'silky-snare',
    label: 'Silky Snare',
    state: { punch: 0.7, warmth: 0.5, boom: 0.25, mix: 0.95, drive: 1.5, texture: true },
  },
  {
    id: 'room-glue',
    label: 'Room Glue',
    state: { punch: 0.4, warmth: 0.55, boom: 0.3, mix: 0.85, drive: 1.0, texture: true },
  },
  {
    id: 'tape-warmth',
    label: 'Tape Warmth',
    state: { punch: 0.35, warmth: 0.75, boom: 0.35, mix: 0.9, drive: 0.8, texture: false },
  },
  {
    id: 'boom-sculpt',
    label: 'Boom Sculpt',
    state: { punch: 0.45, warmth: 0.3, boom: 0.8, mix: 0.9, drive: 1.2, texture: false },
  },
];

interface PresetsSelectProps {
  onApply: (preset: PresetOption) => void;
  presets?: PresetOption[];
}

export const PresetsSelect: React.FC<PresetsSelectProps> = ({ onApply, presets = PRESETS }) => {
  return (
    <div className="min-w-[180px]">
      <Select
        onValueChange={(val) => {
          const p = presets.find((x) => x.id === val);
          if (p) onApply(p);
        }}
      >
        <SelectTrigger className="w-[180px]">
          <SelectValue placeholder="Presets" />
        </SelectTrigger>
        <SelectContent>
          {presets.map((p) => (
            <SelectItem key={p.id} value={p.id}>
              {p.label}
            </SelectItem>
          ))}
        </SelectContent>
      </Select>
    </div>
  );
};
