import React, { useRef, useEffect, useState, useCallback } from 'react';
import { cn } from '@/lib/utils';

type Props = {
  label?: string;
  value: number;           // 0..1
  onChange: (v:number)=>void;
  colorA?: string;         // ring start
  colorB?: string;         // ring end
  size?: number;           // px
  fine?: number;           // fine sensitivity multiplier (default .25)
  disabled?: boolean;
};

export const ThermalKnob: React.FC<Props> = ({
  label, value, onChange, colorA='#ff8a00', colorB='#2fd3ff', size=112, fine=.25, disabled
}) => {
  const [drag, setDrag] = useState<{y:number; v:number; fine:boolean}|null>(null);
  const [hover, setHover] = useState(false);
  const clamp = (x:number) => Math.max(0, Math.min(1, x));

  const start = (e:React.MouseEvent) => {
    e.preventDefault();
    const y = 'touches' in e ? (e as any).touches?.[0]?.clientY : e.clientY;
    setDrag({ y, v:value, fine: e.shiftKey });
    (e.currentTarget as HTMLElement).focus();
  };

  const move = useCallback((e:MouseEvent) => {
    setDrag(prev => {
      if (!prev) return prev;
      const y = e.clientY;
      const dy = prev.y - y;
      const sensitivity = (prev.fine || e.shiftKey) ? (0.003*fine) : 0.01;
      const next = clamp(prev.v + dy * sensitivity);
      onChange(next);
      return { ...prev, y };
    });
  }, [onChange, fine]);

  const up = useCallback(() => setDrag(null), []);
  useEffect(() => {
    if (!drag) return;
    window.addEventListener('mousemove', move);
    window.addEventListener('mouseup', up);
    return () => { window.removeEventListener('mousemove', move); window.removeEventListener('mouseup', up); };
  }, [drag, move, up]);

  const onWheel = (e:React.WheelEvent) => {
    e.preventDefault();
    const delta = e.shiftKey ? -e.deltaY * 0.0008 * fine : -e.deltaY * 0.0032;
    onChange(clamp(value + delta));
  };

  const onKey = (e:React.KeyboardEvent) => {
    if (e.key === 'ArrowUp' || e.key === 'ArrowRight') { onChange(clamp(value + (e.shiftKey? .005 : .02))); e.preventDefault(); }
    if (e.key === 'ArrowDown' || e.key === 'ArrowLeft') { onChange(clamp(value - (e.shiftKey? .005 : .02))); e.preventDefault(); }
    if (e.key === '0' && (e.ctrlKey || e.metaKey)) { onChange(0); e.preventDefault(); }
  };

  const stroke = 10;
  const r = (size/2) - stroke*1.2;
  const circ = 2*Math.PI*r;
  const arc = circ * value;

  return (
    <div className="select-none text-center" style={{width:size}}>
      <button
        className={cn("relative block rounded-full outline-none focus:ring-2 focus:ring-white/30",
          disabled && "opacity-60")}
        style={{width:size, height:size}}
        onMouseDown={start} onWheel={onWheel} onKeyDown={onKey}
        onDoubleClick={()=>onChange(0.0)}
        onMouseEnter={()=>setHover(true)} onMouseLeave={()=>setHover(false)}
      >
        <svg width={size} height={size} viewBox={`0 0 ${size} ${size}`}>
          <defs>
            <linearGradient id="g1" x1="0%" y1="0%" x2="100%" y2="100%">
              <stop offset="0%" stopColor={colorA}/>
              <stop offset="100%" stopColor={colorB}/>
            </linearGradient>
          </defs>
          <circle cx={size/2} cy={size/2} r={r} stroke="#222a" strokeWidth={stroke} fill="none"/>
          <circle cx={size/2} cy={size/2} r={r}
                  stroke="url(#g1)" strokeLinecap="round"
                  strokeWidth={stroke} fill="none"
                  strokeDasharray={`${arc} ${circ-arc}`}
                  transform={`rotate(-90 ${size/2} ${size/2})`}
                  style={{filter:'drop-shadow(0 0 16px rgba(255,140,0,.35))'}}/>
          <circle cx={size/2} cy={size/2} r={r-stroke*0.85} fill="#0b0f15"/>
          {/* pointer dot */}
          <g transform={`rotate(${value*300-150} ${size/2} ${size/2})`}>
            <circle cx={size/2} cy={size/2 - (r-stroke*0.9)} r={6} fill="url(#g1)"/>
          </g>
        </svg>
      </button>
      {label && (
        <div className="mt-2">
          <div className="text-[10px] tracking-[.22em] text-white/70">{label}</div>
          <div className="text-[10px] text-white/60">{Math.round(value*100)}%</div>
          {hover && <div className="text-[9px] text-white/35">Shift=Fine • Wheel/Arrows • ⌘0=Reset</div>}
        </div>
      )}
    </div>
  );
};