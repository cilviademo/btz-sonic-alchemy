import React, { useEffect, useRef } from 'react';

interface CentralVisualizerProps {
  spectrumData: Float32Array;
  waveformData: Float32Array;
  isProcessing: boolean;
  level: number;
}

export const CentralVisualizer: React.FC<CentralVisualizerProps> = ({
  spectrumData,
  waveformData,
  isProcessing,
  level
}) => {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const width = canvas.width;
    const height = canvas.height;
    const centerX = width / 2;
    const centerY = height / 2;
    const baseRadius = Math.min(width, height) * 0.25;

    // Clear canvas with gradient background
    const gradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, baseRadius * 2);
    gradient.addColorStop(0, 'hsl(218 25% 12%)');
    gradient.addColorStop(0.7, 'hsl(218 20% 8%)');
    gradient.addColorStop(1, 'hsl(218 15% 6%)');
    
    ctx.fillStyle = gradient;
    ctx.fillRect(0, 0, width, height);

    // Draw circular spectrum analyzer
    if (spectrumData.length > 0) {
      const bars = Math.min(64, spectrumData.length);
      
      for (let i = 0; i < bars; i++) {
        const angle = (i / bars) * Math.PI * 2;
        const intensity = spectrumData[i] * level;
        const barHeight = intensity * 80;
        const x1 = centerX + Math.cos(angle) * (baseRadius + 10);
        const y1 = centerY + Math.sin(angle) * (baseRadius + 10);
        const x2 = centerX + Math.cos(angle) * (baseRadius + 10 + barHeight);
        const y2 = centerY + Math.sin(angle) * (baseRadius + 10 + barHeight);

        // Color based on frequency range
        let hue = 15; // Orange base
        if (i < bars * 0.2) hue = 5; // Deep red for bass
        else if (i < bars * 0.4) hue = 15; // Orange for low-mid
        else if (i < bars * 0.7) hue = 25; // Yellow for mid
        else hue = 35; // Light orange for highs

        const saturation = 80 + intensity * 15;
        const lightness = 50 + intensity * 20;
        
        ctx.strokeStyle = `hsl(${hue}, ${saturation}%, ${lightness}%)`;
        ctx.lineWidth = 3;
        ctx.lineCap = 'round';
        
        // Glow effect
        ctx.shadowColor = `hsl(${hue}, ${saturation}%, ${lightness}%)`;
        ctx.shadowBlur = 6 + intensity * 8;
        
        ctx.beginPath();
        ctx.moveTo(x1, y1);
        ctx.lineTo(x2, y2);
        ctx.stroke();
        
        ctx.shadowBlur = 0;
      }
    }

    // Draw central waveform
    if (waveformData.length > 0) {
      ctx.strokeStyle = 'hsl(15, 85%, 55%)';
      ctx.lineWidth = 2;
      ctx.shadowColor = 'hsl(15, 85%, 55%)';
      ctx.shadowBlur = 8;
      
      ctx.beginPath();
      
      for (let i = 0; i < waveformData.length; i++) {
        const angle = (i / waveformData.length) * Math.PI * 2;
        const radius = baseRadius * 0.6 + waveformData[i] * 20;
        const x = centerX + Math.cos(angle) * radius;
        const y = centerY + Math.sin(angle) * radius;
        
        if (i === 0) {
          ctx.moveTo(x, y);
        } else {
          ctx.lineTo(x, y);
        }
      }
      
      ctx.closePath();
      ctx.stroke();
      ctx.shadowBlur = 0;
    }

    // Draw processing indicator
    if (isProcessing) {
      const pulseRadius = baseRadius * 0.3 + Math.sin(Date.now() * 0.01) * 5;
      
      ctx.beginPath();
      ctx.arc(centerX, centerY, pulseRadius, 0, Math.PI * 2);
      
      const pulseGradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, pulseRadius);
      pulseGradient.addColorStop(0, 'hsl(15, 85%, 55%)');
      pulseGradient.addColorStop(0.7, 'hsl(15, 85%, 55%)40');
      pulseGradient.addColorStop(1, 'transparent');
      
      ctx.fillStyle = pulseGradient;
      ctx.fill();
    }

    // Draw level meter ring
    const levelAngle = level * Math.PI * 2;
    ctx.strokeStyle = 'hsl(15, 85%, 55%)';
    ctx.lineWidth = 4;
    ctx.shadowColor = 'hsl(15, 85%, 55%)';
    ctx.shadowBlur = 10;
    
    ctx.beginPath();
    ctx.arc(centerX, centerY, baseRadius * 1.2, -Math.PI / 2, -Math.PI / 2 + levelAngle);
    ctx.stroke();
    ctx.shadowBlur = 0;

  }, [spectrumData, waveformData, isProcessing, level]);

  return (
    <div className="relative">
      <canvas 
        ref={canvasRef}
        width={320}
        height={320}
        className="rounded-full border-2 border-audio-primary/20"
        style={{
          background: 'radial-gradient(circle, hsl(218 25% 12%), hsl(218 15% 6%))',
          boxShadow: '0 0 40px hsl(var(--audio-primary) / 0.2), inset 0 0 40px hsl(218 15% 6%)'
        }}
      />
      
      {/* Center BTZ Logo */}
      <div className="absolute inset-0 flex items-center justify-center">
        <div className="text-2xl font-black text-audio-primary opacity-30 tracking-wider">
          BTZ
        </div>
      </div>
    </div>
  );
};