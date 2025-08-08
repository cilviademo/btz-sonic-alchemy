import { useEffect, useMemo, useRef, useState } from 'react';

interface EngineNodes {
  ctx: AudioContext;
  source?: OscillatorNode | MediaStreamAudioSourceNode;
  inputAnalyser: AnalyserNode;
  outputAnalyser: AnalyserNode;
  dryGain: GainNode;
  wetGain: GainNode;
  shaper: WaveShaperNode;
  outMix: GainNode;
}

export function useAudioEngine() {
  const nodesRef = useRef<EngineNodes | null>(null);
  const [running, setRunning] = useState(false);
  const isClient = typeof window !== 'undefined';

  const ensureContext = async () => {
    if (!isClient) return null;
    if (nodesRef.current) return nodesRef.current;

    const ctx = new (window.AudioContext || (window as any).webkitAudioContext)();

    // Core nodes
    const inputAnalyser = ctx.createAnalyser();
    inputAnalyser.fftSize = 2048;

    const outputAnalyser = ctx.createAnalyser();
    outputAnalyser.fftSize = 2048;

    const dryGain = ctx.createGain();
    const wetGain = ctx.createGain();
    const shaper = ctx.createWaveShaper();
    const outMix = ctx.createGain();

    // Default gains
    dryGain.gain.value = 1;
    wetGain.gain.value = 0;
    outMix.gain.value = 0.9;

    // Test tone source (220 Hz, quiet)
    const osc = ctx.createOscillator();
    osc.type = 'sawtooth';
    osc.frequency.value = 110;
    const oscGain = ctx.createGain();
    oscGain.gain.value = 0.05; // quiet

    // Graph: source -> inputAnalyser -> [dry, shaper] -> [dryGain, wetGain] -> outMix -> destination
    osc.connect(oscGain);
    oscGain.connect(inputAnalyser);

    inputAnalyser.connect(dryGain);
    inputAnalyser.connect(shaper);

    shaper.connect(wetGain);

    dryGain.connect(outMix);
    wetGain.connect(outMix);

    outMix.connect(ctx.destination);
    outMix.connect(outputAnalyser);

    nodesRef.current = {
      ctx,
      source: osc,
      inputAnalyser,
      outputAnalyser,
      dryGain,
      wetGain,
      shaper,
      outMix,
    };

    return nodesRef.current;
  };

  const start = async () => {
    const nodes = await ensureContext();
    if (!nodes) return;
    if (nodes.ctx.state === 'suspended') await nodes.ctx.resume();
    if (nodes.source instanceof OscillatorNode) {
      try { nodes.source.start(); } catch {}
    }
    setRunning(true);
  };

  const stop = async () => {
    const nodes = nodesRef.current;
    if (!nodes) return;
    try {
      if (nodes.source instanceof OscillatorNode) {
        nodes.source.stop();
      }
      await nodes.ctx.close();
    } catch {}
    nodesRef.current = null;
    setRunning(false);
  };

  // Update audio params from UI state
  const update = (state: {
    mix?: number;
    drive?: number;
    active?: boolean;
    clippingType?: 'soft' | 'hard' | 'tube' | 'tape' | 'digital';
    clippingBlend?: number;
  }) => {
    const n = nodesRef.current;
    if (!n) return;

    const mix = state.mix ?? 1;
    const drive = state.drive ?? 0;
    const active = state.active ?? true;

    n.dryGain.gain.setTargetAtTime(1 - mix, n.ctx.currentTime, 0.02);
    n.wetGain.gain.setTargetAtTime(mix, n.ctx.currentTime, 0.02);
    n.outMix.gain.setTargetAtTime(active ? 0.9 : 0.0, n.ctx.currentTime, 0.03);

    // Build a waveshaper curve for musical clipping
    const samples = 2048;
    const curve = new Float32Array(samples);
    const amount = 1 + drive * 24; // drive 0..1 -> amount 1..25

    const type = state.clippingType || 'soft';
    const blend = state.clippingBlend ?? 0.5;

    for (let i = 0; i < samples; i++) {
      const x = (i / (samples - 1)) * 2 - 1; // -1..1
      let y: number;
      switch (type) {
        case 'hard':
          y = Math.max(-0.9, Math.min(0.9, x * amount * 0.6));
          break;
        case 'tube':
          y = Math.tanh(x * amount) * 0.9 + 0.1 * x;
          break;
        case 'tape':
          y = (x * amount) / (1 + Math.abs(x * amount));
          break;
        case 'digital':
          y = x * (1 - 0.1) + 0.1 * Math.sign(x) * Math.pow(Math.abs(x), 0.8);
          break;
        case 'soft':
        default:
          y = Math.tanh(x * amount);
          break;
      }
      // Blend with soft tanh for sweeter knee
      const soft = Math.tanh(x * (1 + drive * 10));
      curve[i] = y * (1 - blend) + soft * blend;
    }
    n.shaper.curve = curve;
    n.shaper.oversample = '4x';
  };

  // Expose stable references
  const analysers = useMemo(() => {
    return {
      get input() {
        return nodesRef.current?.inputAnalyser;
      },
      get output() {
        return nodesRef.current?.outputAnalyser;
      },
    };
  }, []);

  useEffect(() => {
    return () => {
      // Cleanup on unmount
      if (nodesRef.current) {
        try { nodesRef.current.ctx.close(); } catch {}
        nodesRef.current = null;
      }
    };
  }, []);

  return {
    running,
    start,
    stop,
    update,
    analyserIn: analysers.input,
    analyserOut: analysers.output,
  } as const;
}
