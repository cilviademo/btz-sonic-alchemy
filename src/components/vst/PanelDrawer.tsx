import React from 'react';
import { cn } from '@/lib/utils';

export const PanelDrawer: React.FC<{
  open: boolean;
  title?: string;
  onClose: () => void;
  children: React.ReactNode;
}> = ({ open, title, onClose, children }) => {
  return (
    <>
      {/* dimmer */}
      <div
        className={cn(
          'fixed inset-0 z-40 transition-opacity',
          open ? 'bg-black/50 opacity-100' : 'pointer-events-none opacity-0'
        )}
        onClick={onClose}
      />
      {/* drawer */}
      <aside
        className={cn(
          'fixed top-0 right-0 z-50 h-full w-[560px] max-w-[92vw] translate-x-full',
          'bg-[linear-gradient(180deg,#151a23,#0f141c)] border-l border-white/10',
          'shadow-[0_0_40px_rgba(0,0,0,.5)] transition-transform duration-300'
        )}
        style={{ transform: open ? 'translateX(0)' : undefined }}
        aria-hidden={!open}
      >
        <header className="flex items-center justify-between px-5 py-4 border-b border-white/10">
          <div className="text-[11px] tracking-[.22em] text-white/70">{title ?? 'DETAILS'}</div>
          <button
            className="px-3 py-1 rounded bg-white/10 hover:bg-white/15 text-sm"
            onClick={onClose}
          >
            Close
          </button>
        </header>
        <div className="p-5 overflow-y-auto h-[calc(100%-56px)]">{children}</div>
      </aside>
    </>
  );
};
