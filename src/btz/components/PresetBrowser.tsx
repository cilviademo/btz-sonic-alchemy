import React, { useState, useMemo } from 'react';
import { cn } from '@/lib/utils';
import type { PresetItem } from './PresetStrip';

export type PresetCategory = 'all' | 'drums' | 'vocals' | 'bass' | 'master' | '2bus' | 'synth';
export type PresetStyle = 'all' | 'clean' | 'warm' | 'aggressive' | 'vintage';

interface CategorizedPresetItem extends PresetItem {
  categories?: PresetCategory[];
  styles?: PresetStyle[];
}

interface Props {
  presets: CategorizedPresetItem[];
  onApply: (p: PresetItem) => void;
  activePresetId?: string;
}

export const PresetBrowser: React.FC<Props> = ({ presets, onApply, activePresetId }) => {
  const [selectedCategory, setSelectedCategory] = useState<PresetCategory>('all');
  const [selectedStyle, setSelectedStyle] = useState<PresetStyle>('all');
  const [searchQuery, setSearchQuery] = useState('');

  const filteredPresets = useMemo(() => {
    return presets.filter(p => {
      const matchesCategory = selectedCategory === 'all' || p.categories?.includes(selectedCategory);
      const matchesStyle = selectedStyle === 'all' || p.styles?.includes(selectedStyle);
      const matchesSearch = !searchQuery || p.label.toLowerCase().includes(searchQuery.toLowerCase());
      return matchesCategory && matchesStyle && matchesSearch;
    });
  }, [presets, selectedCategory, selectedStyle, searchQuery]);

  const categories: Array<{ id: PresetCategory; label: string }> = [
    { id: 'all', label: 'All' },
    { id: 'drums', label: 'Drums' },
    { id: 'vocals', label: 'Vocals' },
    { id: 'bass', label: 'Bass' },
    { id: 'master', label: 'Master' },
    { id: '2bus', label: '2-Bus' },
    { id: 'synth', label: 'Synth' },
  ];

  const styles: Array<{ id: PresetStyle; label: string }> = [
    { id: 'all', label: 'All Styles' },
    { id: 'clean', label: 'Clean' },
    { id: 'warm', label: 'Warm' },
    { id: 'aggressive', label: 'Aggressive' },
    { id: 'vintage', label: 'Vintage' },
  ];

  return (
    <div className="space-y-3">
      {/* Search */}
      <input
        type="text"
        placeholder="Search presets..."
        value={searchQuery}
        onChange={(e) => setSearchQuery(e.target.value)}
        className="w-full rounded bg-white/5 px-3 py-2 text-sm placeholder:opacity-40 focus:bg-white/10 focus:outline-none focus:ring-1 focus:ring-white/30"
      />

      {/* Category Filter */}
      <div>
        <div className="mb-1.5 text-[10px] uppercase tracking-widest opacity-50">Category</div>
        <div className="flex flex-wrap gap-1.5">
          {categories.map(cat => (
            <button
              key={cat.id}
              type="button"
              onClick={() => setSelectedCategory(cat.id)}
              className={cn(
                "rounded px-2 py-1 text-xs transition-all",
                selectedCategory === cat.id
                  ? "bg-white/20 text-white shadow-sm"
                  : "bg-white/5 text-white/60 hover:bg-white/10"
              )}
            >
              {cat.label}
            </button>
          ))}
        </div>
      </div>

      {/* Style Filter */}
      <div>
        <div className="mb-1.5 text-[10px] uppercase tracking-widest opacity-50">Style</div>
        <div className="flex flex-wrap gap-1.5">
          {styles.map(style => (
            <button
              key={style.id}
              type="button"
              onClick={() => setSelectedStyle(style.id)}
              className={cn(
                "rounded px-2 py-1 text-xs transition-all",
                selectedStyle === style.id
                  ? "bg-white/20 text-white shadow-sm"
                  : "bg-white/5 text-white/60 hover:bg-white/10"
              )}
            >
              {style.label}
            </button>
          ))}
        </div>
      </div>

      {/* Preset List */}
      <div>
        <div className="mb-1.5 flex items-center justify-between text-[10px] uppercase tracking-widest opacity-50">
          <span>Presets</span>
          <span>{filteredPresets.length} found</span>
        </div>
        <div className="max-h-[300px] space-y-1 overflow-y-auto pr-1">
          {filteredPresets.map(p => (
            <button
              key={p.id}
              type="button"
              onClick={() => onApply(p)}
              className={cn(
                "w-full rounded px-3 py-2 text-left text-sm transition-all",
                p.id === activePresetId
                  ? "bg-gradient-to-r from-white/20 to-white/10 text-white shadow-sm ring-1 ring-white/30"
                  : "bg-white/5 text-white/80 hover:bg-white/10"
              )}
            >
              <div className="font-medium">{p.label}</div>
              {(p.categories || p.styles) && (
                <div className="mt-0.5 flex gap-1.5 text-[9px] opacity-50">
                  {p.categories?.slice(0, 2).map(cat => (
                    <span key={cat}>#{cat}</span>
                  ))}
                  {p.styles?.slice(0, 1).map(style => (
                    <span key={style} className="italic">
                      {style}
                    </span>
                  ))}
                </div>
              )}
            </button>
          ))}
          {filteredPresets.length === 0 && (
            <div className="py-8 text-center text-sm opacity-40">
              No presets match your filters
            </div>
          )}
        </div>
      </div>
    </div>
  );
};
