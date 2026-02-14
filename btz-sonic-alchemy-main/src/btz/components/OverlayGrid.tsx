import React from 'react';

export const OverlayGrid: React.FC<{ show: boolean }> = ({ show }) => {
  if (!show) return null;
  return (
    <div className="pointer-events-none fixed inset-0 grid grid-cols-12 gap-4 opacity-20">
      {[...Array(12)].map((_, i) => (
        <div key={i} className="border-l border-white/30" />
      ))}
    </div>
  );
};