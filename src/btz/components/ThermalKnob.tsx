import React, { useMemo, useState } from 'react';
import { cn } from '@/lib/utils';

type Props = {
  label?: string;
  value: number;               // 0..1
  onChange: (v:number)=>void;
  colorA?: string;             // ring start
  colorB?: string;             // ring end
  size?: number;               // px
  fine?: number;               // fine multiplier (default .25)
  disabled?: boolean;
  toDisplay?: (v:number)=>string; // optional display formatter
};

const clamp01 = (x:number)=> Math.max(0, Math.min(1, x));

export const ThermalKnob: React.FC<Props> = ({
  label, value, onChange,
  colorA='#ff8a00', colorB='#2fd3ff',
  size=112, fine=.25, disabled,
  toDisplay
}) => {
  const [drag, setDrag] = useState<{y:number; v:number; fine:boolean} | null>(null);
  const [hover, setHover] = useState(false);

  const r = (size/2) - 10 * 1.2;
  const circ = 2*Math.PI*r;
  const arc = circ * value;

  const display = useMemo(
    () => toDisplay ? toDisplay(value) : `${Math.round(value*100)}%`,
    [toDisplay, value]
  );

  const onPointerDown = (e: React.PointerEvent) => {
    if (disabled) return;
    (e.currentTarget as HTMLElement).setPointerCapture(e.pointerId);
    setDrag({ y: e.clientY, v: value, fine: e.shiftKey });
    (e.currentTarget as HTMLElement).focus();
  };
  const onPointerMove = (e: React.PointerEvent) => {
    if (!drag || disabled) return;
    const dy = drag.y - e.clientY;
    const sens = (drag.fine || e.shiftKey) ? (0.003*(fine ?? .25)) : 0.01;
    onChange(clamp01(drag.v + dy*sens));
  };
  const onPointerUp = (e: React.PointerEvent) => {
    if (!drag) return;
    (e.currentTarget as HTMLElement).releasePointerCapture(e.pointerId);
    setDrag(null);
  };

  const onWheel = (e: React.WheelEvent) => {
    if (disabled) return;
    e.preventDefault();
    const delta = e.shiftKey ? -e.deltaY * 0.0008 * (fine ?? .25) : -e.deltaY * 0.0032;
    onChange(clamp01(value + delta));
  };

  const onKeyDown = (e: React.KeyboardEvent) => {
    if (disabled) return;
    if (e.key === 'ArrowUp' || e.key === 'ArrowRight') { onChange(clamp01(value + (e.shiftKey? .005 : .02))); e.preventDefault(); }
    if (e.key === 'ArrowDown' || e.key === 'ArrowLeft') { onChange(clamp01(value - (e.shiftKey? .005 : .02))); e.preventDefault(); }
    if ((e.key === '0' && (e.ctrlKey || e.metaKey)) || e.key === 'Enter') { onChange(0); e.preventDefault(); }
  };

  return (
    <div
      className={cn(
        'btz-knob inline-flex select-none flex-col items-center outline-none',
        disabled && 'opacity-40 cursor-not-allowed'
      )}
      role="slider"
      aria-label={label}
      aria-valuemin={0}
      aria-valuemax={1}
      aria-valuenow={Number(value.toFixed(3))}
      tabIndex={disabled ? -1 : 0}
      onPointerDown={onPointerDown}
      onPointerMove={onPointerMove}
      onPointerUp={onPointerUp}
      onWheel={onWheel}
      onKeyDown={onKeyDown}
      onDoubleClick={()=> !disabled && onChange(0)}
      onMouseEnter={()=>setHover(true)}
      onMouseLeave={()=>setHover(false)}
      style={{ width: size, userSelect: 'none' }}
    >
      <svg width={size} height={size} viewBox={`0 0 ${size} ${size}`}>
        <defs>
          <linearGradient id="btzGrad" x1="0%" y1="0%" x2="100%" y2="0%">
            <stop offset="0%" stopColor={colorA}/>
            <stop offset="100%" stopColor={colorB}/>
          </linearGradient>
        </defs>
        {/* rail */}
        <circle cx={size/2} cy={size/2} r={r} stroke="rgba(255,255,255,.12)" strokeWidth={10} fill="none" />
        {/* arc */}
        <circle
          cx={size/2} cy={size/2} r={r}
          stroke="url(#btzGrad)" strokeWidth={10} fill="none"
          strokeDasharray={`${arc} ${circ-arc}`} strokeLinecap="round"
          transform={`rotate(-90 ${size/2} ${size/2})`}
        />
        {/* pointer */}
        <circle
          cx={size/2} cy={size/2 - r}
          r={5}
          fill={disabled ? 'gray' : '#fff'}
          transform={`rotate(${value*360} ${size/2} ${size/2})`}
        />
      </svg>

      {label && (
        <div className="mt-2 text-center text-xs leading-tight">
          <div className="font-semibold tracking-wide">{label}</div>
          <div className="opacity-80">{display}</div>
          {hover && (
            <div className="opacity-60">Shift=fine • Wheel/Arrows • ⌘0/Enter=Reset</div>
          )}
        </div>
      )}
    </div>
  );
};