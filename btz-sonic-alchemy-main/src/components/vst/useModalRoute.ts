import { useCallback, useState } from 'react';

export type ModalRoute =
  | null
  | 'spark'
  | 'shine'
  | 'master'
  | 'deep'
  | 'ir'
  | 'ai'
  | 'meters'
  | 'chain';

export function useModalRoute() {
  const [route, setRoute] = useState<ModalRoute>(null);
  const open = useCallback((r: ModalRoute) => setRoute(r), []);
  const close = useCallback(() => setRoute(null), []);
  return { route, open, close, setRoute };
}
