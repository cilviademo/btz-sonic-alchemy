import React, { useEffect, useState } from 'react';

type Props = {
  show?: boolean;
  opacity?: number; // 0..1
  stroke?: string;  // CSS color for lines (uses currentColor if omitted)
};

export const GridOverlay: React.FC<Props> = ({ show = false, opacity = 0.5, stroke = 'currentColor' }) => {
  const [visible, setVisible] = useState(show);
  const [alpha, setAlpha] = useState(opacity);

  // Hotkeys: G toggle, Shift+G cycle opacity
  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if (e.key.toLowerCase() === 'g' && !e.metaKey && !e.ctrlKey && !e.altKey) {
        if (e.shiftKey) {
          setAlpha((a) => (a >= 0.9 ? 0.25 : Math.min(1, a + 0.25)));
        } else {
          setVisible((v) => !v);
        }
        e.preventDefault();
      }
    };
    window.addEventListener('keydown', onKey);
    return () => window.removeEventListener('keydown', onKey);
  }, []);

  if (!visible) return null;

  return (
    <div
      aria-hidden
      style={{
        position: 'absolute',
        inset: 0,
        pointerEvents: 'none',
        zIndex: 9999,
        color: stroke,
        opacity: alpha,
        mixBlendMode: 'difference', // pops on dark/light
      }}
    >
      <img
        src={`/btz_grid_1200x700.svg`}
        alt=""
        style={{ width: '100%', height: '100%', objectFit: 'fill' }}
      />
    </div>
  );
};