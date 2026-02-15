import React from 'react';

interface MicroParam {
  label: string;
  value: string;
  unit?: string;
}

export const MicroParams: React.FC<{ params: MicroParam[] }> = ({ params }) => {
  if (params.length === 0) return null;

  return (
    <div className="mt-1 flex flex-wrap gap-x-3 gap-y-0.5 text-[9px] uppercase opacity-60">
      {params.map((p, i) => (
        <span key={i} className="whitespace-nowrap">
          <span className="opacity-70">{p.label}</span>
          {' '}
          <span className="font-mono opacity-90">{p.value}</span>
          {p.unit && <span className="opacity-60">{p.unit}</span>}
        </span>
      ))}
    </div>
  );
};
