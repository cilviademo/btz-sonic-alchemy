/*
  Box Tone Zone (BTZ) — BTZComponents.h  v11
  ──────────────────────────────────────────────────────────────────────────
  Reusable UI components. Every colour and dimension comes from BTZTheme.h.
  Zero hardcoded hex values. Zero magic numbers.
  Namespace: btz::
  ──────────────────────────────────────────────────────────────────────────
*/
#pragma once

#include <JuceHeader.h>
#include "BTZTheme.h"
#include <array>
#include <cmath>
#include <vector>

namespace btz {

// ═══════════════════════════════════════════════════════════════════════════
// HarmonicVisualizer — real-time overtone bar display
// ═══════════════════════════════════════════════════════════════════════════

class HarmonicVisualizer : public juce::Component, private juce::Timer {
public:
    static constexpr int kMaxBars = 16;

    HarmonicVisualizer() {
        setOpaque(false);
        startTimerHz(anim::fps);
        current.fill(0.0f);
        target.fill(0.0f);
    }

    void setMagnitudes(const float* mags, int count) {
        const int n = juce::jmin(count, kMaxBars);
        for (int i = 0; i < n; ++i)
            target[static_cast<size_t>(i)] = juce::jlimit(0.0f, 1.0f, mags[i]);
        for (int i = n; i < kMaxBars; ++i)
            target[static_cast<size_t>(i)] = 0.0f;
    }

    void paint(juce::Graphics& g) override {
        const auto bounds = getLocalBounds().toFloat();

        // Background panel
        g.setColour(juce::Colour(palette::surface));
        g.fillRoundedRectangle(bounds, radius::md);

        // Subtle border
        g.setColour(juce::Colour(palette::border));
        g.drawRoundedRectangle(bounds.reduced(0.5f), radius::md, 0.5f);

        const float barW = 4.0f;
        const float gap  = 3.0f;
        const float totalW = kMaxBars * barW + (kMaxBars - 1) * gap;
        const float padY = 8.0f;
        const float maxH = bounds.getHeight() - padY * 2.0f - 12.0f;
        float x = bounds.getCentreX() - totalW * 0.5f;

        for (int i = 0; i < kMaxBars; ++i) {
            const auto colour = harmonicColour(i);

            // Track (faint)
            g.setColour(colour.withAlpha(0.08f));
            g.fillRoundedRectangle(x, padY, barW, maxH, 2.0f);

            // Active bar
            const float h = current[static_cast<size_t>(i)] * maxH;
            if (h > 0.5f) {
                const float y = padY + maxH - h;
                g.setColour(colour);
                g.fillRoundedRectangle(x, y, barW, h, 2.0f);
            }

            // Index label
            g.setColour(juce::Colour(palette::inkFaint));
            g.setFont(juce::Font(type::mono(), type::micro, juce::Font::plain));
            g.drawText(juce::String(i + 1),
                       juce::Rectangle<float>(x - 1.0f, bounds.getBottom() - 12.0f, barW + 2.0f, 10.0f),
                       juce::Justification::centred, false);

            x += barW + gap;
        }
    }

private:
    std::array<float, kMaxBars> target;
    std::array<float, kMaxBars> current;

    void timerCallback() override {
        bool changed = false;
        for (int i = 0; i < kMaxBars; ++i) {
            const float t = target[static_cast<size_t>(i)];
            float& c = current[static_cast<size_t>(i)];
            const float coeff = (t > c) ? 0.3f : 0.1f;
            const float prev = c;
            c += (t - c) * coeff;
            if (std::abs(c - prev) > 0.001f) changed = true;
        }
        if (changed) repaint();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HarmonicVisualizer)
};

// ═══════════════════════════════════════════════════════════════════════════
// GainReductionRibbon — horizontal GR history
// ═══════════════════════════════════════════════════════════════════════════

class GainReductionRibbon : public juce::Component {
public:
    static constexpr int kSize = 192;

    void push(float grDb) {
        buf[pos] = juce::jlimit(-24.0f, 0.0f, grDb);
        pos = (pos + 1) % kSize;
        repaint();
    }

    void paint(juce::Graphics& g) override {
        const auto bounds = getLocalBounds().toFloat();

        // Background
        g.setColour(juce::Colour(palette::surface));
        g.fillRoundedRectangle(bounds, radius::sm);

        // Path
        const float stepX = bounds.getWidth() / static_cast<float>(kSize);
        juce::Path path;
        path.startNewSubPath(bounds.getX(), bounds.getCentreY());

        for (int i = 0; i < kSize; ++i) {
            const int idx = (pos + i) % kSize;
            const float norm = -buf[idx] / 24.0f;
            const float y = bounds.getY() + bounds.getHeight() * 0.1f + norm * bounds.getHeight() * 0.8f;
            path.lineTo(bounds.getX() + i * stepX, y);
        }

        // Fill
        juce::Path fill = path;
        fill.lineTo(bounds.getRight(), bounds.getCentreY());
        fill.lineTo(bounds.getRight(), bounds.getBottom());
        fill.lineTo(bounds.getX(), bounds.getBottom());
        fill.closeSubPath();
        g.setColour(juce::Colour(palette::orange).withAlpha(0.15f));
        g.fillPath(fill);

        // Stroke
        g.setColour(juce::Colour(palette::orange).withAlpha(0.7f));
        g.strokePath(path, juce::PathStrokeType(1.2f));

        // Border
        g.setColour(juce::Colour(palette::border));
        g.drawRoundedRectangle(bounds.reduced(0.5f), radius::sm, 0.5f);
    }

private:
    float buf[kSize] = {};
    int pos = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// SpectrumDisplay — before/after FFT overlay
// ═══════════════════════════════════════════════════════════════════════════

class SpectrumDisplay : public juce::Component {
public:
    static constexpr int kBins = 128;

    void setData(const float* before, const float* after, int count) {
        const int n = juce::jmin(count, kBins);
        for (int i = 0; i < n; ++i) {
            beforeBins[i] = before[i];
            afterBins[i]  = after[i];
        }
        repaint();
    }

    void paint(juce::Graphics& g) override {
        const auto bounds = getLocalBounds().toFloat();

        // Background
        g.setColour(juce::Colour(palette::surface));
        g.fillRoundedRectangle(bounds, radius::md);

        const float stepX = bounds.getWidth() / static_cast<float>(kBins);

        // Before (faint ink)
        {
            juce::Path p;
            p.startNewSubPath(bounds.getX(), bounds.getBottom());
            for (int i = 0; i < kBins; ++i) {
                const float h = beforeBins[i] * bounds.getHeight();
                p.lineTo(bounds.getX() + i * stepX, bounds.getBottom() - h);
            }
            p.lineTo(bounds.getRight(), bounds.getBottom());
            p.closeSubPath();
            g.setColour(juce::Colour(palette::ink).withAlpha(0.06f));
            g.fillPath(p);
        }

        // After (sage)
        {
            juce::Path p;
            p.startNewSubPath(bounds.getX(), bounds.getBottom());
            for (int i = 0; i < kBins; ++i) {
                const float h = afterBins[i] * bounds.getHeight();
                p.lineTo(bounds.getX() + i * stepX, bounds.getBottom() - h);
            }
            p.lineTo(bounds.getRight(), bounds.getBottom());
            p.closeSubPath();
            g.setColour(juce::Colour(palette::sage).withAlpha(0.2f));
            g.fillPath(p);
            g.setColour(juce::Colour(palette::sage));
            g.strokePath(p, juce::PathStrokeType(1.0f));
        }

        // Border
        g.setColour(juce::Colour(palette::border));
        g.drawRoundedRectangle(bounds.reduced(0.5f), radius::md, 0.5f);
    }

private:
    float beforeBins[kBins] = {};
    float afterBins[kBins]  = {};
};

// ═══════════════════════════════════════════════════════════════════════════
// ProcessingIndicator — small coloured dot
// ═══════════════════════════════════════════════════════════════════════════

class ProcessingIndicator : public juce::Component {
public:
    void setActive(bool active, juce::Colour c = juce::Colour(palette::sage)) {
        isActive = active;
        colour = c;
        repaint();
    }

    void paint(juce::Graphics& g) override {
        const auto b = getLocalBounds().toFloat().reduced(2.0f);
        const float sz = juce::jmin(b.getWidth(), b.getHeight());
        auto dot = juce::Rectangle<float>(sz, sz).withCentre(b.getCentre());

        if (isActive) {
            g.setColour(colour.withAlpha(0.2f));
            g.fillEllipse(dot.expanded(1.5f));
            g.setColour(colour);
        } else {
            g.setColour(juce::Colour(palette::well));
        }
        g.fillEllipse(dot);
    }

private:
    bool isActive = false;
    juce::Colour colour { palette::sage };
};

// ═══════════════════════════════════════════════════════════════════════════
// TabBar — minimal horizontal tab strip
// ═══════════════════════════════════════════════════════════════════════════

class TabBar : public juce::Component {
public:
    void setTabs(const juce::StringArray& names) { tabNames = names; repaint(); }
    void setActive(int idx) { activeIdx = idx; repaint(); }
    std::function<void(int)> onTabChanged;

    void paint(juce::Graphics& g) override {
        const auto bounds = getLocalBounds();
        const int tabW = 60;
        const int gap = space::xs;
        const int totalW = tabNames.size() * (tabW + gap) - gap;
        int x = (bounds.getWidth() - totalW) / 2;

        for (int i = 0; i < tabNames.size(); ++i) {
            auto tb = juce::Rectangle<int>(x, bounds.getY(), tabW, bounds.getHeight());

            if (i == activeIdx) {
                g.setColour(juce::Colour(palette::sage));
                g.fillRoundedRectangle(tb.toFloat(), radius::sm);
                g.setColour(juce::Colour(palette::canvas));
            } else {
                g.setColour(juce::Colour(palette::inkMuted));
            }

            g.setFont(juce::Font(type::sans(), type::label, juce::Font::bold));
            g.drawText(tabNames[i].toUpperCase(), tb, juce::Justification::centred);
            x += tabW + gap;
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const int tabW = 60;
        const int gap = space::xs;
        const int totalW = tabNames.size() * (tabW + gap) - gap;
        const int startX = (getWidth() - totalW) / 2;
        const int relX = e.getPosition().getX() - startX;
        if (relX >= 0) {
            const int idx = relX / (tabW + gap);
            if (idx >= 0 && idx < tabNames.size()) {
                activeIdx = idx;
                if (onTabChanged) onTabChanged(idx);
                repaint();
            }
        }
    }

private:
    juce::StringArray tabNames;
    int activeIdx = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// LabeledKnob — knob + label + value (uses btz::paintKnob)
// ═══════════════════════════════════════════════════════════════════════════

class LabeledKnob : public juce::Component {
public:
    LabeledKnob(const juce::String& name, int compID)
        : labelText(name), componentID(compID) {
        setRepaintsOnMouseActivity(true);
    }

    void setValue(float norm, const juce::String& display) {
        normValue = norm;
        valueText = display;
        repaint();
    }

    void setHovered(bool h) { hovered = h; repaint(); }

    // Callback: called when user drags to change value
    std::function<void(float)> onValueChange;

    void paint(juce::Graphics& g) override {
        const auto bounds = getLocalBounds();
        const int labelH = 13;
        const int valueH = 11;
        const int knobSize = bounds.getHeight() - labelH - valueH - space::xs * 2;

        // Knob
        auto knobBounds = juce::Rectangle<float>(
            bounds.getCentreX() - knobSize * 0.5f,
            static_cast<float>(bounds.getY()),
            static_cast<float>(knobSize),
            static_cast<float>(knobSize));
        paintKnob(g, knobBounds, normValue, componentID, hovered || isMouseOver());

        // Label
        auto bottomArea = bounds.withTop(bounds.getY() + knobSize + space::xs);
        g.setColour(juce::Colour(palette::inkFaint));
        g.setFont(juce::Font(type::sans(), type::label, juce::Font::plain));
        g.drawText(labelText.toUpperCase(), bottomArea.removeFromTop(labelH),
                   juce::Justification::centred);

        // Value
        g.setColour(juce::Colour(palette::ink));
        g.setFont(juce::Font(type::mono(), type::value, juce::Font::plain));
        g.drawText(valueText, bottomArea.removeFromTop(valueH),
                   juce::Justification::centred);
    }

    // ── Mouse interaction (rotary drag) ──────────────────────────────────
    void mouseDown(const juce::MouseEvent& e) override {
        dragStartY = e.getPosition().getY();
        dragStartValue = normValue;
    }

    void mouseDrag(const juce::MouseEvent& e) override {
        const float sensitivity = 200.0f;  // pixels for full range
        const float delta = static_cast<float>(dragStartY - e.getPosition().getY()) / sensitivity;
        const float newVal = juce::jlimit(0.0f, 1.0f, dragStartValue + delta);
        if (std::abs(newVal - normValue) > 0.001f) {
            normValue = newVal;
            if (onValueChange) onValueChange(normValue);
            repaint();
        }
    }

    void mouseEnter(const juce::MouseEvent&) override { hovered = true; repaint(); }
    void mouseExit(const juce::MouseEvent&) override { hovered = false; repaint(); }

    void mouseDoubleClick(const juce::MouseEvent&) override {
        // Reset to center on double-click
        normValue = 0.5f;
        if (onValueChange) onValueChange(normValue);
        repaint();
    }

private:
    juce::String labelText;
    juce::String valueText;
    int componentID;
    float normValue = 0.0f;
    bool hovered = false;
    int dragStartY = 0;
    float dragStartValue = 0.0f;
};

// ═══════════════════════════════════════════════════════════════════════════
// PresetBrowser — minimal preset list
// ═══════════════════════════════════════════════════════════════════════════

class PresetBrowser : public juce::Component {
public:
    struct Entry { juce::String name; juce::String category; };

    void setPresets(const std::vector<Entry>& list) { presets = list; repaint(); }
    void setSelected(int idx) { selectedIdx = idx; repaint(); }
    std::function<void(int)> onSelect;

    void paint(juce::Graphics& g) override {
        const auto bounds = getLocalBounds();
        g.setColour(juce::Colour(palette::surface));
        g.fillRoundedRectangle(bounds.toFloat(), radius::md);

        const int rowH = 22;
        auto area = bounds.reduced(space::sm);

        for (int i = 0; i < static_cast<int>(presets.size()); ++i) {
            auto row = area.removeFromTop(rowH);
            if (row.getY() > bounds.getBottom()) break;

            if (i == selectedIdx) {
                g.setColour(juce::Colour(palette::sageFaint));
                g.fillRoundedRectangle(row.toFloat(), radius::sm);
                g.setColour(juce::Colour(palette::sagePressed));
            } else {
                g.setColour(juce::Colour(palette::ink));
            }

            g.setFont(juce::Font(type::sans(), type::body, juce::Font::plain));
            g.drawText(presets[i].name, row.reduced(space::sm, 0),
                       juce::Justification::centredLeft);

            g.setColour(juce::Colour(palette::inkFaint));
            g.setFont(juce::Font(type::sans(), type::micro, juce::Font::plain));
            g.drawText(presets[i].category, row.reduced(space::sm, 0),
                       juce::Justification::centredRight);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const int rowH = 22;
        const int idx = (e.getPosition().getY() - space::sm) / rowH;
        if (idx >= 0 && idx < static_cast<int>(presets.size())) {
            selectedIdx = idx;
            if (onSelect) onSelect(idx);
            repaint();
        }
    }

private:
    std::vector<Entry> presets;
    int selectedIdx = -1;
};

// ═══════════════════════════════════════════════════════════════════════════
// DirectManipSpectrum — interactive spectrum with draggable crossover points
// ═══════════════════════════════════════════════════════════════════════════

class DirectManipSpectrum : public juce::Component {
public:
    struct Band {
        float freqHz = 1000.0f;
        juce::Colour colour { palette::sage };
    };

    void setSpectrum(const float* mags, int numBins, float sampleRate) {
        specData.assign(mags, mags + numBins);
        sr = sampleRate;
        repaint();
    }

    void setBands(const std::vector<Band>& b) { bands = b; repaint(); }
    std::function<void(int, float)> onCrossoverDrag;

    void paint(juce::Graphics& g) override {
        const auto bounds = getLocalBounds().toFloat();

        // Background
        g.setColour(juce::Colour(palette::surface));
        g.fillRoundedRectangle(bounds, radius::md);

        // Spectrum fill
        if (!specData.empty()) {
            juce::Path p;
            p.startNewSubPath(bounds.getX(), bounds.getBottom());
            const float w = bounds.getWidth();
            const float h = bounds.getHeight();

            for (int i = 0; i < static_cast<int>(specData.size()); ++i) {
                const float freq = static_cast<float>(i) * sr / static_cast<float>(specData.size() * 2);
                if (freq < 20.0f || freq > 20000.0f) continue;
                const float x = bounds.getX() + freqToX(freq, w);
                const float db = 20.0f * std::log10(specData[i] + 1e-10f);
                const float y = bounds.getBottom() - ((db + 80.0f) / 80.0f) * h;
                p.lineTo(x, juce::jlimit(bounds.getY(), bounds.getBottom(), y));
            }
            p.lineTo(bounds.getRight(), bounds.getBottom());
            p.closeSubPath();

            g.setColour(juce::Colour(palette::sage).withAlpha(0.12f));
            g.fillPath(p);
            g.setColour(juce::Colour(palette::sage).withAlpha(0.6f));
            g.strokePath(p, juce::PathStrokeType(1.0f));
        }

        // Crossover lines
        for (int i = 0; i < static_cast<int>(bands.size()); ++i) {
            const float x = bounds.getX() + freqToX(bands[i].freqHz, bounds.getWidth());
            g.setColour(bands[i].colour.withAlpha(0.5f));
            g.drawVerticalLine(static_cast<int>(x), bounds.getY(), bounds.getBottom());
            g.setColour(bands[i].colour);
            g.fillEllipse(x - 3.0f, bounds.getCentreY() - 3.0f, 6.0f, 6.0f);
        }

        // Border
        g.setColour(juce::Colour(palette::border));
        g.drawRoundedRectangle(bounds.reduced(0.5f), radius::md, 0.5f);
    }

    void mouseDown(const juce::MouseEvent& e) override { dragIdx = findNearest(e.position.x); }
    void mouseDrag(const juce::MouseEvent& e) override {
        if (dragIdx >= 0 && onCrossoverDrag)
            onCrossoverDrag(dragIdx, xToFreq(e.position.x, static_cast<float>(getWidth())));
    }
    void mouseUp(const juce::MouseEvent&) override { dragIdx = -1; }

private:
    std::vector<float> specData;
    std::vector<Band> bands;
    float sr = 44100.0f;
    int dragIdx = -1;

    float freqToX(float f, float w) const {
        return std::log2(f / 20.0f) / std::log2(20000.0f / 20.0f) * w;
    }
    float xToFreq(float x, float w) const {
        return 20.0f * std::pow(2.0f, (x / w) * std::log2(20000.0f / 20.0f));
    }
    int findNearest(float mx) const {
        int best = -1; float minD = 16.0f;
        for (int i = 0; i < static_cast<int>(bands.size()); ++i) {
            float d = std::abs(mx - freqToX(bands[i].freqHz, static_cast<float>(getWidth())));
            if (d < minD) { minD = d; best = i; }
        }
        return best;
    }
};

} // namespace btz
