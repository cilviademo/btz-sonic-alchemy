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

    // Clear canvas with deep dark gradient background like Output interfaces
    const gradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, baseRadius * 2);
    gradient.addColorStop(0, 'hsl(220, 25%, 8%)');
    gradient.addColorStop(0.5, 'hsl(220, 20%, 5%)');
    gradient.addColorStop(1, 'hsl(220, 15%, 2%)');
    
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

    // Draw central waveform with Output-style gradient colors
    if (waveformData.length > 0) {
      // Create colorful gradient like in the reference images
      const waveGradient = ctx.createLinearGradient(centerX - baseRadius, centerY, centerX + baseRadius, centerY);
      waveGradient.addColorStop(0, '#00d4ff'); // Cyan
      waveGradient.addColorStop(0.25, '#8a2be2'); // Purple
      waveGradient.addColorStop(0.5, '#ff1493'); // Pink
      waveGradient.addColorStop(0.75, '#ff8c00'); // Orange
      waveGradient.addColorStop(1, '#ffff00'); // Yellow
      
      ctx.strokeStyle = waveGradient;
      ctx.lineWidth = 4;
      ctx.shadowColor = '#ff1493';
      ctx.shadowBlur = 12;
      
      ctx.beginPath();
      
      for (let i = 0; i < waveformData.length; i++) {
        const angle = (i / waveformData.length) * Math.PI * 2;
        const radius = baseRadius * 0.6 + waveformData[i] * 25;
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
        width={280}
        height={280}
        className="rounded-full border-3 border-foreground/10"
        style={{
          background: `conic-gradient(from 0deg, 
            hsl(220, 15%, 8%), hsl(220, 12%, 12%), hsl(220, 15%, 8%), 
            hsl(220, 18%, 6%), hsl(220, 15%, 8%)
          )`,
          boxShadow: `
            0 0 40px hsl(var(--audio-primary) / 0.3), 
            inset 0 6px 24px rgba(0,0,0,0.8),
            inset 0 -6px 12px rgba(255,255,255,0.05)
          `
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