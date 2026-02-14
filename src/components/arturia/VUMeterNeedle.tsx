import React from 'react';

export const VUMeterNeedle: React.FC<{
  value: number; /* 0..1 */
  label?: string;
}> = ({ value, label = 'VU' }) => {
  const angle = -50 + value * 100; // -50..+50 deg
  return (
    <div
      className="relative rounded-md"
      style={{ background: 'var(--art-display)', height: 90, boxShadow:'inset 0 2px 10px rgba(0,0,0,.6)' }}
    >
      <svg viewBox="0 0 160 90" className="w-full h-full">
        <defs>
          <linearGradient id="vuGrad" x1="0" y1="0" x2="1" y2="0">
            <stop offset="65%" stopColor="#4ea1ff"/><stop offset="85%" stopColor="#ffb74d"/><stop offset="100%" stopColor="#ff5e57"/>
          </linearGradient>
        </defs>
        <path d="M20 75 A60 60 0 0 1 140 75" fill="none" stroke="url(#vuGrad)" strokeWidth="6" opacity="0.9"/>
        {/* ticks */}
        {Array.from({length:21}).map((_,i)=>{
          const t = -50 + i*5;
          const rad = (Math.PI/180)*(t+90);
          const x1 = 80 + Math.cos(rad)*50, y1 = 75 - Math.sin(rad)*50;
          const x2 = 80 + Math.cos(rad)*60, y2 = 75 - Math.sin(rad)*60;
          return <line key={i} x1={x1} y1={y1} x2={x2} y2={y2} stroke="#fff" opacity={i%2? .25:.5}/>;
        })}
        {/* needle */}
        <g transform={`rotate(${angle} 80 75)`}>
          <line x1="80" y1="75" x2="80" y2="18" stroke="#ff7247" strokeWidth="2"/>
          <circle cx="80" cy="75" r="3" fill="#ff7247"/>
        </g>
        <text x="80" y="86" fill="#fff" fontSize="10" textAnchor="middle" opacity=".8">{label}</text>
      </svg>
    </div>
  );
};
