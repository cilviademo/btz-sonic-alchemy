import React, { useEffect, useRef } from 'react';

export const CentralVisualizerCanvas: React.FC<{ peakNorm: number }> = ({ peakNorm }) => {
  const ref = useRef<HTMLCanvasElement|null>(null);

  useEffect(() => {
    const cvs = ref.current; if (!cvs) return;
    const ctx = cvs.getContext('2d'); if (!ctx) return;
    let raf = 0;

    const draw = () => {
      raf = requestAnimationFrame(draw);
      const w = cvs.width, h = cvs.height;
      ctx.clearRect(0,0,w,h);
      const amp = peakNorm * h * 0.8;
      ctx.fillStyle = '#111';
      ctx.fillRect(0,0,w,h);
      ctx.fillStyle = '#39ff88';
      ctx.fillRect(0, h/2 - amp/2, w, amp);
    };
    draw();
    return () => cancelAnimationFrame(raf);
  }, [peakNorm]);

  return <canvas ref={ref} width={640} height={160} className="w-full rounded bg-black/50" />;
};