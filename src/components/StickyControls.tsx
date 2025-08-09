export function StickyControls({ left, right }:{left?:React.ReactNode; right?:React.ReactNode;}){
  return (
    <div className="fixed bottom-0 left-0 right-0 z-40 px-3 pb-[env(safe-area-inset-bottom)] md:hidden">
      <div className="mx-auto max-w-4xl rounded-2xl border border-white/10 backdrop-blur-md"
        style={{ background:'rgba(10,10,16,.72)', boxShadow:'var(--btz-shadow)' }}>
        <div className="flex items-center justify-between gap-3 p-3">{left}{right}</div>
      </div>
    </div>
  );
}
