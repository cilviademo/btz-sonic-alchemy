import { BTZPluginState } from '@/components/vst/types';

export function morphParams(
  from: BTZPluginState,
  to: BTZPluginState,
  ms: number,
  onStep: (patch: Partial<BTZPluginState>) => void,
  done: () => void
) {
  const numericKeys = Object.keys(to).filter(
    (k) => typeof (to as any)[k] === 'number'
  ) as (keyof BTZPluginState)[];
  const start = performance.now();
  const tick = (t: number) => {
    const k = Math.min(1, (t - start) / ms);
    const patch: Partial<BTZPluginState> = {} as Partial<BTZPluginState>;
    for (const key of numericKeys) {
      (patch as any)[key] = (from as any)[key] + ((to as any)[key] - (from as any)[key]) * k;
    }
    onStep(patch);
    if (k < 1) requestAnimationFrame(tick);
    else done();
  };
  requestAnimationFrame(tick);
}
