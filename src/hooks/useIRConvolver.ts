import { useCallback, useEffect, useRef, useState } from 'react';

export function useIRConvolver(ctxRef: React.MutableRefObject<AudioContext|undefined>,
                               nodeRef: React.MutableRefObject<AudioNode|undefined>) {
  const [ready,setReady]=useState(false);
  const convolverRef=useRef<ConvolverNode>(); const wetRef=useRef<GainNode>(); const dryRef=useRef<GainNode>();
  const preDelayRef=useRef<DelayNode>(); const preHPRef=useRef<BiquadFilterNode>(); const preLPRef=useRef<BiquadFilterNode>();
  const postDampRef=useRef<BiquadFilterNode>();

  const connect=useCallback(()=>{
    const ctx=ctxRef.current, src=nodeRef.current; if(!ctx||!src) return;
    const wet=wetRef.current=wetRef.current||ctx.createGain(); wet.gain.value=.18;
    const dry=dryRef.current=dryRef.current||ctx.createGain(); dry.gain.value=.82;
    const cv=convolverRef.current=convolverRef.current||new ConvolverNode(ctx,{normalize:true});
    const preDel=preDelayRef.current=preDelayRef.current||ctx.createDelay(2); preDel.delayTime.value=0;
    const hp=preHPRef.current=preHPRef.current||ctx.createBiquadFilter(); hp.type='highpass'; hp.frequency.value=100;
    const lp=preLPRef.current=preLPRef.current||ctx.createBiquadFilter(); lp.type='lowpass'; lp.frequency.value=16000;
    const damp=postDampRef.current=postDampRef.current||ctx.createBiquadFilter(); damp.type='lowpass'; damp.frequency.value=14000;

    try{ (src as any).disconnect(); }catch{}
    (src as any).connect(dry);
    (src as any).connect(hp).connect(lp).connect(preDel).connect(cv).connect(damp).connect(wet);
    setReady(true);
  },[ctxRef,nodeRef]);

  useEffect(()=>{ connect(); },[connect]);

  const setWet=(v:number)=>{ wetRef.current && (wetRef.current.gain.value=v); };
  const setDry=(v:number)=>{ dryRef.current && (dryRef.current.gain.value=v); };
  const setPreDelay=(ms:number)=>{ if(preDelayRef.current) preDelayRef.current.delayTime.value=Math.max(0,Math.min(2,ms/1000)); };
  const setHP=(hz:number)=>{ if(preHPRef.current) preHPRef.current.frequency.value=Math.max(10,Math.min(1000,hz)); };
  const setLP=(hz:number)=>{ if(preLPRef.current) preLPRef.current.frequency.value=Math.max(1000,Math.min(20000,hz)); };
  const setDamp=(hz:number)=>{ if(postDampRef.current) postDampRef.current.frequency.value=Math.max(2000,Math.min(20000,hz)); };

  const loadIRFromArrayBuffer=useCallback(async (buf:ArrayBuffer)=>{
    const ctx=ctxRef.current; if(!ctx) return;
    const audioBuf=await ctx.decodeAudioData(buf.slice(0)); if(convolverRef.current) convolverRef.current.buffer=audioBuf;
  },[ctxRef]);

  const loadIRFromUrl=useCallback(async (url:string)=>{
    const res=await fetch(url); const buf=await res.arrayBuffer(); await loadIRFromArrayBuffer(buf);
  },[loadIRFromArrayBuffer]);

  return { ready, convolver:convolverRef, dry:dryRef, wet:wetRef,
           setWet,setDry,setPreDelay,setHP,setLP,setDamp, loadIRFromUrl, loadIRFromArrayBuffer } as const;
}
