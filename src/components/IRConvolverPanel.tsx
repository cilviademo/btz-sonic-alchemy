import React, { useState } from 'react';
import { Slider } from '@/components/ui/slider';

export function IRConvolverPanel({
  loadIRFromUrl, loadIRFromArrayBuffer, setWet, setDry, setPreDelay, setHP, setLP, setDamp,
  currentWet=.18, currentDry=.82
}:{
  loadIRFromUrl:(u:string)=>Promise<void>; loadIRFromArrayBuffer:(a:ArrayBuffer)=>Promise<void>;
  setWet:(v:number)=>void; setDry:(v:number)=>void; setPreDelay:(ms:number)=>void; setHP:(hz:number)=>void; setLP:(hz:number)=>void; setDamp:(hz:number)=>void;
  currentWet?:number; currentDry?:number;
}){
  const [hover,setHover]=useState(false);
  const presets=[
    {id:'room-small',name:'Room – Small',url:'/irs/room_small.wav'},
    {id:'room-medium',name:'Room – Medium',url:'/irs/room_medium.wav'},
    {id:'mic-boom',name:'Mic – Boom',url:'/irs/mic_boom.wav'},
    {id:'plate',name:'Plate – Classic',url:'/irs/plate_classic.wav'},
    {id:'spring',name:'Spring – Vintage',url:'/irs/spring_vintage.wav'},
    {id:'vintage-hall',name:'Vintage Hall',url:'/irs/vintage_hall.wav'},
  ];
  const onDrop=async(e:React.DragEvent)=>{ e.preventDefault(); setHover(false);
    const f=e.dataTransfer.files?.[0]; if(!f) return; const buf=await f.arrayBuffer(); await loadIRFromArrayBuffer(buf); };
  return (
    <div className="rounded-2xl border border-white/8" style={{background:'var(--btz-panel)'}}>
      <div className="flex items-center justify-between p-5 sm:p-6 border-b border-white/5">
        <div>
          <h3 className="text-lg font-bold" style={{color:'var(--btz-ink)'}}>Convolution Reverb</h3>
          <p className="text-xs" style={{color:'var(--btz-ink-muted)'}}>Drag & drop IR WAVs or use presets</p>
        </div>
      </div>
      <div className="p-5 sm:p-6 space-y-6">
        <div onDragOver={(e)=>{e.preventDefault();setHover(true);}} onDragLeave={()=>setHover(false)} onDrop={onDrop}
          className={`rounded-xl border-2 p-6 text-center transition ${hover?'border-[var(--btz-green)]/80 bg-[var(--btz-green)]/10':'border-white/10 bg-[var(--btz-panel-2)]/40'}`}>
          <div className="text-xs opacity-80 mb-3">Drop an IR file here (.wav)</div>
          <div className="flex flex-wrap gap-2 justify-center">
            {presets.map(p=>(
              <button key={p.id} onClick={()=>loadIRFromUrl(p.url)}
                className="px-3 py-2 rounded-lg text-xs border border-white/10 hover:border-[var(--btz-blue)]/50 hover:bg-[var(--btz-blue)]/10 transition">
                {p.name}
              </button>
            ))}
          </div>
        </div>

        <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
          <Row label="Wet"><Slider value={[Math.round(currentWet*100)]} onValueChange={v=>setWet((v[0]??0)/100)} /></Row>
          <Row label="Dry"><Slider value={[Math.round(currentDry*100)]} onValueChange={v=>setDry((v[0]??0)/100)} /></Row>
          <Row label="Pre‑Delay (ms)"><Slider max={120} step={1} value={[0]} onValueChange={v=>setPreDelay(v[0]??0)} /></Row>
          <Row label="Pre HP (Hz)"><Slider max={1000} step={5} value={[100]} onValueChange={v=>setHP(v[0]??100)} /></Row>
          <Row label="Pre LP (Hz)"><Slider min={1000} max={20000} step={50} value={[16000]} onValueChange={v=>setLP(v[0]??16000)} /></Row>
          <Row label="Damp (Hz)"><Slider min={2000} max={20000} step={50} value={[14000]} onValueChange={v=>setDamp(v[0]??14000)} /></Row>
        </div>
      </div>
    </div>
  );
}
function Row({label,children}:{label:string;children:React.ReactNode}) {
  return (
    <div className="flex items-center gap-3">
      <span className="w-28 text-[11px] opacity-80">{label}</span>
      <div className="flex-1">{children}</div>
    </div>
  );
}
