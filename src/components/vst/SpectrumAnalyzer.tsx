import React, { useRef, useEffect, useState } from 'react';
import { cn } from '@/lib/utils';

interface SpectrumAnalyzerProps {
  fftData: Float32Array;
  width?: number;
  height?: number;
  showFreqLabels?: boolean;
  showGrid?: boolean;
  colorMode?: 'blue' | 'rainbow' | 'warm';
}

export const SpectrumAnalyzer: React.FC<SpectrumAnalyzerProps> = ({
  fftData = new Float32Array(1024),
  width = 400,
  height = 200,
  showFreqLabels = true,
  showGrid = true,
  colorMode = 'blue'
}) => {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const [analyzerData, setAnalyzerData] = useState<number[]>(new Array(256).fill(0));

  // Simulate spectrum data if none provided
  useEffect(() => {
    if (fftData.length <= 1) {
      const interval = setInterval(() => {
        const newData = new Array(256).fill(0).map((_, i) => {
          const freq = (i / 256) * 24000; // 0-24kHz range
          let magnitude = Math.random() * 0.1;
          
          // Add some realistic frequency content
          if (freq < 100) magnitude += Math.random() * 0.3; // Low freq energy
          if (freq > 60 && freq < 300) magnitude += Math.random() * 0.4; // Sub/bass
          if (freq > 1000 && freq < 4000) magnitude += Math.random() * 0.3; // Mids
          if (freq > 8000) magnitude += Math.random() * 0.2; // Highs
          
          return Math.max(0, Math.min(1, magnitude));
        });
        setAnalyzerData(newData);
      }, 50);
      
      return () => clearInterval(interval);
    }
  }, [fftData]);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    // Clear canvas
    ctx.fillStyle = 'hsl(var(--plugin-surface))';
    ctx.fillRect(0, 0, width, height);

    // Draw grid
    if (showGrid) {
      ctx.strokeStyle = 'hsl(var(--plugin-raised))';
      ctx.lineWidth = 1;
      
      // Horizontal lines (dB)
      for (let i = 0; i <= 4; i++) {
        const y = (i / 4) * height;
        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(width, y);
        ctx.stroke();
      }
      
      // Vertical lines (frequency)
      const freqLines = [100, 1000, 10000]; // Hz
      freqLines.forEach(freq => {
        const x = (Math.log10(freq) - 1) / 4 * width; // Log scale
        if (x >= 0 && x <= width) {
          ctx.beginPath();
          ctx.moveTo(x, 0);
          ctx.lineTo(x, height);
          ctx.stroke();
        }
      });
    }

    // Draw frequency labels
    if (showFreqLabels) {
      ctx.fillStyle = 'hsl(var(--foreground) / 0.5)';
      ctx.font = '10px var(--font-mono)';
      ctx.textAlign = 'center';
      
      const labels = [
        { freq: 100, label: '100' },
        { freq: 1000, label: '1k' },
        { freq: 10000, label: '10k' }
      ];
      
      labels.forEach(({ freq, label }) => {
        const x = (Math.log10(freq) - 1) / 4 * width;
        if (x >= 0 && x <= width) {
          ctx.fillText(label, x, height - 5);
        }
      });
    }

    // Draw spectrum
    const dataSource = fftData.length > 1 ? Array.from(fftData.slice(0, 256)) : analyzerData;
    const barWidth = width / dataSource.length;
    
    dataSource.forEach((magnitude, i) => {
      const x = i * barWidth;
      const normalizedMag = Math.max(0, Math.min(1, magnitude));
      const barHeight = normalizedMag * height;
      const y = height - barHeight;
      
      // Color based on mode
      let color = 'hsl(var(--audio-primary))';
      if (colorMode === 'rainbow') {
        const hue = 240 - (i / dataSource.length) * 180; // Blue to red
        color = `hsl(${hue}, 70%, 60%)`;
      } else if (colorMode === 'warm') {
        const warmth = 30 + (normalizedMag * 30); // Orange to yellow
        color = `hsl(${warmth}, 85%, ${50 + normalizedMag * 20}%)`;
      }
      
      // Add glow for higher frequencies
      if (normalizedMag > 0.3) {
        ctx.shadowBlur = 4;
        ctx.shadowColor = color;
      } else {
        ctx.shadowBlur = 0;
      }
      
      ctx.fillStyle = color;
      ctx.fillRect(x, y, barWidth - 1, barHeight);
    });

    // Draw peak line
    const maxIndex = dataSource.indexOf(Math.max(...dataSource));
    if (maxIndex >= 0) {
      const peakX = maxIndex * barWidth + barWidth / 2;
      ctx.strokeStyle = 'hsl(var(--audio-accent))';
      ctx.lineWidth = 2;
      ctx.setLineDash([4, 4]);
      ctx.beginPath();
      ctx.moveTo(peakX, 0);
      ctx.lineTo(peakX, height);
      ctx.stroke();
      ctx.setLineDash([]);
    }

  }, [analyzerData, fftData, width, height, showFreqLabels, showGrid, colorMode]);

  return (
    <div className="relative bg-plugin-surface rounded-lg border border-plugin-raised overflow-hidden">
      <canvas 
        ref={canvasRef} 
        width={width} 
        height={height}
        className="block"
        style={{ width: `${width}px`, height: `${height}px` }}
      />
      
      {/* Overlay controls */}
      <div className="absolute top-2 right-2 flex gap-1">
        <button 
          onClick={() => {}} 
          className="px-2 py-1 text-xs bg-plugin-panel/80 text-foreground/70 rounded border border-plugin-raised/50 hover:bg-plugin-raised/80 transition-colors"
        >
          FREEZE
        </button>
        <button 
          onClick={() => {}} 
          className="px-2 py-1 text-xs bg-plugin-panel/80 text-foreground/70 rounded border border-plugin-raised/50 hover:bg-plugin-raised/80 transition-colors"
        >
          PEAK
        </button>
      </div>
    </div>
  );
};