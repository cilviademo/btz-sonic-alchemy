export const BTZ_PARAMS = ['punch','warmth','boom','mix','drive','texture','clippingBlend'] as const;
export type ParamId = typeof BTZ_PARAMS[number];
export function asClipType(v:any):'soft'|'hard'|'tube'|'tape'|'digital'{
  return (['soft','hard','tube','tape','digital'] as const).includes(v) ? v : 'soft';
}
