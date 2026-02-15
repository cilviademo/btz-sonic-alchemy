import React, { useState } from 'react';
import { BTZPluginState } from './types';
import { cn } from '@/lib/utils';

interface ModulationMatrixProps {
  state: BTZPluginState;
  updateParameter: (key: keyof BTZPluginState, value: any) => void;
}

const SOURCES = [
  { id: 'lfo1', label: 'LFO 1', color: 'hsl(var(--audio-primary))' },
  { id: 'lfo2', label: 'LFO 2', color: 'hsl(var(--audio-secondary))' },
  { id: 'env1', label: 'ENV 1', color: 'hsl(var(--audio-tertiary))' },
  { id: 'seq1', label: 'SEQ 1', color: 'hsl(var(--audio-success))' },
];

const DESTINATIONS = [
  { id: 'punch', label: 'Punch' },
  { id: 'warmth', label: 'Warmth' },
  { id: 'boom', label: 'Boom' },
  { id: 'drive', label: 'Drive' },
  { id: 'mix', label: 'Mix' },
];

export const ModulationMatrix: React.FC<ModulationMatrixProps> = ({
  state,
  updateParameter
}) => {
  const [connections, setConnections] = useState<Record<string, { source: string; amount: number }>>({});

  const handleConnectionToggle = (source: string, dest: string) => {
    const key = `${source}-${dest}`;
    setConnections(prev => ({
      ...prev,
      [key]: prev[key] ? undefined : { source, amount: 0.5 }
    }));
  };

  return (
    <div className="bg-plugin-panel rounded-2xl p-6 border border-audio-primary/10">
      <h3 className="text-lg font-bold text-center text-foreground mb-6 tracking-wide">
        MODULATION MATRIX
      </h3>
      
      <div className="space-y-6">
        {/* Matrix Grid */}
        <div className="grid gap-2" style={{ gridTemplateColumns: 'auto repeat(5, 1fr)' }}>
          {/* Header */}
          <div></div>
          {DESTINATIONS.map(dest => (
            <div key={dest.id} className="text-xs font-bold text-center text-foreground/80 p-2 uppercase">
              {dest.label}
            </div>
          ))}
          
          {/* Matrix Rows */}
          {SOURCES.map(source => (
            <React.Fragment key={source.id}>
              <div className="text-xs font-bold text-foreground/80 p-2 uppercase flex items-center">
                <div 
                  className="w-3 h-3 rounded-full mr-2" 
                  style={{ background: source.color }}
                ></div>
                {source.label}
              </div>
              {DESTINATIONS.map(dest => {
                const key = `${source.id}-${dest.id}`;
                const connection = connections[key];
                const isConnected = !!connection;
                
                return (
                  <button
                    key={key}
                    onClick={() => handleConnectionToggle(source.id, dest.id)}
                    className={cn(
                      "w-8 h-8 rounded-lg border-2 transition-all duration-300 mx-auto",
                      isConnected 
                        ? "border-audio-primary bg-audio-primary/20" 
                        : "border-plugin-raised hover:border-audio-primary/50"
                    )}
                    style={isConnected ? {
                      boxShadow: `0 0 10px ${source.color}40`
                    } : {}}
                  >
                    {isConnected && (
                      <div 
                        className="w-2 h-2 rounded-full mx-auto"
                        style={{ background: source.color }}
                      ></div>
                    )}
                  </button>
                );
              })}
            </React.Fragment>
          ))}
        </div>

        {/* Active Connections */}
        <div className="space-y-2">
          <h4 className="text-sm font-bold text-foreground/80 uppercase tracking-wide">
            Active Connections
          </h4>
          <div className="space-y-2 max-h-32 overflow-y-auto">
            {Object.entries(connections).map(([key, conn]) => {
              if (!conn) return null;
              const [sourceId, destId] = key.split('-');
              const source = SOURCES.find(s => s.id === sourceId);
              const dest = DESTINATIONS.find(d => d.id === destId);
              
              return (
                <div key={key} className="flex items-center justify-between bg-plugin-surface rounded-lg p-3">
                  <div className="flex items-center gap-2 text-sm">
                    <div 
                      className="w-2 h-2 rounded-full" 
                      style={{ background: source?.color }}
                    ></div>
                    <span>{source?.label} → {dest?.label}</span>
                  </div>
                  <input
                    type="range"
                    min="-1"
                    max="1"
                    step="0.01"
                    value={conn.amount}
                    onChange={(e) => setConnections(prev => ({
                      ...prev,
                      [key]: { ...conn, amount: parseFloat(e.target.value) }
                    }))}
                    className="w-20 accent-audio-primary"
                  />
                </div>
              );
            })}
          </div>
        </div>
      </div>
    </div>
  );
};