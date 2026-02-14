import { useEffect, useRef } from 'react';

export function useHorizontalDragScroll<T extends HTMLElement>() {
  const ref = useRef<T | null>(null);
  useEffect(() => {
    const el = ref.current;
    if (!el) return;

    let isDown = false, startX = 0, scrollLeft = 0;

    const down = (e: MouseEvent | TouchEvent) => {
      isDown = true;
      const pageX = 'touches' in e ? e.touches[0].pageX : (e as MouseEvent).pageX;
      startX = pageX - el.offsetLeft;
      scrollLeft = el.scrollLeft;
    };
    const leave = () => (isDown = false);
    const up = () => (isDown = false);
    const move = (e: MouseEvent | TouchEvent) => {
      if (!isDown) return;
      e.preventDefault();
      const pageX = 'touches' in e ? e.touches[0].pageX : (e as MouseEvent).pageX;
      const x = pageX - el.offsetLeft;
      el.scrollLeft = scrollLeft - (x - startX);
    };
    const wheel = (e: WheelEvent) => {
      if (Math.abs(e.deltaY) > Math.abs(e.deltaX)) el.scrollLeft += e.deltaY;
    };

    el.addEventListener('mousedown', down);
    el.addEventListener('mouseleave', leave);
    el.addEventListener('mouseup', up);
    el.addEventListener('mousemove', move as any, { passive: false });
    el.addEventListener('touchstart', down, { passive: true });
    el.addEventListener('touchmove', move as any, { passive: false });
    el.addEventListener('touchend', up);
    el.addEventListener('wheel', wheel, { passive: true });

    return () => {
      el.removeEventListener('mousedown', down);
      el.removeEventListener('mouseleave', leave);
      el.removeEventListener('mouseup', up);
      el.removeEventListener('mousemove', move as any);
      el.removeEventListener('touchstart', down);
      el.removeEventListener('touchmove', move as any);
      el.removeEventListener('touchend', up);
      el.removeEventListener('wheel', wheel);
    };
  }, []);
  return ref;
}
