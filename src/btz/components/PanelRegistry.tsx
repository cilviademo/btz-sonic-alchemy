import React from 'react';
import type { BTZPluginState, PanelId } from '@/btz/types';

type Update = <K extends keyof BTZPluginState>(k:K, v:BTZPluginState[K])=>void;

export function renderPanel(id: PanelId, state: BTZPluginState, update: Update): React.ReactNode {
  switch (id) {
    case 'spark':
      return (
        <div className="space-y-4 text-sm">
          <div>
            <h3 className="mb-2 font-bold uppercase tracking-widest text-orange-400">
              ⚡ SPARK - Advanced Clipping Engine
            </h3>
            <p className="text-xs opacity-60 leading-relaxed">
              The magic of BTZ. Combining FL Studio Clipper + GoldClip + BigClipper + KClip + Acustica clippers
              for brutal loudness with surgical transparency. Up to 16x oversampling prevents aliasing.
            </p>
          </div>

          <div className="space-y-3">
            <label className="block">
              <div className="mb-1 flex items-center justify-between">
                <span className="text-xs uppercase tracking-wide opacity-70">Target LUFS</span>
                <span className="font-mono text-xs opacity-90">{state.sparkLUFS} LUFS</span>
              </div>
              <input
                type="range"
                min="-14"
                max="0"
                step="0.5"
                value={state.sparkLUFS}
                onChange={e=>update('sparkLUFS', Number(e.target.value))}
                className="w-full"
              />
              <div className="mt-1 text-[10px] opacity-50">
                -14 LUFS (streaming) → 0 LUFS (maximum loudness)
              </div>
            </label>

            <label className="block">
              <div className="mb-1 flex items-center justify-between">
                <span className="text-xs uppercase tracking-wide opacity-70">True Peak Ceiling</span>
                <span className="font-mono text-xs opacity-90">{state.sparkCeiling} dBTP</span>
              </div>
              <input
                type="range"
                min="-3"
                max="0"
                step="0.1"
                value={state.sparkCeiling}
                onChange={e=>update('sparkCeiling', Number(e.target.value))}
                className="w-full"
              />
              <div className="mt-1 text-[10px] opacity-50">
                Prevents inter-sample peaks for streaming platforms
              </div>
            </label>

            <label className="block">
              <div className="mb-1 flex items-center justify-between">
                <span className="text-xs uppercase tracking-wide opacity-70">Mix (Wet/Dry)</span>
                <span className="font-mono text-xs opacity-90">{Math.round(state.sparkMix * 100)}%</span>
              </div>
              <input
                type="range"
                min="0"
                max="1"
                step="0.01"
                value={state.sparkMix}
                onChange={e=>update('sparkMix', Number(e.target.value))}
                className="w-full"
              />
              <div className="mt-1 text-[10px] opacity-50">
                Blend between dry (0%) and fully clipped (100%)
              </div>
            </label>

            <label className="block">
              <div className="mb-1 text-xs uppercase tracking-wide opacity-70">Oversampling Factor</div>
              <select
                value={state.sparkOS}
                onChange={e=>update('sparkOS', Number(e.target.value) as BTZPluginState['sparkOS'])}
                className="w-full rounded bg-white/10 px-3 py-2 text-sm focus:bg-white/20 focus:outline-none focus:ring-1 focus:ring-orange-400"
              >
                <option value="1">1x (No oversampling)</option>
                <option value="2">2x (Good quality)</option>
                <option value="4">4x (High quality)</option>
                <option value="8">8x (Excellent quality - Default)</option>
                <option value="16">16x (Ultra quality - CPU intensive)</option>
              </select>
              <div className="mt-1 text-[10px] opacity-50">
                Higher values = cleaner, more transparent limiting
              </div>
            </label>

            <label className="flex items-center justify-between">
              <span className="text-xs uppercase tracking-wide opacity-70">Auto Oversampling</span>
              <button
                type="button"
                onClick={()=>update('sparkAutoOS', !state.sparkAutoOS)}
                className={`rounded px-3 py-1 text-xs uppercase transition-all ${
                  state.sparkAutoOS ? 'bg-green-500/20 text-green-400' : 'bg-white/10 hover:bg-white/20'
                }`}
              >
                {state.sparkAutoOS ? 'ON' : 'OFF'}
              </button>
            </label>

            <label className="block">
              <div className="mb-1 text-xs uppercase tracking-wide opacity-70">Clipping Mode</div>
              <select
                value={state.sparkMode}
                onChange={e=>update('sparkMode', e.target.value as BTZPluginState['sparkMode'])}
                className="w-full rounded bg-white/10 px-3 py-2 text-sm focus:bg-white/20 focus:outline-none focus:ring-1 focus:ring-orange-400"
              >
                <option value="soft">Soft (Musical, warm saturation)</option>
                <option value="hard">Hard (Aggressive, punchy attack)</option>
              </select>
            </label>
          </div>

          <div className="rounded bg-orange-500/10 p-3 text-xs">
            <strong className="text-orange-400">💡 Pro Tip:</strong> For streaming platforms like Spotify,
            use -14 LUFS with -0.3 dBTP ceiling. For club/DJ use, push to -9 LUFS with soft mode.
          </div>
        </div>
      );
    case 'shine':
      return (
        <div className="space-y-4 text-sm">
          <div>
            <h3 className="mb-2 font-bold uppercase tracking-widest text-cyan-400">
              ✨ SHINE - Ultra-High Frequency Air
            </h3>
            <p className="text-xs opacity-60 leading-relaxed">
              SSL Fusion Air + Maag EQ Air Band emulation. Extends beyond 48kHz Nyquist limit via oversampling,
              adding ethereal highs, crystalline crispness, and analog-inspired sheen up to 80kHz.
            </p>
          </div>

          <div className="space-y-3">
            <label className="block">
              <div className="mb-1 flex items-center justify-between">
                <span className="text-xs uppercase tracking-wide opacity-70">Center Frequency</span>
                <span className="font-mono text-xs opacity-90">
                  {state.shineFreqHz >= 1000 ? `${(state.shineFreqHz/1000).toFixed(1)}kHz` : `${state.shineFreqHz}Hz`}
                </span>
              </div>
              <input
                type="range"
                min="10000"
                max="80000"
                step="1000"
                value={state.shineFreqHz}
                onChange={e=>update('shineFreqHz', Number(e.target.value))}
                className="w-full"
              />
              <div className="mt-1 text-[10px] opacity-50">
                10kHz (sparkle) → 80kHz (ultrasonic air)
              </div>
            </label>

            <label className="block">
              <div className="mb-1 flex items-center justify-between">
                <span className="text-xs uppercase tracking-wide opacity-70">Gain</span>
                <span className="font-mono text-xs opacity-90">
                  {state.shineGainDb > 0 ? '+' : ''}{state.shineGainDb.toFixed(1)} dB
                </span>
              </div>
              <input
                type="range"
                min="-12"
                max="12"
                step="0.5"
                value={state.shineGainDb}
                onChange={e=>update('shineGainDb', Number(e.target.value))}
                className="w-full"
              />
              <div className="mt-1 text-[10px] opacity-50">
                Boost or cut high frequencies
              </div>
            </label>

            <label className="block">
              <div className="mb-1 flex items-center justify-between">
                <span className="text-xs uppercase tracking-wide opacity-70">Q (Resonance)</span>
                <span className="font-mono text-xs opacity-90">{state.shineQ.toFixed(2)}</span>
              </div>
              <input
                type="range"
                min="0.1"
                max="2.0"
                step="0.1"
                value={state.shineQ}
                onChange={e=>update('shineQ', Number(e.target.value))}
                className="w-full"
              />
              <div className="mt-1 text-[10px] opacity-50">
                0.1 (broad, natural sheen) → 2.0 (focused, crisp peak)
              </div>
            </label>

            <label className="block">
              <div className="mb-1 flex items-center justify-between">
                <span className="text-xs uppercase tracking-wide opacity-70">Mix (Wet/Dry)</span>
                <span className="font-mono text-xs opacity-90">{Math.round(state.shineMix * 100)}%</span>
              </div>
              <input
                type="range"
                min="0"
                max="1"
                step="0.01"
                value={state.shineMix}
                onChange={e=>update('shineMix', Number(e.target.value))}
                className="w-full"
              />
              <div className="mt-1 text-[10px] opacity-50">
                Blend between dry and enhanced signal
              </div>
            </label>

            <label className="flex items-center justify-between">
              <span className="text-xs uppercase tracking-wide opacity-70">Auto Oversampling</span>
              <button
                type="button"
                onClick={()=>update('shineAutoOS', !state.shineAutoOS)}
                className={`rounded px-3 py-1 text-xs uppercase transition-all ${
                  state.shineAutoOS ? 'bg-green-500/20 text-green-400' : 'bg-white/10 hover:bg-white/20'
                }`}
              >
                {state.shineAutoOS ? 'ON' : 'OFF'}
              </button>
            </label>
          </div>

          <div className="rounded bg-cyan-500/10 p-3 text-xs">
            <strong className="text-cyan-400">💡 Pro Tip:</strong> Start with 20kHz @ +3dB for gentle air.
            For vocals, try 12-15kHz. For cymbals/hi-hats, push to 25-30kHz for ethereal shimmer.
          </div>
        </div>
      );
    case 'master':
      return (
        <div className="space-y-3 text-sm">
          <div className="font-semibold uppercase tracking-widest">Master</div>
          <label className="block">Blend
            <select className="ml-2 bg-white/5 px-2 py-1"
              value={state.masterBlend}
              onChange={e=>update('masterBlend', e.target.value as BTZPluginState['masterBlend'])}
            >
              <option value="transparent">Transparent</option>
              <option value="glue">Glue</option>
              <option value="vintage">Vintage</option>
            </select>
          </label>
        </div>
      );
    case 'meters':
      return <div className="text-sm opacity-80">Detailed metering coming soon.</div>;
    case 'convolver':
      return <div className="text-sm opacity-80">Sub/Convolver settings.</div>;
    case 'deep':
      return <div className="text-sm opacity-80">Deep parameter view.</div>;
    default:
      return null;
  }
}