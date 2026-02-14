import React from 'react';
import { cn } from '@/lib/utils';

export const ArturiaFrame: React.FC<
  React.PropsWithChildren<{ className?: string; title?: string; subtitle?: string }>
> = ({ className, children, title, subtitle }) => {
  return (
    <section className={cn('relative p-5 sm:p-6 art-plate text-[var(--panel-ink)]', className)}>
      {/* screws */}
      <div className="art-screw" style={{ left: 10, top: 10 }} />
      <div className="art-screw" style={{ right: 10, top: 10 }} />
      <div className="art-screw" style={{ left: 10, bottom: 10 }} />
      <div className="art-screw" style={{ right: 10, bottom: 10 }} />

      {(title || subtitle) && (
        <header className="mb-4 flex items-end justify-between">
          <div>
            <h3 className="tracking-[.18em] font-black text-[13px]">{title}</h3>
            {subtitle && (
              <p className="text-xs opacity-70 tracking-wide">{subtitle}</p>
            )}
          </div>
          <div className="text-[11px] tracking-[.22em] font-semibold opacity-60">
            BTZ • BOX TONE ZONE
          </div>
        </header>
      )}

      {children}
    </section>
  );
};
