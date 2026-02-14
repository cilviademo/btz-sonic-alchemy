import React from 'react';

export const OverlayGrid: React.FC<{show:boolean}> = ({ show }) => {
  if (!show) return null;
  return <div className="plugin-grid-overlay" />;
};
