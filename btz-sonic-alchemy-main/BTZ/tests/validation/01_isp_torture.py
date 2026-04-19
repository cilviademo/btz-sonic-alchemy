#!/usr/bin/env python3
"""
BTZ v7 — ISP (Inter-Sample Peak) Torture Test

Requires: compiled BTZ plugin + a host that can render offline (e.g., REAPER
command-line render, or a custom JUCE AudioPluginHost test harness).

This script generates a 60-second torture signal and measures the output
at 4x oversampled true-peak resolution.

Pass criteria: max ISP ≤ +0.2 dBTP above sparkCeiling.
"""
import numpy as np
from scipy.signal import resample_poly
from scipy.io import wavfile
import argparse
import os
import sys

def generate_torture_signal(sr=48000, duration=60.0):
    """Generate a worst-case ISP torture signal:
    - Swept sine from 20 Hz to 20 kHz
    - Clipped square wave bursts
    - Transient impulses
    - Pink noise at 0 dBFS
    """
    n = int(sr * duration)
    t = np.arange(n) / sr
    sig = np.zeros(n)

    # Segment 1 (0-15s): Swept sine 20 Hz → 20 kHz at 0 dBFS
    seg1 = int(15 * sr)
    freq = np.logspace(np.log10(20), np.log10(20000), seg1)
    phase = np.cumsum(2 * np.pi * freq / sr)
    sig[:seg1] = np.sin(phase)

    # Segment 2 (15-30s): Hard-clipped square wave at various frequencies
    seg2_start = seg1
    seg2_end = int(30 * sr)
    for f in [100, 500, 2000, 8000, 16000]:
        chunk = int(3 * sr)
        t_chunk = np.arange(chunk) / sr
        square = np.sign(np.sin(2 * np.pi * f * t_chunk))
        end = min(seg2_start + chunk, seg2_end)
        sig[seg2_start:end] = square[:end - seg2_start]
        seg2_start = end

    # Segment 3 (30-45s): Transient impulses (worst case for limiters)
    seg3_start = int(30 * sr)
    seg3_end = int(45 * sr)
    impulse_interval = int(0.01 * sr)  # 10ms apart
    for i in range(seg3_start, seg3_end, impulse_interval):
        if i < n:
            sig[i] = 1.0
            if i + 1 < n:
                sig[i + 1] = -1.0

    # Segment 4 (45-60s): Pink noise at 0 dBFS
    seg4_start = int(45 * sr)
    white = np.random.randn(n - seg4_start)
    # Simple pink filter (Voss-McCartney approximation)
    b = [0.049922035, -0.095993537, 0.050612699, -0.004709510]
    a = [1.0, -2.494956002, 2.017265875, -0.522189400]
    from scipy.signal import lfilter
    pink = lfilter(b, a, white)
    pink = pink / (np.max(np.abs(pink)) + 1e-10)
    sig[seg4_start:] = pink

    # Normalize to 0 dBFS
    sig = sig / (np.max(np.abs(sig)) + 1e-10) * 0.999

    return sig.astype(np.float32), sr


def measure_true_peak(signal, sr, oversample=4):
    """Measure true peak via 4x oversampling (ITU-R BS.1770-4 method)."""
    upsampled = resample_poly(signal, oversample, 1)
    true_peak_lin = np.max(np.abs(upsampled))
    true_peak_dbtp = 20 * np.log10(true_peak_lin + 1e-20)
    return true_peak_dbtp, true_peak_lin


def main():
    parser = argparse.ArgumentParser(description="BTZ ISP Torture Test")
    parser.add_argument("--generate-only", action="store_true",
                        help="Only generate the torture signal WAV")
    parser.add_argument("--measure", type=str, default=None,
                        help="Path to rendered output WAV to measure")
    parser.add_argument("--ceiling", type=float, default=-0.3,
                        help="Expected sparkCeiling in dBFS (default: -0.3)")
    parser.add_argument("--output-dir", type=str, default=".",
                        help="Directory to save test files")
    args = parser.parse_args()

    os.makedirs(args.output_dir, exist_ok=True)

    if args.generate_only or args.measure is None:
        print("Generating 60s ISP torture signal...")
        sig, sr = generate_torture_signal()
        path = os.path.join(args.output_dir, "isp_torture_input.wav")
        wavfile.write(path, sr, sig)
        print(f"  Saved: {path}")
        print(f"  Duration: {len(sig)/sr:.1f}s, SR: {sr} Hz")
        print(f"  Peak: {20*np.log10(np.max(np.abs(sig))+1e-20):.2f} dBFS")
        print()
        print("Next steps:")
        print("  1. Load this WAV into your DAW")
        print("  2. Insert BTZ on the channel (sparkCeiling = {:.1f} dB)".format(args.ceiling))
        print("  3. Render offline to WAV")
        print("  4. Run: python3 01_isp_torture.py --measure output.wav --ceiling {:.1f}".format(args.ceiling))

    if args.measure:
        print(f"Measuring true peak of: {args.measure}")
        sr, data = wavfile.read(args.measure)
        if data.dtype == np.int16:
            data = data.astype(np.float32) / 32768.0
        elif data.dtype == np.int32:
            data = data.astype(np.float32) / 2147483648.0

        if data.ndim == 2:
            # Stereo — measure both channels
            for ch in range(data.shape[1]):
                tp_db, tp_lin = measure_true_peak(data[:, ch], sr)
                overshoot = tp_db - args.ceiling
                status = "PASS" if overshoot <= 0.2 else "FAIL"
                print(f"  Ch{ch}: True Peak = {tp_db:.3f} dBTP, "
                      f"Overshoot = {overshoot:+.3f} dB  [{status}]")
        else:
            tp_db, tp_lin = measure_true_peak(data, sr)
            overshoot = tp_db - args.ceiling
            status = "PASS" if overshoot <= 0.2 else "FAIL"
            print(f"  True Peak = {tp_db:.3f} dBTP, "
                  f"Overshoot = {overshoot:+.3f} dB  [{status}]")

        print()
        print(f"Pass criteria: overshoot ≤ +0.2 dB above ceiling ({args.ceiling:.1f} dBFS)")


if __name__ == "__main__":
    main()
