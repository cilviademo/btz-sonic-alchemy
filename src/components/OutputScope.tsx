import React, { useEffect, useRef } from 'react';
export function OutputScope({ data, lufs, peak }:{ data:Float32Array; lufs:number; peak:number; }){
  const ref=useRef<HTMLCanvasElement>(null);
  useEffect(()=>{
    const c=ref.current!, x=c.getContext('2d')!; const draw=()=>{
      const w=(c.width=c.clientWidth*devicePixelRatio), h=(c.height=c.clientHeight*devicePixelRatio);
      x.fillStyle='hsl(220 12% 9%)'; x.fillRect(0,0,w,h);
      x.strokeStyle='#fff'; x.globalAlpha=.08; for(let i=1;i<4;i++){ const y=(h*i/4)|0; x.beginPath(); x.moveTo(0,y); x.lineTo(w,y); x.stroke(); }
      x.globalAlpha=1; x.beginPath();
      for(let i=0;i<data.length;i++){ const xx=(i/(data.length-1))*w; const yy=h*(.5-(data[i]-.5)*.7); i?x.lineTo(xx,yy):x.moveTo(xx,yy); }
      x.lineWidth=3; x.shadowColor='rgba(255,47,185,.7)'; x.shadowBlur=12;
      const grad=x.createLinearGradient(0,0,w,0); grad.addColorStop(0,'#00d4ff'); grad.addColorStop(.3,'#8a2be2'); grad.addColorStop(.6,'#ff2fb9'); grad.addColorStop(1,'#ff8c00');
      x.strokeStyle=grad; x.stroke(); requestAnimationFrame(draw);
    }; draw();
  },[data]);
  return (
    <div className="rounded-2xl border border-white/8 p-4" style={{background:'var(--btz-panel)'}}>
      <div className="flex items-center justify-between mb-2">
        <span className="text-sm font-bold text-white/90">OUTPUT</span>
        <div className="flex gap-3">
          <span className="px-2.5 py-1 rounded-md text-[11px] font-mono" style={{background:'rgba(255,255,255,.06)',border:'1px solid rgba(255,255,255,.08)',color:'var(--btz-pink)'}}>LUFS {lufs.toFixed(1)}</span>
          <span className="px-2.5 py-1 rounded-md text-[11px] font-mono" style={{background:'rgba(255,255,255,.06)',border:'1px solid rgba(255,255,255,.08)',color:'var(--btz-green)'}}>PEAK {(peak>0?'+':'')+peak.toFixed(1)}dB</span>
        </div>
      </div>
      <canvas ref={ref} className="w-full h-28 rounded-md" />
    </div>
  );
}
