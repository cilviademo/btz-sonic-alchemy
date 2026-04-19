"""
BTZ Sonic Alchemy — ADAA Alias Rejection Investigation
═══════════════════════════════════════════════════════════
Addresses critique item #1:
  "Current report claims 2.4 dB rejection, which is far below expected (18–36 dB).
   Investigate root cause."

Tests:
  A. Verify F1/x1 state persists between process() calls
  B. Measure epsilon fallback firing rate over 10s test signal
  C. Verify logCosh numerical stability at extreme inputs
  D. Measure alias rejection by identifying SPECIFIC reflected harmonics
     (not broadband), report before/after ADAA dB delta
"""
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from scipy.signal.windows import blackmanharris
import os, json

SR = 48000.0
DURATION = 10.0  # seconds
FREQ = 5000.0
DRIVE_DB = 12.0
DRIVE_LIN = 10.0 ** (DRIVE_DB / 20.0)

os.makedirs('/home/ubuntu/tests_validation/results', exist_ok=True)

# ═══════════════════════════════════════════════════════════════════════════
# Python reimplementation of BTZDsp::ADAATanh (exact C++ port)
# ═══════════════════════════════════════════════════════════════════════════
def logCosh(x):
    """Numerically stable: |x| + log1p(exp(-2|x|)) - ln2"""
    ax = np.abs(x)
    return ax + np.log1p(np.exp(-2.0 * ax)) - 0.6931472

class ADAATanh:
    def __init__(self):
        self.x1 = 0.0
        self.F1 = 0.0  # logCosh(0) = 0
        self.epsilon_count = 0
        self.total_count = 0

    def reset(self):
        self.x1 = 0.0
        self.F1 = 0.0
        self.epsilon_count = 0
        self.total_count = 0

    def process(self, x):
        self.total_count += 1
        dx = x - self.x1
        F = logCosh(x)

        if abs(dx) < 1.0e-5:
            self.epsilon_count += 1
            y = np.tanh(0.5 * (x + self.x1))
        else:
            y = (F - self.F1) / dx

        self.x1 = x
        self.F1 = F
        return y

# ═══════════════════════════════════════════════════════════════════════════
# Test A: State persistence verification
# ═══════════════════════════════════════════════════════════════════════════
print("=" * 70)
print("TEST A: State Persistence Verification")
print("=" * 70)

adaa = ADAATanh()
# Process a few samples
for i in range(10):
    x = 0.5 * np.sin(2 * np.pi * 1000 * i / SR)
    adaa.process(x)

# Check that state persists
x1_after = adaa.x1
F1_after = adaa.F1
assert x1_after != 0.0, "FAIL: x1 was reset to zero"
assert F1_after != 0.0, "FAIL: F1 was reset to zero"
print(f"  x1 = {x1_after:.8f} (non-zero: PASS)")
print(f"  F1 = {F1_after:.8f} (non-zero: PASS)")

# Process more samples and verify state continues
adaa.process(0.3)
assert adaa.x1 == 0.3, "FAIL: x1 not updated"
print(f"  After process(0.3): x1 = {adaa.x1:.8f} (== 0.3: PASS)")
print("  RESULT: State persists correctly between calls.")

# ═══════════════════════════════════════════════════════════════════════════
# Test B: Epsilon fallback firing rate over 10s
# ═══════════════════════════════════════════════════════════════════════════
print("\n" + "=" * 70)
print("TEST B: Epsilon Fallback Firing Rate (10s, 5 kHz sine, +12 dB drive)")
print("=" * 70)

N = int(SR * DURATION)
t = np.arange(N) / SR
sine = np.sin(2 * np.pi * FREQ * t)
driven = sine * DRIVE_LIN

adaa_test = ADAATanh()
adaa_test.reset()
for i in range(N):
    adaa_test.process(driven[i])

eps_rate = adaa_test.epsilon_count / adaa_test.total_count * 100.0
print(f"  Total samples:    {adaa_test.total_count}")
print(f"  Epsilon fires:    {adaa_test.epsilon_count}")
print(f"  Epsilon rate:     {eps_rate:.4f}%")
print(f"  THRESHOLD: < 5% is acceptable for a 5 kHz sine")
if eps_rate < 5.0:
    print(f"  RESULT: PASS ({eps_rate:.4f}% < 5%)")
else:
    print(f"  RESULT: FAIL ({eps_rate:.4f}% >= 5%) — epsilon firing too often!")

# Also test at DC-like (very slow) signal where epsilon should fire more
adaa_dc = ADAATanh()
adaa_dc.reset()
dc_signal = np.full(N, 0.5)  # constant
for i in range(N):
    adaa_dc.process(dc_signal[i])
dc_eps_rate = adaa_dc.epsilon_count / adaa_dc.total_count * 100.0
print(f"\n  DC signal epsilon rate: {dc_eps_rate:.4f}%")
print(f"  (Expected: ~100% for constant input, which is correct behavior)")

# ═══════════════════════════════════════════════════════════════════════════
# Test C: logCosh numerical stability at extreme inputs
# ═══════════════════════════════════════════════════════════════════════════
print("\n" + "=" * 70)
print("TEST C: logCosh Numerical Stability")
print("=" * 70)

test_values = [0.0, 0.001, 0.1, 1.0, 5.0, 10.0, 50.0, 100.0, 500.0, 1000.0, -500.0, -1000.0]
all_stable = True
for x in test_values:
    result = logCosh(x)
    ref = np.log(np.cosh(np.clip(x, -500, 500)))  # clip for reference
    is_finite = np.isfinite(result)
    if not is_finite:
        all_stable = False
    # For large |x|, logCosh(x) ≈ |x| - ln(2)
    expected_large = abs(x) - 0.6931472
    print(f"  logCosh({x:>8.1f}) = {result:>12.6f}  finite={is_finite}  "
          f"(expected ≈ {expected_large:>12.6f} for large |x|)")

print(f"\n  RESULT: {'PASS — all values finite' if all_stable else 'FAIL — non-finite values detected'}")

# ═══════════════════════════════════════════════════════════════════════════
# Test D: Measure SPECIFIC reflected harmonics (the critical test)
# ═══════════════════════════════════════════════════════════════════════════
print("\n" + "=" * 70)
print("TEST D: Reflected Harmonic Alias Rejection Measurement")
print("=" * 70)
print(f"  Signal: {FREQ} Hz sine at +{DRIVE_DB} dB drive")
print(f"  SR: {SR} Hz, Nyquist: {SR/2} Hz")
print()

# Generate test signal
N_fft = int(SR * 2.0)  # 2 seconds for good frequency resolution
t_fft = np.arange(N_fft) / SR
test_sine = np.sin(2 * np.pi * FREQ * t_fft) * DRIVE_LIN

# Process through naive tanh
naive_out = np.tanh(test_sine)

# Process through ADAA
adaa_measure = ADAATanh()
adaa_measure.reset()
adaa_out = np.zeros(N_fft)
for i in range(N_fft):
    adaa_out[i] = adaa_measure.process(test_sine[i])

# FFT analysis with Blackman-Harris window
window = blackmanharris(N_fft)
freq_axis = np.fft.rfftfreq(N_fft, 1.0 / SR)

naive_fft = np.abs(np.fft.rfft(naive_out * window)) / (N_fft / 2)
adaa_fft = np.abs(np.fft.rfft(adaa_out * window)) / (N_fft / 2)

# Convert to dB
naive_db = 20 * np.log10(naive_fft + 1e-20)
adaa_db = 20 * np.log10(adaa_fft + 1e-20)

# Identify the harmonics of 5 kHz
# Harmonics: 5, 10, 15, 20, 25, 30, 35, 40, 45 kHz
# At 48 kHz SR, Nyquist = 24 kHz
# Harmonics above Nyquist reflect:
#   25 kHz -> reflects to 48-25 = 23 kHz
#   30 kHz -> reflects to 48-30 = 18 kHz
#   35 kHz -> reflects to 48-35 = 13 kHz
#   40 kHz -> reflects to 48-40 = 8 kHz
#   45 kHz -> reflects to 48-45 = 3 kHz

# True harmonics (below Nyquist)
true_harmonics = []
h = 1
while h * FREQ < SR / 2:
    true_harmonics.append(h * FREQ)
    h += 1

# Reflected aliases
reflected_aliases = []
h_alias = h  # first harmonic above Nyquist
while h_alias * FREQ < SR * 2:  # reasonable range
    alias_freq = abs(SR - h_alias * FREQ)
    if 0 < alias_freq < SR / 2:
        reflected_aliases.append((h_alias, alias_freq))
    h_alias += 1

print(f"  True harmonics below Nyquist: {true_harmonics}")
print(f"  Reflected aliases:")
for h_num, a_freq in reflected_aliases:
    print(f"    Harmonic {h_num} ({h_num * FREQ} Hz) -> reflects to {a_freq:.0f} Hz")

# Measure energy at each reflected alias frequency
freq_resolution = SR / N_fft
search_width = 3  # bins on each side

def measure_peak_at_freq(fft_db, freq_target, freq_axis):
    """Find peak energy within ±search_width bins of target frequency"""
    idx = np.argmin(np.abs(freq_axis - freq_target))
    lo = max(0, idx - search_width)
    hi = min(len(fft_db), idx + search_width + 1)
    return np.max(fft_db[lo:hi])

print(f"\n  Freq resolution: {freq_resolution:.2f} Hz")
print(f"\n  {'Alias Freq':>12s}  {'Naive (dB)':>12s}  {'ADAA (dB)':>12s}  {'Delta (dB)':>12s}")
print("  " + "-" * 56)

alias_naive_energies = []
alias_adaa_energies = []

for h_num, a_freq in reflected_aliases:
    naive_level = measure_peak_at_freq(naive_db, a_freq, freq_axis)
    adaa_level = measure_peak_at_freq(adaa_db, a_freq, freq_axis)
    delta = naive_level - adaa_level
    alias_naive_energies.append(naive_level)
    alias_adaa_energies.append(adaa_level)
    print(f"  {a_freq:>10.0f} Hz  {naive_level:>12.1f}  {adaa_level:>12.1f}  {delta:>+12.1f}")

# Aggregate: total alias energy (sum of linear powers)
naive_alias_total = 10 * np.log10(sum(10**(e/10) for e in alias_naive_energies) + 1e-20)
adaa_alias_total = 10 * np.log10(sum(10**(e/10) for e in alias_adaa_energies) + 1e-20)
total_delta = naive_alias_total - adaa_alias_total

print(f"\n  Total alias energy (naive):  {naive_alias_total:.1f} dB")
print(f"  Total alias energy (ADAA):   {adaa_alias_total:.1f} dB")
print(f"  Total rejection delta:       {total_delta:+.1f} dB")
print(f"\n  THRESHOLD: >= 18 dB for correctly implemented ADAA-1")
if total_delta >= 18.0:
    print(f"  RESULT: PASS ({total_delta:.1f} dB >= 18 dB)")
elif total_delta >= 6.0:
    print(f"  RESULT: MARGINAL ({total_delta:.1f} dB — better than naive but below ADAA-1 theoretical)")
else:
    print(f"  RESULT: FAIL ({total_delta:.1f} dB < 18 dB — ADAA implementation likely broken)")

# ═══════════════════════════════════════════════════════════════════════════
# Diagnostic: Compare ADAA output to direct tanh to see if ADAA is working
# ═══════════════════════════════════════════════════════════════════════════
print("\n" + "=" * 70)
print("DIAGNOSTIC: ADAA vs Direct Tanh Waveform Comparison")
print("=" * 70)

# Check if ADAA output differs meaningfully from direct tanh
diff = adaa_out - naive_out
diff_rms = np.sqrt(np.mean(diff**2))
naive_rms = np.sqrt(np.mean(naive_out**2))
diff_ratio_db = 20 * np.log10(diff_rms / naive_rms + 1e-20)
print(f"  RMS difference between ADAA and naive: {diff_rms:.6f}")
print(f"  Relative to signal: {diff_ratio_db:.1f} dB")
print(f"  (If close to 0 dB, ADAA is barely changing the signal)")

# ═══════════════════════════════════════════════════════════════════════════
# Plot: Spectrum comparison with reflected harmonics marked
# ═══════════════════════════════════════════════════════════════════════════
fig, axes = plt.subplots(2, 1, figsize=(14, 10))

# Full spectrum
ax = axes[0]
ax.plot(freq_axis, naive_db, color='#ff6b6b', alpha=0.7, linewidth=0.5, label='Naive tanh')
ax.plot(freq_axis, adaa_db, color='#4ecdc4', alpha=0.7, linewidth=0.5, label='ADAA-1 tanh')
for h_num, a_freq in reflected_aliases:
    ax.axvline(a_freq, color='#ffd93d', alpha=0.5, linestyle='--', linewidth=0.8)
ax.set_xlim(0, SR/2)
ax.set_ylim(-120, 0)
ax.set_xlabel('Frequency (Hz)')
ax.set_ylabel('Magnitude (dB)')
ax.set_title(f'ADAA Alias Rejection — {FREQ:.0f} Hz @ +{DRIVE_DB} dB drive, SR={SR:.0f} Hz')
ax.legend()
ax.grid(True, alpha=0.3)

# Zoomed to alias region
ax2 = axes[1]
for h_num, a_freq in reflected_aliases:
    lo_freq = a_freq - 200
    hi_freq = a_freq + 200
    mask = (freq_axis >= lo_freq) & (freq_axis <= hi_freq)
    if np.any(mask):
        ax2.plot(freq_axis[mask], naive_db[mask], color='#ff6b6b', alpha=0.7, linewidth=1.0)
        ax2.plot(freq_axis[mask], adaa_db[mask], color='#4ecdc4', alpha=0.7, linewidth=1.0)
        ax2.axvline(a_freq, color='#ffd93d', alpha=0.5, linestyle='--', linewidth=0.8)
        ax2.annotate(f'{a_freq:.0f} Hz\n(H{h_num})', xy=(a_freq, -40), fontsize=7,
                     ha='center', color='#ffd93d')

ax2.set_ylim(-120, 0)
ax2.set_xlabel('Frequency (Hz)')
ax2.set_ylabel('Magnitude (dB)')
ax2.set_title('Zoomed: Reflected Alias Frequencies')
ax2.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('/home/ubuntu/tests_validation/results/01_adaa_alias_rejection.png', dpi=150)
plt.close()

# ═══════════════════════════════════════════════════════════════════════════
# Save results as JSON for the final report
# ═══════════════════════════════════════════════════════════════════════════
results = {
    "test": "ADAA Alias Rejection Investigation",
    "state_persistence": "PASS",
    "epsilon_rate_pct": round(eps_rate, 4),
    "epsilon_rate_pass": eps_rate < 5.0,
    "logcosh_stability": "PASS" if all_stable else "FAIL",
    "alias_rejection_db": round(total_delta, 1),
    "alias_rejection_pass": total_delta >= 18.0,
    "naive_alias_total_db": round(naive_alias_total, 1),
    "adaa_alias_total_db": round(adaa_alias_total, 1),
    "per_harmonic": [
        {"harmonic": h_num, "alias_freq_hz": round(a_freq, 0),
         "naive_db": round(n_db, 1), "adaa_db": round(a_db, 1),
         "delta_db": round(n_db - a_db, 1)}
        for (h_num, a_freq), n_db, a_db in zip(reflected_aliases, alias_naive_energies, alias_adaa_energies)
    ]
}

with open('/home/ubuntu/tests_validation/results/01_adaa_results.json', 'w') as f:
    json.dump(results, f, indent=2)

print(f"\nResults saved to /home/ubuntu/tests_validation/results/")
print("Done.")
