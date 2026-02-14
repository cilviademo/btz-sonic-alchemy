import { useRef, useCallback } from 'react';

export function useRafThrottle(fps = 60) {
  const ticking = useRef(false);
  const last = useRef(0);
  const frame = 1000 / fps;

  return useCallback((fn: () => void) => {
    const now = performance.now();
    if (ticking.current && now - last.current < frame) return;
    ticking.current = true;
    requestAnimationFrame(() => {
      last.current = performance.now();
      ticking.current = false;
      fn();
    });
  }, [fps]);
}
