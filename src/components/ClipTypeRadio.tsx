import * as React from 'react';
const TYPES=[{id:'soft',label:'Soft',icon:'∿'},{id:'hard',label:'Hard',icon:'▷'},{id:'tube',label:'Tube',icon:'◯'},
             {id:'tape',label:'Tape',icon:'▢'},{id:'digital',label:'Digital',icon:'▣'}] as const;
export function ClipTypeRadio({ value, onChange }:{value:string; onChange:(v:string)=>void;}){
  return (
    <div role="radiogroup" aria-label="Clipping Type" className="grid grid-cols-3 sm:grid-cols-5 gap-10 sm:gap-6">
      {TYPES.map(t=>{
        const active=value===t.id;
        return (
          <button key={t.id} role="radio" aria-checked={active} onClick={()=>onChange(t.id)}
            className="h-14 w-24 rounded-2xl flex flex-col items-center justify-center gap-1 border transition shadow-sm focus:outline-none"
            style={{
              background: active ? 'var(--btz-green)' : 'var(--btz-panel-2)',
              color: active ? '#04110a' : 'var(--btz-ink)',
              borderColor: active ? 'transparent' : 'rgba(255,255,255,.06)',
              boxShadow: active ? '0 0 0 3px rgba(57,255,136,.25), 0 8px 30px rgba(0,0,0,.4)' : 'none'
            }}>
            <span className="text-[18px] leading-none">{t.icon}</span>
            <span className="text-[11px] font-semibold tracking-wide">{t.label}</span>
          </button>
        );
      })}
    </div>
  );
}
