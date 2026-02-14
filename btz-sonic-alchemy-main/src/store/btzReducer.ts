import { BTZPluginState } from '@/components/vst/types';

export type BTZAction =
  | { type: 'set'; key: keyof BTZPluginState; value: any }
  | { type: 'batch'; patch: Partial<BTZPluginState> }
  | { type: 'apply-preset'; state: BTZPluginState; morphMs?: number }
  | { type: 'undo' }
  | { type: 'redo' };

export function makeBTZReducer(initial: BTZPluginState) {
  const undo: BTZPluginState[] = [];
  const redo: BTZPluginState[] = [];

  return function reducer(state: BTZPluginState = initial, action: BTZAction): BTZPluginState {
    const push = (s: BTZPluginState) => {
      undo.push(s);
      redo.length = 0;
    };

    switch (action.type) {
      case 'set': {
        const next = { ...state, [action.key]: action.value } as BTZPluginState;
        push(state);
        return next;
      }
      case 'batch': {
        const next = { ...state, ...action.patch } as BTZPluginState;
        push(state);
        return next;
      }
      case 'apply-preset': {
        if (!action.morphMs) {
          push(state);
          return action.state;
        }
        return state; // morph is scheduled externally
      }
      case 'undo':
        return undo.length ? (redo.push(state), undo.pop() as BTZPluginState) : state;
      case 'redo':
        return redo.length ? (undo.push(state), redo.pop() as BTZPluginState) : state;
      default:
        return state;
    }
  };
}
