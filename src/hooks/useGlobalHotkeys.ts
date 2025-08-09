import { useEffect } from 'react';

export const useGlobalHotkeys = (on:{
  bypass?:()=>void; toggleMeters?:()=>void; nextPreset?:()=>void; prevPreset?:()=>void; reset?:()=>void; undo?:()=>void
})=>{
  useEffect(()=>{
    const h=(e:KeyboardEvent)=>{
      if (e.key===' ') { on.bypass?.(); e.preventDefault(); }
      if (e.key.toLowerCase()==='m') on.toggleMeters?.();
      if (e.key==='ArrowRight') on.nextPreset?.();
      if (e.key==='ArrowLeft') on.prevPreset?.();
      if (e.key.toLowerCase()==='r') on.reset?.();
      if ((e.metaKey||e.ctrlKey) && e.key.toLowerCase()==='z') on.undo?.();
    };
    window.addEventListener('keydown',h); return()=>window.removeEventListener('keydown',h);
  },[on]);
};
