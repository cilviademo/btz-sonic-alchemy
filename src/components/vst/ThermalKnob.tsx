import React, { useRef, useState } from 'react';

type Props = {
  value: number; // 0..1
  onChange: (v:number)=>void;
  label?: string;
  spectrumData?: Float32Array;
  waveformData?: Float32Array;
  colorA?: string; colorB?: string;
};

export const ThermalKnob: React.FC<Props> = ({
  value, onChange, label, spectrumData, waveformData, colorA = '#ff6a00', colorB = '#ff00a8'
}) => {
  const ref = useRef<HTMLDivElement>(null);
  const [drag, setDrag] = useState<{ y: number; start: number } | null>(null);
  const [hover, setHover] = useState(false);
  const [ripple, setRipple] = useState<{x:number;y:number}|null>(null);

  const startDrag = (e: React.MouseEvent) => {
    setDrag({ y: e.clientY, start: value });
    const rect = (e.currentTarget as HTMLDivElement).getBoundingClientRect();
    setRipple({ x: e.clientX - rect.left, y: e.clientY - rect.top });
    setTimeout(()=>setRipple(null), 350);
  };
  const moveDrag = (e: React.MouseEvent) => {
    if (!drag) return;
    const dy = drag.y - e.clientY;
    const next = Math.min(1, Math.max(0, drag.start + dy / 220)); // sensitivity
    onChange(next);
  };
  const endDrag = () => { setDrag(null); setHover(false); };

  const pct = Math.round((value ?? 0) * 100);
  const ring = `conic-gradient(${colorA} ${pct}%, #31363d ${pct}%)`;
  const glow = hover ? `0 0 28px ${colorB}55` : `0 0 18px ${colorB}33`;

  return (
    <div className="group select-none flex flex-col items-center">
      <div
        ref={ref}
        onMouseDown={startDrag}
        onMouseMove={moveDrag}
        onMouseUp={endDrag}
        onMouseLeave={endDrag}
        onDoubleClick={() => onChange(0)}
        onMouseEnter={() => setHover(true)}
        
        className="relative w-[110px] h-[110px] rounded-full grid place-items-center cursor-[ns-resize] transition-shadow"
        style={{ background: ring, boxShadow: glow }}
        aria-label={label}
      >
        <div className="w-[92px] h-[92px] rounded-full bg-[#151a1f] border border-white/5 grid place-items-center relative overflow-hidden">
          {/* Thermal shimmer */}
          <div className="absolute inset-0" style={{
            background: 'radial-gradient(60% 60% at 50% 50%, rgba(255,255,255,0.06) 0%, transparent 70%)'
          }}/>
          <div className="w-[10px] h-[10px] rounded-full bg-white/85" />
        </div>
        {/* Ripple */}
        {ripple && (
          <span
            className="absolute block rounded-full pointer-events-none"
            style={{
              left: ripple.x - 2,
              top: ripple.y - 2,
              width: 4,
              height: 4,
              background: '#ffffff55',
              boxShadow: `0 0 30px ${colorB}aa`,
              animation: 'knob-ripple 0.35s ease-out forwards'
            }}
          />
        )}
      </div>
      {/* Live tooltip */}
      <div className="mt-2 text-[11px] tracking-[.18em] text-foreground/80 uppercase">{label}</div>
      <div className="text-[11px] text-foreground/60">{pct}%</div>
      <style>{`@keyframes knob-ripple{from{transform:scale(1);opacity:0.9}to{transform:scale(14);opacity:0}}`}</style>
    </div>
  );
};
