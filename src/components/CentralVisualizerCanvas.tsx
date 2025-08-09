import React, { useEffect, useRef } from 'react';
export function CentralVisualizerCanvas({ spectrumData, waveformData, level, isProcessing }:{
  spectrumData: Float32Array; waveformData: Float32Array; level:number; isProcessing:boolean;
}){
  const ref=useRef<HTMLCanvasElement>(null);
  useEffect(()=>{
    const cvs=ref.current; if(!cvs) return;
    const ctx=cvs.getContext('2d',{alpha:true})!; let raf=0; const draw=()=>{
      raf=requestAnimationFrame(draw);
      const w=(cvs.width=cvs.clientWidth*devicePixelRatio), h=(cvs.height=cvs.clientHeight*devicePixelRatio);
      ctx.clearRect(0,0,w,h);

      // background
      const g=ctx.createRadialGradient(w/2,h/2,0,w/2,h/2,Math.max(w,h)*.6);
      g.addColorStop(0,'rgba(255,47,185,.06)'); g.addColorStop(1,'rgba(39,75,255,.02)'); ctx.fillStyle=g; ctx.fillRect(0,0,w,h);

      // ring waveform
      const cx=w/2, cy=h/2, r=Math.min(w,h)*.28; ctx.save(); ctx.translate(cx,cy); ctx.rotate(-Math.PI/2);
      ctx.beginPath(); ctx.lineWidth=Math.max(2,r*.03);
      for(let i=0;i<waveformData.length;i++){
        const th=(i/waveformData.length)*Math.PI*2, amp=(waveformData[i]-.5)*2, rr=r+amp*r*.3;
        const x=Math.cos(th)*rr, y=Math.sin(th)*rr; i?ctx.lineTo(x,y):ctx.moveTo(x,y);
      }
      ctx.closePath(); ctx.shadowColor='rgba(255,47,185,.8)'; ctx.shadowBlur=18; ctx.strokeStyle='rgba(57,255,136,.95)'; ctx.stroke(); ctx.restore();

      // spectrum bars
      const bars=spectrumData.length, bw=w/(bars*1.6);
      for(let i=0;i<bars;i++){ const v=spectrumData[i], x=i*(bw*1.6)+bw*.3, barH=v*(h*.35), y=h-barH-8*devicePixelRatio;
        ctx.fillStyle=`rgba(39,75,255,${0.25+v*0.6})`; ctx.fillRect(x,y,bw,barH); }

      // level ring
      ctx.beginPath(); ctx.arc(cx,cy,r*(0.65+level*.2),0,Math.PI*2);
      ctx.strokeStyle='rgba(255,47,185,.35)'; ctx.lineWidth=2*devicePixelRatio; ctx.stroke();
    }; draw(); return ()=>cancelAnimationFrame(raf);
  },[spectrumData,waveformData,level,isProcessing]);
  return <canvas ref={ref} className="w-full max-w-3xl h-56 sm:h-72 rounded-2xl" />;
}
