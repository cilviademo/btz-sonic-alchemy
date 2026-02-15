import React, { useEffect, useRef, useState, useCallback } from 'react';

function makeNoise(seed=1){ let s=seed>>>0; const rand=()=> (s=(s*1664525+1013904223)>>>0)/0xffffffff;
  const grad=new Float32Array(512).map(()=> (rand()*2-1));
  return (x:number,y:number)=>{ const i=(Math.floor(x)&255), j=(Math.floor(y)&255);
    const xf=x-Math.floor(x), yf=y-Math.floor(y); const u=xf*xf*(3-2*xf), v=yf*yf*(3-2*yf);
    const g00=grad[i+j], g10=grad[i+1+j], g01=grad[i+j+1], g11=grad[i+1+j+1];
    const n00=g00*(xf)+g01*(yf), n10=g10*(xf-1)+g11*(yf), n01=g00*(xf)+g01*(yf-1), n11=g10*(xf-1)+g11*(yf-1);
    const nx0=n00*(1-u)+n10*u, nx1=n01*(1-u)+n11*u; return nx0*(1-v)+nx1*v; };
}

type Props = {
  value:number; onChange:(v:number)=>void; label:string;
  min?:number; max?:number; step?:number;
  spectrumData?:Float32Array; waveformData?:Float32Array;
  colorA?:string; colorB?:string; disabled?:boolean;
};
export const ThermalKnob: React.FC<Props> = ({
  value,onChange,label,min=0,max=1,step=.001,spectrumData,waveformData,
  colorA='#ff2fb9', colorB='#39ff88', disabled=false
})=>{
  const ref=useRef<HTMLCanvasElement>(null);
  const [drag,setDrag]=useState<{y:number;start:number}|null>(null);
  const [hover,setHover]=useState(false);
  const noise=useRef(makeNoise(1337)); const valRef=useRef(value);
  useEffect(()=>{ valRef.current=value; },[value]);
  const clamp=(v:number)=>Math.max(min,Math.min(max,v));
  const quant=(v:number)=>Math.round(v/step)*step;

  const startDrag=(e:React.MouseEvent|React.TouchEvent)=>{ if(disabled) return;
    const y='touches'in e?e.touches[0].clientY:(e as React.MouseEvent).clientY; setDrag({y,start:value}); };
  const moveDrag=useCallback((clientY:number,mult=0.25)=>{ if(!drag)return;
    const dy=drag.y-clientY; const delta=(max-min)*dy*0.002*mult; onChange(quant(clamp(drag.start+delta))); },[drag,min,max,onChange]);
  useEffect(()=>{ const up=()=>setDrag(null);
    const move=(e:MouseEvent)=>moveDrag(e.clientY, e.shiftKey?0.05:(e.altKey?0.6:0.25));
    if(drag){ window.addEventListener('mousemove',move); window.addEventListener('mouseup',up,{once:true}); }
    return ()=>window.removeEventListener('mousemove',move); },[drag,moveDrag]);
  const onWheel=(e:React.WheelEvent)=>{ if(disabled) return; const dir=e.deltaY>0?-1:1; const mult=e.shiftKey?.1:1; onChange(quant(clamp(value+dir*step*10*mult))); };
  const onKey=(e:React.KeyboardEvent)=>{ if(disabled) return;
    if(e.key==='ArrowUp'||e.key==='ArrowRight') onChange(quant(clamp(value+step)));
    if(e.key==='ArrowDown'||e.key==='ArrowLeft') onChange(quant(clamp(value-step))); };
  const onDouble=()=>!disabled && onChange(0.5*(min+max));

  useEffect(()=>{ const c=ref.current; if(!c)return; const ctx=c.getContext('2d',{alpha:true})!;
    let raf=0,t=0; const draw=()=>{ raf=requestAnimationFrame(draw); t+=0.016;
      const size=Math.min(c.clientWidth,c.clientHeight), W=(c.width=size*devicePixelRatio), H=(c.height=size*devicePixelRatio);
      const cx=W/2, cy=H/2; ctx.clearRect(0,0,W,H);
      const bg=ctx.createRadialGradient(cx,cy,0,cx,cy,W*.55); bg.addColorStop(0,hover?'rgba(255,47,185,.10)':'rgba(255,47,185,.05)'); bg.addColorStop(1,'rgba(20,22,30,.0)');
      ctx.fillStyle=bg; ctx.fillRect(0,0,W,H);
      const rBase=W*.28, val01=(valRef.current-min)/(max-min);

      // pulse ring
      const N=96;
      for(let i=0;i<N;i++){ const ang=(i/N)*Math.PI*2;
        const spec=spectrumData ? spectrumData[Math.floor(i/N*spectrumData.length)] : 0.35;
        const n=noise.current(Math.cos(ang)*2+t*.25,Math.sin(ang)*2+t*.25)*.5+.5;
        const amp=(0.2+spec*.9)*(0.8+0.2*Math.sin(t*2+i*.12));
        const rr=rBase*(1+amp*.35), x=cx+Math.cos(ang)*rr, y=cy+Math.sin(ang)*rr;
        ctx.beginPath(); ctx.arc(x,y,Math.max(1.5,W*.008)*(0.6+n*.8),0,Math.PI*2);
        ctx.fillStyle=`hsla(${(300+i*2)%360},100%,${60+amp*20}%,.8)`; ctx.shadowColor=colorB; ctx.shadowBlur=8; ctx.fill();
      }

      // inner blob
      ctx.save(); ctx.translate(cx,cy); ctx.rotate(-Math.PI/2);
      const R=rBase*(0.6+val01*.25); ctx.beginPath(); const M=128;
      for(let i=0;i<=M;i++){ const th=(i/M)*Math.PI*2;
        const wf=waveformData? (waveformData[Math.floor(i/M*waveformData.length)]-.5)*2 : 0;
        const n=noise.current(Math.cos(th)*1.2+t*.35,Math.sin(th)*1.2+t*.35);
        const r=R*(1+n*.08+wf*.12), x=Math.cos(th)*r, y=Math.sin(th)*r; i?ctx.lineTo(x,y):ctx.moveTo(x,y); }
      ctx.closePath(); const grad=ctx.createLinearGradient(-R,0,R,0); grad.addColorStop(0,colorA); grad.addColorStop(1,colorB);
      ctx.lineWidth=Math.max(3,W*.022); ctx.strokeStyle=grad; ctx.shadowColor=colorA; ctx.shadowBlur=18; ctx.stroke(); ctx.restore();

      // value arc
      ctx.beginPath(); ctx.lineWidth=Math.max(4,W*.02); ctx.strokeStyle='rgba(255,255,255,.12)'; ctx.arc(cx,cy,rBase*.95,0,Math.PI*2); ctx.stroke();
      ctx.beginPath(); ctx.strokeStyle=colorA; ctx.shadowColor=colorA; ctx.shadowBlur=12; ctx.arc(cx,cy,rBase*.95,-Math.PI/2,-Math.PI/2+val01*Math.PI*1.5); ctx.stroke();

      ctx.beginPath(); ctx.arc(cx,cy,Math.max(4,W*.016),0,Math.PI*2); ctx.fillStyle=colorB; ctx.shadowBlur=10; ctx.shadowColor=colorB; ctx.fill();
    }; draw(); return ()=>cancelAnimationFrame(raf);
  },[min,max,colorA,colorB,spectrumData,waveformData]);

  return (
    <div className="btz-knob select-none"
      onMouseEnter={()=>setHover(true)} onMouseLeave={()=>setHover(false)}
      onWheel={onWheel} onKeyDown={onKey}
      tabIndex={0} role="slider" aria-valuemin={min} aria-valuemax={max} aria-valuenow={value} aria-label={label}>
      <div className="relative rounded-full" style={{width:128,height:128,cursor:disabled?'not-allowed':'ns-resize'}}
        onMouseDown={startDrag} onTouchStart={startDrag} onTouchMove={(e)=>moveDrag(e.touches[0].clientY)} onDoubleClick={onDouble}>
        <canvas ref={ref} className="absolute inset-0 w-full h-full" />
      </div>
      <span className="btz-value-badge">{Math.round(value*100)}%</span>
      <span className="btz-knob-label">{label}</span>
    </div>
  );
};
