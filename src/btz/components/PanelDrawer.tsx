import React, { useEffect } from 'react';
import { cn } from '@/lib/utils';

export const PanelDrawer: React.FC<{
  open: boolean;
  title?: string;
  onClose: () => void;
  children: React.ReactNode;
}> = ({ open, title, onClose, children }) => {
  useEffect(() => {
    if (!open) return;
    const on = (e: KeyboardEvent) => { if (e.key === 'Escape') onClose(); };
    window.addEventListener('keydown', on);
    return () => window.removeEventListener('keydown', on);
  }, [open, onClose]);

  return (
    <>
      <div
        className={cn(
          'fixed inset-0 bg-black/40 transition-opacity',
          open ? 'opacity-100' : 'pointer-events-none opacity-0'
        )}
        aria-hidden="true"
        onClick={onClose}
      />
      <aside
        role="dialog"
        aria-modal="true"
        aria-labelledby="btz-panel-title"
        className={cn(
          'fixed right-0 top-0 h-full w-[420px] bg-neutral-900 shadow-xl transition-transform',
          open ? 'translate-x-0' : 'translate-x-full'
        )}
      >
        <header className="btz-drawer-header flex items-center justify-between px-4 py-3">
          <h2 id="btz-panel-title" className="btz-label-xs">
            {title ?? 'DETAILS'}
          </h2>
          <button onClick={onClose} aria-label="Close panel" className="text-xs uppercase opacity-80 hover:opacity-100 transition-opacity">
            ✕ Close
          </button>
        </header>
        <div className="btz-scrollbar h-full overflow-auto p-4 pb-20">{children}</div>
      </aside>
    </>
  );
};