import { useEffect } from 'react';
export function useHotkeys(map: Record<string,(e:KeyboardEvent)=>void>) {
  useEffect(()=>{
    const h=(e:KeyboardEvent)=>{
      const key=[e.metaKey?'Cmd':'',e.ctrlKey?'Ctrl':'',e.shiftKey?'Shift':'',e.altKey?'Alt':'',
                 e.key.length===1?e.key.toUpperCase():e.key].filter(Boolean).join('+');
      if(map[key]){ e.preventDefault(); map[key](e); }
    };
    window.addEventListener('keydown',h);
    return ()=>window.removeEventListener('keydown',h);
  },[map]);
}
