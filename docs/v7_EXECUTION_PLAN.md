# BTZ Sonic Alchemy: v7 Execution Plan

Based on the competitive audit, here is the ranked implementation plan to close the most critical gaps separating BTZ from industry-leading plugins. The ranking formula is `(advantage gained) / (cost to achieve)`.

## 1. Remove ADAA, Implement 8x Oversampling
* **Competitor Benchmark:** FabFilter Pro-L 2 (>120 dB alias rejection) [1].
* **Current BTZ:** -18.6 dB (ADAA-1 + 4x OS).
* **Required Change:** Remove the `ADAATanh` class entirely. It is actively degrading alias rejection when combined with oversampling. Replace it with plain `fastTanh` and increase the oversampling factor to 8x for the `Ultra` quality mode.
* **Success Measurement:** Alias rejection improves from -18.6 dB to <-58.0 dB (measured via `measure_all_v2.py`).
* **Cost:** Low development time (deleting code, changing a constant). Moderate CPU cost increase (8x OS vs 4x OS).
* **Value:** True differentiator. -18.6 dB is audible aliasing; -58.0 dB is commercially acceptable.

## 2. Fix True-Peak Limiter Lookahead Alignment
* **Competitor Benchmark:** FabFilter Pro-L 2 (-0.0 dBTP guaranteed) [1].
* **Current BTZ:** +0.33 dB overshoot.
* **Required Change:** The `TruePeakLimiter` lookahead scanning is misaligned with the 4x oversampled sidechain. The lookahead window must scan the exact same samples that will be processed, accounting for the oversampling ratio.
* **Success Measurement:** ISP overshoot drops from +0.33 dB to <0.2 dB (measured via `measure_all_v2.py`).
* **Cost:** Moderate development time (math fix in C++). Zero CPU cost.
* **Value:** Parity. A mastering limiter must be true-peak compliant.

## 3. Fix Null-Test Transparency (DC Blocker Cutoff)
* **Competitor Benchmark:** iZotope Ozone (<-140 dBFS null) [2].
* **Current BTZ:** -46.0 dB peak delta, -10.1 dB relative delta on pink noise.
* **Required Change:** The `SafetyLayer` DC blockers use a 5 Hz cutoff, which is too aggressive and causes massive phase shift. Lower the cutoff to 1 Hz, or remove the output DC blocker entirely.
* **Success Measurement:** Full-chain pink noise relative delta improves from -10.1 dB to <-60.0 dB (measured via `measure_all_v2.py`).
* **Cost:** Low development time (changing a constant). Zero CPU cost.
* **Value:** Parity. The plugin must not color the sound when all parameters are neutral.

---

### References
[1] FabFilter. "Pro-L 2 Help - Oversampling." https://www.fabfilter.com/help/pro-l/using/oversampling
[2] iZotope. "Ozone 11 Advanced." https://www.izotope.com/en/products/ozone.html
