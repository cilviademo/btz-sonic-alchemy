import { useEffect, useRef, useState, useCallback } from 'react';

type EngineParams = {
  mix?: number; drive?: number; active?: boolean;
  clippingType?: 'soft'|'hard'|'tube'|'tape'|'digital';
  clippingBlend?: number;
};

export function useAudioEngine(){
  const ctxRef=useRef<AudioContext>();          // expose for IR hook
  const nodeRef=useRef<AudioWorkletNode>();
  const [running,setRunning]=useState(false);
  const [analyserOut,setAnalyserOut]=useState<AnalyserNode>();

  const start=useCallback(async()=>{
    if(running || typeof window==='undefined') return;
    const ctx=new AudioContext({ latencyHint:'interactive' });
    await ctx.audioWorklet.addModule('/worklets/btz-processor.js');
    const node=new AudioWorkletNode(ctx,'btz-processor',{ parameterData:{ mix:1, drive:0, active:1, clipBlend:.5 } });
    const analyser=ctx.createAnalyser(); analyser.fftSize=2048;

    // temporary input: microphone (replace with host bridge in native build)
    const mic=await navigator.mediaDevices.getUserMedia({audio:true}).catch(()=>null);
    if(!mic){ await ctx.close(); return; }
    const src=ctx.createMediaStreamSource(mic);
    src.connect(node);

    ctxRef.current=ctx; nodeRef.current=node; setAnalyserOut(analyser);
    setRunning(true);
  },[running]);

  const stop=useCallback(async()=>{ if(!ctxRef.current) return;
    await ctxRef.current.close(); ctxRef.current=undefined; nodeRef.current=undefined; setRunning(false);
  },[]);

  const update=useCallback((p:EngineParams)=>{
    const node=nodeRef.current, ctx=ctxRef.current; if(!node||!ctx) return;
    const set=(name:string,v:number)=>{ const ap=(node.parameters as any).get(name) as AudioParam;
      ap?.linearRampToValueAtTime(v, ctx.currentTime+0.02); };
    if(p.mix!=null) set('mix', p.mix);
    if(p.drive!=null) set('drive', p.drive);
    if(p.active!=null) set('active', p.active?1:0);
    if(p.clippingBlend!=null) set('clipBlend', p.clippingBlend);
    if(p.clippingType) node.port.postMessage({ type:'clipType', value:p.clippingType });
  },[]);

  useEffect(()=>()=>{ if(ctxRef.current) ctxRef.current.close(); },[]);
  return { running, start, stop, update, analyserOut, ctxRef, nodeRef } as const;
}
