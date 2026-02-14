import React from 'react';
import { cn } from '@/lib/utils';
import { ThermalKnob } from './ThermalKnob';

export const ModKnob:React.FC<{
  label:string; on:boolean; value:number; setOn:(v:boolean)=>void; setVal:(v:number)=>void;
  colorA:string; colorB:string; disabled?:boolean; open?:()=>void;
}> = ({label,on,value,setOn,setVal,colorA,colorB,disabled,open})=>{
  return (
    <div className={cn("flex items-center gap-3", disabled && "opacity-40")}>
      <button
        type="button"
        className="rounded bg-white/5 px-2 py-1 text-[10px] uppercase tracking-widest hover:bg-white/10 disabled:cursor-not-allowed"
        aria-pressed={on}
        aria-disabled={disabled || undefined}
        disabled={disabled}
        onClick={()=>!disabled && setOn(!on)}
      >
        {on ? 'On' : 'Off'}
      </button>
      <div className="w-28">
        <ThermalKnob
          label={label}
          value={value}
          onChange={setVal}
          colorA={colorA}
          colorB={colorB}
          size={90}
          toDisplay={(v)=>`${Math.round(v*100)}%`}
        />
      </div>
      <button
        type="button"
        className="ml-auto text-[10px] uppercase opacity-70 hover:opacity-100"
        onClick={open}
      >
        Details
      </button>
    </div>
  );
};