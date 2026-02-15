import { useEffect } from 'react';

type Hotkeys = {
  bypass?:()=>void; toggleMeters?:()=>void; nextPreset?:()=>void; prevPreset?:()=>void; reset?:()=>void; undo?:()=>void; toggleGrid?:()=>void;
};

export const useGlobalHotkeys = (on: Hotkeys)=>{
  useEffect(()=>{
    const h=(e:KeyboardEvent)=>{
      // Skip hotkeys when user is typing in input fields
      const target = e.target as HTMLElement;
      const isInput = target.tagName === 'INPUT' || target.tagName === 'TEXTAREA' || target.isContentEditable;
      if (isInput) return;

      if (e.key===' ') { on.bypass?.(); e.preventDefault(); }
      if (e.key.toLowerCase()==='m') { on.toggleMeters?.(); e.preventDefault(); }
      if (e.key==='ArrowRight') on.nextPreset?.();
      if (e.key==='ArrowLeft') on.prevPreset?.();
      if (e.key.toLowerCase()==='r') { on.reset?.(); e.preventDefault(); }
      if (e.key.toLowerCase()==='g') { on.toggleGrid?.(); e.preventDefault(); }
      if ((e.metaKey||e.ctrlKey) && e.key.toLowerCase()==='z') on.undo?.();
    };
    window.addEventListener('keydown',h); return()=>window.removeEventListener('keydown',h);
  },[on]);
};
