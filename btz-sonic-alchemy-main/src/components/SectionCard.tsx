export function SectionCard({ title, subtitle, right, children }:{
  title:string; subtitle?:string; right?:React.ReactNode; children:React.ReactNode;
}){
  return (
    <section className="rounded-2xl border border-white/6"
      style={{ background:'var(--btz-panel)', boxShadow:'inset 0 1px 0 rgba(255,255,255,.05), var(--btz-shadow)' }}>
      <header className="flex items-start justify-between gap-4 p-5 sm:p-6 border-b border-white/5">
        <div>
          <h3 className="text-base sm:text-lg font-bold tracking-wide" style={{color:'var(--btz-ink)'}}>{title}</h3>
          {subtitle && <p className="text-xs sm:text-[13px]" style={{color:'var(--btz-ink-muted)'}}>{subtitle}</p>}
        </div>
        {right}
      </header>
      <div className="p-5 sm:p-6">{children}</div>
    </section>
  );
}
