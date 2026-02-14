import React, { useEffect, useRef } from 'react';

export const CentralVisualizerCanvas: React.FC<{
  level: number;            // 0..1
  spectrum?: Float32Array;  // optional
}> = ({ level, spectrum }) => {
  const ref = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    if (!ref.current) return;
    const ctx = ref.current.getContext('2d');
    if (!ctx) return;

    let raf = 0;
    const W = ref.current.width, H = ref.current.height;
    const cx = W/2, cy = H/2;

    const draw = (t:number) => {
      raf = requestAnimationFrame(draw);
      ctx.clearRect(0,0,W,H);

      // vignette / portal glow
      const g = ctx.createRadialGradient(cx, cy, 0, cx, cy, Math.max(W,H)*0.6);
      g.addColorStop(0, 'rgba(63,255,172,.16)');
      g.addColorStop(.45, 'rgba(147,51,234,.06)');
      g.addColorStop(1, 'rgba(0,0,0,0)');
      ctx.fillStyle = g;
      ctx.fillRect(0,0,W,H);

      const ring = (r:number, c:string, lw:number) => {
        ctx.beginPath();
        ctx.arc(cx, cy, r, 0, Math.PI*2);
        ctx.lineWidth = lw;
        ctx.strokeStyle = c;
        ctx.shadowColor = c;
        ctx.shadowBlur = 16;
        ctx.stroke();
        ctx.shadowBlur = 0;
      };

      ring(Math.min(W,H)*0.28, 'rgba(57,255,136,.85)', 5);
      ring(Math.min(W,H)*0.18, 'rgba(255,47,185,.65)', 4);

      // orbiting dots
      for (let i=0;i<24;i++){
        const p = (i/24)*Math.PI*2 + t*0.001 + i*0.2;
        const r = Math.min(W,H)*(0.22 + 0.05*Math.sin(t*0.001 + i));
        const x = cx + r*Math.cos(p), y = cy + r*Math.sin(p);
        ctx.fillStyle = i%2 ? '#ff8a00' : '#2fd3ff';
        ctx.globalAlpha = .8;
        ctx.beginPath(); ctx.arc(x,y, 2.2+(level*1.8), 0, Math.PI*2); ctx.fill();
        ctx.globalAlpha = 1;
      }

      // optional horizontal spectrum trace
      if (spectrum) {
        ctx.save();
        ctx.translate(0, H*0.84);
        ctx.strokeStyle = 'rgba(255,47,185,.7)';
        ctx.lineWidth = 2;
        ctx.beginPath();
        for (let i=0;i<spectrum.length;i++){
          const x = (i/(spectrum.length-1))*W;
          const y = -20 - spectrum[i]*60;
          if (i === 0) ctx.moveTo(x,y); else ctx.lineTo(x,y);
        }
        ctx.stroke();
        ctx.restore();
      }
    };

    raf = requestAnimationFrame(draw);
    return () => cancelAnimationFrame(raf);
  }, [level, spectrum]);

  return (
    <canvas
      ref={ref}
      width={880}
      height={360}
      className="w-full h-[280px] md:h-[320px] rounded-2xl bg-[#0c1117]/60 border border-white/5"
    />
  );
};
