const KEY='btz:preset-lib';
export function usePresetLibrary(){
  const load=()=>{ try{ return JSON.parse(localStorage.getItem(KEY)||'[]'); }catch{return [];} };
  const save=(items:any[])=>localStorage.setItem(KEY, JSON.stringify(items));
  return { load, save } as const;
}
