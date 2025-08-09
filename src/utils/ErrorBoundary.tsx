import React from 'react';

export class ErrorBoundary extends React.Component<{ children: React.ReactNode }, { hasError: boolean; msg?: string }> {
  constructor(props: any) {
    super(props);
    this.state = { hasError: false };
  }
  static getDerivedStateFromError(err: any) {
    return { hasError: true, msg: String(err?.message || err) };
  }
  componentDidCatch(err: any, info: any) {
    if (process.env.NODE_ENV !== 'production') console.error('BTZ UI crash:', err, info);
  }
  render() {
    if (this.state.hasError) {
      return (
        <div className="p-6 text-foreground/80">
          <h3 className="font-bold mb-2">Something went wrong.</h3>
          <p className="text-sm opacity-80">{this.state.msg}</p>
        </div>
      );
    }
    return this.props.children as any;
  }
}
