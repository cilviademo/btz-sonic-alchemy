import { useEffect, useRef, useState } from 'react';

export function useAnalyser(analyser?: AnalyserNode, fps = 60) {
  const raf = useRef<number>();
  const last = useRef(0);
  const [data, setData] = useState({
    spectrum: new Float32Array(64),
    waveform: new Float32Array(128),
    levelIn: 0,
    levelOut: 0,
  });

  useEffect(() => {
    if (!analyser || typeof window === 'undefined') return;
    const fft = new Float32Array(analyser.frequencyBinCount);
    const time = new Float32Array(256);

    const tick = (t: number) => {
      raf.current = requestAnimationFrame(tick);
      if (t - last.current < 1000 / fps) return;

      analyser.getFloatFrequencyData(fft);
      analyser.getFloatTimeDomainData(time);

      // map [-140..0] dB to 0..1
      const norm = (db: number) => Math.min(1, Math.max(0, (db + 90) / 90));

      // downsample to 64 bins (max pool)
      const spectrum = data.spectrum; // reuse buffer
      const step = Math.max(1, Math.floor(fft.length / spectrum.length));
      for (let i = 0; i < spectrum.length; i++) {
        let acc = -160;
        const base = i * step;
        for (let j = 0; j < step; j++) acc = Math.max(acc, fft[base + j] ?? -160);
        spectrum[i] = norm(acc);
      }

      // normalize waveform -1..1 ➜ 0..1 for drawing
      const waveform = data.waveform; // reuse buffer
      const hop = Math.max(1, Math.floor(time.length / waveform.length));
      for (let i = 0; i < waveform.length; i++) {
        waveform[i] = time[i * hop] * 0.5 + 0.5;
      }

      // simple level estimation
      let rms = 0;
      for (let i = 0; i < time.length; i++) rms += time[i] * time[i];
      const level = Math.sqrt(rms / time.length);

      setData(prev => ({
        spectrum,
        waveform,
        levelIn: prev.levelIn * 0.85 + level * 0.15,
        levelOut: prev.levelOut * 0.85 + level * 0.15,
      }));

      last.current = t;
    };

    raf.current = requestAnimationFrame(tick);
    return () => {
      if (raf.current) cancelAnimationFrame(raf.current);
    };
  }, [analyser, fps]);

  return data;
}
