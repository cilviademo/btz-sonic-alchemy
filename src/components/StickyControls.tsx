import React from 'react';

type Props = {
  left?: React.ReactNode;
  right?: React.ReactNode;
  active?: boolean;
  audioRunning?: boolean;
  onTogglePower?: () => void;
  onEngineClick?: () => void;
  lufs?: number;
  peak?: number;
};

export function StickyControls(props: Props){
  const { left, right, active, audioRunning, onTogglePower, onEngineClick, lufs, peak } = props;
  const hasSmart = onEngineClick != null || onTogglePower != null;
  return (
    <div className="fixed bottom-0 left-0 right-0 z-40 px-3 pb-[env(safe-area-inset-bottom)] md:hidden">
      <div className="mx-auto max-w-4xl rounded-2xl border border-white/10 backdrop-blur-md"
        style={{ background:'rgba(10,10,16,.72)', boxShadow:'var(--btz-shadow)' }}>
        <div className="flex items-center justify-between gap-3 p-3">
          <div className="flex items-center gap-2">
            {left ?? (hasSmart && (
              <>
                <button
                  onClick={onEngineClick}
                  className="px-3 py-2 rounded-lg text-xs font-semibold border border-foreground/20 bg-plugin-panel hover:bg-plugin-surface transition"
                  aria-pressed={!!audioRunning}
                >
                  {audioRunning ? 'Audio On' : 'Enable Audio'}
                </button>
                <button
                  onClick={onTogglePower}
                  className="px-3 py-2 rounded-lg text-xs font-semibold border border-foreground/20 bg-plugin-panel hover:bg-plugin-surface transition"
                  aria-pressed={!!active}
                >
                  {active ? 'Power On' : 'Power Off'}
                </button>
              </>
            ))}
          </div>
          <div className="flex items-center gap-3">
            {hasSmart && (
              <div className="text-[11px] text-foreground/70">
                <span className="mr-3">LUFS: {lufs?.toFixed?.(1) ?? '—'}</span>
                <span>Peak: {peak?.toFixed?.(1) ?? '—'}</span>
              </div>
            )}
            {right}
          </div>
        </div>
      </div>
    </div>
  );
}
