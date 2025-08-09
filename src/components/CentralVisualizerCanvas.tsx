import React, { useEffect, useRef } from 'react';
export function CentralVisualizerCanvas({ spectrumData, waveformData, level, isProcessing }:{
  spectrumData: Float32Array; waveformData: Float32Array; level:number; isProcessing:boolean;
}){
  const ref=useRef<HTMLCanvasElement>(null);
  useEffect(()=>{
    const cvs=ref.current; if(!cvs) return;
    const ctx=cvs.getContext('2d',{alpha:true})!;
    let raf=0; let t=0;

    // lightweight value-noise (perlin-ish)
    const makeNoise=(seed=1)=>{ let s=seed>>>0; const rand=()=> (s=(s*1664525+1013904223)>>>0)/0xffffffff;
      const grad=new Float32Array(512); for(let i=0;i<512;i++) grad[i]=rand()*2-1;
      return (x:number,y:number)=>{ const i=(Math.floor(x)&255), j=(Math.floor(y)&255);
        const xf=x-Math.floor(x), yf=y-Math.floor(y); const u=xf*xf*(3-2*xf), v=yf*yf*(3-2*yf);
        const g00=grad[i+j], g10=grad[i+1+j], g01=grad[i+j+1], g11=grad[i+1+j+1];
        const n00=g00*(xf)+g01*(yf), n10=g10*(xf-1)+g11*(yf), n01=g00*(xf)+g01*(yf-1), n11=g10*(xf-1)+g11*(yf-1);
        const nx0=n00*(1-u)+n10*u, nx1=n01*(1-u)+n11*v; return nx0*(1-v)+nx1*v; };
    };
    const noise = makeNoise(1337);

    const draw=()=>{
      raf=requestAnimationFrame(draw); t+=0.016;
      const w=(cvs.width=cvs.clientWidth*devicePixelRatio), h=(cvs.height=cvs.clientHeight*devicePixelRatio);
      ctx.clearRect(0,0,w,h);

      const cx=w/2, cy=h/2, r=Math.min(w,h)*.28;

      // background depth
      const bg=ctx.createRadialGradient(cx,cy,0,cx,cy,Math.max(w,h)*.6);
      bg.addColorStop(0,'rgba(255,47,185,.05)');
      bg.addColorStop(1,'rgba(39,75,255,.03)');
      ctx.fillStyle=bg; ctx.fillRect(0,0,w,h);

      // THERMAL PORTAL LAYERS
      ctx.save(); ctx.translate(cx,cy); ctx.rotate(-Math.PI/2);

      // inner plasma ring (noise + spectrum)
      ctx.beginPath(); const M=160;
      for(let i=0;i<=M;i++){
        const th=i/M*Math.PI*2;
        const spec = spectrumData[Math.floor((i/M)*spectrumData.length)] ?? 0;
        const n = noise(Math.cos(th)*1.1 + t*.3, Math.sin(th)*1.1 + t*.3);
        const rr = r*(0.75 + n*0.08 + spec*0.18);
        const x=Math.cos(th)*rr, y=Math.sin(th)*rr;
        i?ctx.lineTo(x,y):ctx.moveTo(x,y);
      }
      ctx.closePath();
      const grad=ctx.createLinearGradient(-r,0,r,0);
      grad.addColorStop(0,'rgba(57,255,136,.9)');
      grad.addColorStop(1,'rgba(255,47,185,.9)');
      ctx.lineWidth=Math.max(3,r*.035);
      ctx.strokeStyle=grad;
      ctx.shadowColor='rgba(255,47,185,.5)'; ctx.shadowBlur=18; ctx.globalAlpha=0.9; ctx.stroke();

      // particle shimmer ring
      ctx.globalAlpha=1;
      const N=110;
      for(let i=0;i<N;i++){
        const th=i/N*Math.PI*2;
        const spec=spectrumData[Math.floor((i/N)*spectrumData.length)] ?? 0.2;
        const pulse = 0.8 + 0.2*Math.sin(t*3 + i*0.15);
        const rr = r*(1.0 + spec*0.35*pulse);
        const x=Math.cos(th)*rr, y=Math.sin(th)*rr;
        const sz = Math.max(1.5, r*.015)*(0.6 + spec*0.8 + (level||0)*0.3);
        ctx.beginPath(); ctx.filter='blur(0.5px)';
        ctx.fillStyle=`rgba(39,75,255,${0.35+spec*0.6})`;
        ctx.arc(x,y,sz,0,Math.PI*2); ctx.fill(); ctx.filter='none';
      }

      // outer transient arc
      ctx.beginPath(); ctx.lineWidth=Math.max(2, r*.02); ctx.strokeStyle='rgba(255,255,255,.14)';
      ctx.arc(0,0,r*1.05,0,Math.PI*2); ctx.stroke();
      ctx.beginPath();
      const span = Math.PI*1.6* Math.min(1, (level||0)*1.2 + 0.1);
      ctx.strokeStyle='rgba(255,47,185,.8)'; ctx.shadowColor='rgba(255,47,185,.8)'; ctx.shadowBlur=12;
      ctx.arc(0,0,r*1.05,-Math.PI/2,-Math.PI/2+span); ctx.stroke();

      ctx.restore();

      // LED segmented spectrum at bottom
      const bars=spectrumData.length; const segH=6*devicePixelRatio, gap=3*devicePixelRatio; const maxH=h*.32;
      const bw=Math.max(2, w/(bars*1.8));
      for(let i=0;i<bars;i++){
        const v=spectrumData[i];
        const total=Math.floor(maxH/(segH+gap));
        const lit=Math.floor(v*total);
        const x=i*(bw*1.8)+bw*.3;
        for(let s=0;s<total;s++){
          const y=h - (s+1)*(segH+gap);
          const hot = s < lit;
          const hue = 120 - (s/total)*120; // green->red
          ctx.fillStyle = hot ? `hsla(${hue},100%,55%,${0.35 + s/total*0.5})` : 'rgba(255,255,255,0.06)';
          ctx.fillRect(x,y,bw,segH);
        }
      }

      // level ring for depth
      ctx.beginPath(); ctx.arc(cx,cy,r*(0.65+(level||0)*.2),0,Math.PI*2);
      ctx.strokeStyle='rgba(255,47,185,.35)'; ctx.lineWidth=2*devicePixelRatio; ctx.stroke();
    };
    draw();
    return ()=>cancelAnimationFrame(raf);
  },[spectrumData,waveformData,level,isProcessing]);
  return <canvas ref={ref} className="w-full max-w-3xl h-56 sm:h-72 rounded-2xl" />;
}
