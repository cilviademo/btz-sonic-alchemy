import React, { useEffect, useRef, useState } from 'react';

export const LEDMeter: React.FC<{ value: number; segments?: number }>=({ value, segments = 32 })=>{
  const [peak, setPeak] = useState(0);
  const val = Math.max(0, Math.min(1, value || 0));
  const peakRef = useRef(0);

  useEffect(()=>{
    // peak hold + fall
    if (val > peakRef.current) {
      peakRef.current = val;
      setPeak(val);
    }
    const id = requestAnimationFrame(()=>{
      peakRef.current = Math.max(0, peakRef.current - 0.005);
      setPeak(peakRef.current);
    });
    return ()=>cancelAnimationFrame(id);
  },[val]);

  const lit = Math.round(val * segments);
  const peakSeg = Math.max(0, Math.min(segments-1, Math.round(peak * segments)));

  return (
    <div className="relative h-full w-6 sm:w-7 md:w-8 flex flex-col-reverse justify-start items-stretch gap-[3px]">
      {Array.from({ length: segments }).map((_, i) => {
        const hot = i < lit;
        const hue = 120 - (i/segments)*120; // green -> red
        const isPeak = i === peakSeg;
        return (
          <div key={i}
               className="rounded-[2px]"
               style={{
                 height: 6,
                 background: hot ? `hsl(${hue} 100% 55% / ${0.5 + i/segments*0.4})` : 'rgba(255,255,255,0.06)',
                 boxShadow: hot ? `0 0 ${isPeak ? 10 : 6}px hsl(${hue} 100% 60% / ${isPeak ? 0.9 : 0.45})` : 'none',
               }}
          />
        );
      })}
      {/* Peak marker line */}
      <div className="absolute left-0 right-0" style={{
        bottom: peakSeg * (6+3), height: 2, background: 'rgba(255,255,255,0.25)'
      }}/>
    </div>
  );
};
