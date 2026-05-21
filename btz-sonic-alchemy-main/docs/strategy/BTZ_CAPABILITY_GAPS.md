# BTZ Sonic Alchemy — Capability Gap Analysis

## 1. Critical Gaps (Must Fix for v1.0)

These are features that professional users assume exist in any premium plugin. Their absence will break trust immediately.

| Feature | Why It's Critical | Implementation Effort |
|---------|-------------------|-----------------------|
| **Typed Value Entry** | Pros need to type exact values (e.g., `-14.0` for LUFS). The Target Lock engine requires this. | Low (UI text editor overlay on knobs). |
| **Real Units Display** | Knobs showing `0-100%` instead of `Hz`, `ms`, or `dB` feels amateur. | Low (Update APVTS parameter formatting). |
| **HQ-on-Export** | Users want low CPU during playback but maximum oversampling during offline render. | Medium (Check `juce::AudioProcessor::isNonRealtime()` in `processBlock`). |

## 2. Important Gaps (Should Fix for v1.1)

These features elevate the plugin from "good" to "professional standard."

| Feature | Why It's Important | Implementation Effort |
|---------|--------------------|-----------------------|
| **Sidechain Input** | Essential for bus compression and dynamic EQ/saturation. | Medium (Requires JUCE bus layout changes). |
| **Vectorscope / Goniometer** | Since BTZ does Mid/Side processing, users need to see phase correlation visually, not just via a single number. | High (Requires new UI component and DSP buffer). |
| **Host Tempo Sync** | If LFOs remain, they must sync to the DAW tempo (e.g., 1/4 note, 1/8 note). | Medium (Query `juce::AudioPlayHead`). |

## 3. Future / Expansion Gaps (v2.0+)

| Feature | Why It's Future |
|---------|-----------------|
| **Modulation Routing Matrix** | Full drag-and-drop modulation is complex and dilutes the core "Target Lock" identity. Better suited for a dedicated sound design plugin. |
| **External IR Loading** | Allowing users to load their own impulse responses for the cabinet/room simulation. Nice to have, but complicates the UI and preset sharing. |

## 4. Unnecessary Gaps (Do Not Implement)

- **Built-in EQ:** BTZ is a saturation and dynamics processor. Adding a full parametric EQ turns it into a generic channel strip and ruins the focused identity. Let users use Pro-Q 3 for surgical EQ.
