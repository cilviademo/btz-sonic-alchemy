import React from 'react';
import { cn } from '@/lib/utils';

interface PathNode {
  id: string;
  label: string;
  active: boolean;
  color?: string;
}

export const ActivePathStrip: React.FC<{ nodes: PathNode[] }> = ({ nodes }) => {
  return (
    <div className="mb-3 flex items-center gap-1 overflow-x-auto pb-1">
      <div className="text-[9px] uppercase tracking-widest opacity-40 mr-1">Signal:</div>
      {nodes.map((node, i) => (
        <React.Fragment key={node.id}>
          <div
            className={cn(
              "rounded px-2 py-0.5 text-[10px] font-medium uppercase tracking-wide transition-all",
              node.active
                ? "bg-white/10 text-white shadow-sm"
                : "bg-white/5 text-white/30"
            )}
            style={node.active && node.color ? { borderLeft: `2px solid ${node.color}` } : undefined}
            title={node.active ? `${node.label} (active)` : `${node.label} (bypassed)`}
          >
            {node.label}
          </div>
          {i < nodes.length - 1 && (
            <svg width="8" height="8" viewBox="0 0 8 8" className="opacity-30">
              <path d="M 2 4 L 6 4" stroke="currentColor" strokeWidth="1" />
            </svg>
          )}
        </React.Fragment>
      ))}
    </div>
  );
};
