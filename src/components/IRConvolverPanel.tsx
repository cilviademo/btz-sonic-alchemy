import React, { useRef } from 'react';
import { ArturiaFrame } from '@/components/arturia/ArturiaFrame';
import { Slider } from '@/components/ui/slider';

export const IRConvolverPanel: React.FC<{
  loadIRFromUrl: (url: string)=>Promise<void>|void;
  loadIRFromArrayBuffer: (buf: ArrayBuffer)=>Promise<void>|void;
  setWet: (v:number)=>void; setDry:(v:number)=>void;
  setPreDelay: (ms:number)=>void; setHP:(hz:number)=>void; setLP:(hz:number)=>void; setDamp:(v:number)=>void;
}> = (p) => {
  const fileRef = useRef<HTMLInputElement>(null);

  const presets: {label:string; url:string}[] = [
    { label:'Room – Small',   url:'/irs/room_small.wav' },
    { label:'Room – Medium',  url:'/irs/room_medium.wav' },
    { label:'Mic – Boom',     url:'/irs/mic_boom.wav' },
    { label:'Plate – Classic',url:'/irs/plate_classic.wav' },
    { label:'Spring – Vintage',url:'/irs/spring_vintage.wav' },
    { label:'Vintage Hall',   url:'/irs/vintage_hall.wav' },
  ];

  const onDrop = (e: React.DragEvent) => {
    e.preventDefault();
    const file = e.dataTransfer.files?.[0];
    if (!file) return;
    file.arrayBuffer().then(p.loadIRFromArrayBuffer as any).catch(console.error);
  };

  return (
    <ArturiaFrame title="CONVOLUTION REVERB" subtitle="Drag & drop IR WAVs or choose a preset">
      {/* Drop zone / Presets */}
      <div
        onDragOver={(e)=>e.preventDefault()}
        onDrop={onDrop}
        className="rounded-lg border border-black/10 bg-white/60 p-4 mb-5"
        style={{ boxShadow:'inset 0 1px 0 #fff, inset 0 -1px 0 rgba(0,0,0,.06)' }}
      >
        <div className="text-xs mb-3 text-black/70">Drop an IR file here (.wav) or pick:</div>
        <div className="flex flex-wrap gap-2">
          {presets.map(preset => (
            <button
              key={preset.label}
              onClick={()=>p.loadIRFromUrl(preset.url)}
              className="px-3 py-2 rounded-md border border-black/10 bg-[var(--art-plate)] hover:bg-white transition"
            >
              {preset.label}
            </button>
          ))}
          <input ref={fileRef} type="file" accept=".wav" hidden onChange={(e)=>{
            const f = e.target.files?.[0]; if (!f) return;
            f.arrayBuffer().then(p.loadIRFromArrayBuffer as any);
          }}/>
          <button
            onClick={()=>fileRef.current?.click()}
            className="px-3 py-2 rounded-md border border-black/10 bg-[var(--art-plate)] hover:bg-white"
          >
            Browse…
          </button>
        </div>
      </div>

      {/* Controls */}
      <div className="grid sm:grid-cols-2 gap-5">
        <ArturiaSlider label="Wet" onChange={p.setWet} defaultValue={40}/>
        <ArturiaSlider label="Dry" onChange={p.setDry} defaultValue={70}/>
        <ArturiaSlider label="Pre-Delay (ms)" onChange={p.setPreDelay} defaultValue={20} max={250}/>
        <ArturiaSlider label="Damp" onChange={p.setDamp} defaultValue={10}/>
        <ArturiaSlider label="Pre HP (Hz)" onChange={p.setHP} defaultValue={80} max={2000}/>
        <ArturiaSlider label="Pre LP (Hz)" onChange={p.setLP} defaultValue={12000} max={20000}/>
      </div>
    </ArturiaFrame>
  );
};

const ArturiaSlider: React.FC<{label:string; onChange:(v:number)=>void; defaultValue?:number; max?:number}> =
({ label, onChange, defaultValue=50, max=100 }) => {
  return (
    <div className="flex items-center gap-4">
      <div className="w-32 text-[11px] tracking-[.18em] font-semibold opacity-70">{label}</div>
      <div className="flex-1">
        <Slider
          defaultValue={[defaultValue]}
          min={0}
          max={max}
          step={1}
          onValueChange={(v)=>onChange((v?.[0] ?? 0))}
        />
      </div>
    </div>
  );
};
