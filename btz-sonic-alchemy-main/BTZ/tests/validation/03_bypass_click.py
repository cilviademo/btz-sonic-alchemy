#!/usr/bin/env python3
"""
BTZ v7 — Bypass Click Test

Tests that toggling bypass produces no audible click.
Requires: rendered WAV with bypass automation (on→off→on every 2 seconds).

Pass criteria: max sample-to-sample delta < 0.01 during bypass transitions.
"""
import numpy as np
from scipy.io import wavfile
import argparse
import os


def detect_clicks(signal, sr, threshold=0.01, window_ms=5.0):
    """Detect clicks by finding sample-to-sample deltas exceeding threshold."""
    diff = np.abs(np.diff(signal.astype(np.float64)))
    click_indices = np.where(diff > threshold)[0]

    clicks = []
    if len(click_indices) > 0:
        # Group nearby clicks
        groups = np.split(click_indices, np.where(np.diff(click_indices) > int(sr * window_ms / 1000))[0] + 1)
        for group in groups:
            if len(group) > 0:
                max_delta = diff[group].max()
                time_s = group[0] / sr
                clicks.append({
                    'time': time_s,
                    'max_delta': max_delta,
                    'sample': group[0]
                })

    return clicks


def main():
    parser = argparse.ArgumentParser(description="BTZ Bypass Click Test")
    parser.add_argument("--measure", type=str, required=True,
                        help="Path to rendered output WAV with bypass automation")
    parser.add_argument("--threshold", type=float, default=0.01,
                        help="Max acceptable sample-to-sample delta (default: 0.01)")
    args = parser.parse_args()

    print(f"Analyzing: {args.measure}")
    sr, data = wavfile.read(args.measure)
    if data.dtype == np.int16:
        data = data.astype(np.float32) / 32768.0
    elif data.dtype == np.int32:
        data = data.astype(np.float32) / 2147483648.0

    if data.ndim == 2:
        channels = [data[:, i] for i in range(data.shape[1])]
    else:
        channels = [data]

    all_pass = True
    for ch_idx, ch_data in enumerate(channels):
        clicks = detect_clicks(ch_data, sr, args.threshold)
        if clicks:
            all_pass = False
            print(f"  Ch{ch_idx}: FAIL — {len(clicks)} click(s) detected:")
            for c in clicks[:10]:  # Show first 10
                print(f"    t={c['time']:.4f}s  delta={c['max_delta']:.6f}  sample={c['sample']}")
        else:
            max_delta = np.max(np.abs(np.diff(ch_data.astype(np.float64))))
            print(f"  Ch{ch_idx}: PASS — max delta = {max_delta:.6f} (threshold: {args.threshold})")

    print()
    print(f"Overall: {'PASS' if all_pass else 'FAIL'}")
    print(f"Threshold: {args.threshold}")
    print()
    print("Test setup instructions:")
    print("  1. Load a sustained sine wave (440 Hz, -6 dBFS) into your DAW")
    print("  2. Insert BTZ on the channel with default settings")
    print("  3. Automate bypass: ON at 1s, OFF at 3s, ON at 5s, OFF at 7s")
    print("  4. Render offline to WAV (48 kHz, 32-bit float)")
    print("  5. Run: python3 03_bypass_click.py --measure output.wav")


if __name__ == "__main__":
    main()
