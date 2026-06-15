/*
  Box Tone Zone (BTZ) — BTZComponents.h  v1.0 "Ivory System"
  ──────────────────────────────────────────────────────────────────────────
  All reusable UI components. Every colour and dimension comes from BTZTheme.h.
  Zero hardcoded hex values. Zero magic numbers.
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
// LabeledKnob — Interactive ceramic-style knob with label + value display
// ═══════════════════════════════════════════════════════════════════════════

class LabeledKnob : public juce::Component, public juce::SettableTooltipClient {
public:
    LabeledKnob(const juce::String& name, int compID,
                const juce::String& tooltipText = {})
        : labelText(name), componentID(compID), tip(tooltipText)
    {
        setRepaintsOnMouseActivity(true);
        setTooltip(tooltipText);
    }

    void setValue(float norm, const juce::String& display = {}) {
        normValue = norm;
        valueText = display.isEmpty() ? juce::String(norm, 2) : display;
        repaint();
    }

    float getValue() const { return normValue; }
    void setHovered(bool h) { hovered = h; repaint(); }

    // Callback: called when user drags to change value
    std::function<void(float)> onValueChange;

    void paint(juce::Graphics& g) override {
        const auto bounds = getLocalBounds();
        const int labelH = 14;
        const int valueH = 12;
        const int knobSize = bounds.getHeight() - labelH - valueH - space::xs * 2;

        // Knob (ceramic style via paintKnob)
        auto knobBounds = juce::Rectangle<float>(
            bounds.getCentreX() - knobSize * 0.5f,
            static_cast<float>(bounds.getY()),
            static_cast<float>(knobSize),
            static_cast<float>(knobSize));
        paintKnob(g, knobBounds, normValue, componentID, hovered || isMouseOver());

        // Label (uppercase, tracked, muted)
        auto bottomArea = bounds.withTop(bounds.getY() + knobSize + space::xs);
        g.setColour(juce::Colour(palette::inkMuted));
        g.setFont(juce::Font(type::sans(), type::label, juce::Font::plain));
        g.drawText(labelText.toUpperCase(), bottomArea.removeFromTop(labelH),
                   juce::Justification::centred);

        // Value (monospace, precise)
        g.setColour(juce::Colour(palette::ink));
        g.setFont(juce::Font(type::mono(), type::value, juce::Font::plain));
        g.drawText(valueText, bottomArea.removeFromTop(valueH),
                   juce::Justification::centred);
    }

    // ── Mouse interaction (rotary drag) ──────────────────
    void mouseDown(const juce::MouseEvent& e) override {
        dragStartY = e.getPosition().getY();
        dragStartValue = normValue;
    }

    void mouseDrag(const juce::MouseEvent& e) override {
        const float sensitivity = 200.0f;
        const float delta = static_cast<float>(dragStartY - e.getPosition().getY()) / sensitivity;
        const float newVal = juce::jlimit(0.0f, 1.0f, dragStartValue + delta);
        if (std::abs(newVal - normValue) > 0.001f) {
            normValue = newVal;
            valueText = juce::String(normValue, 2);
            if (onValueChange) onValueChange(normValue);
            repaint();
        }
    }

    void mouseEnter(const juce::MouseEvent&) override { hovered = true; repaint(); }
    void mouseExit(const juce::MouseEvent&) override { hovered = false; repaint(); }

    void mouseDoubleClick(const juce::MouseEvent&) override {
        normValue = 0.5f;
        valueText = "0.50";
        if (onValueChange) onValueChange(normValue);
        repaint();
    }

private:
    juce::String labelText;
    juce::String valueText { "0.00" };
    juce::String tip;
    int componentID;
    float normValue = 0.0f;
    bool hovered = false;
    int dragStartY = 0;
    float dragStartValue = 0.0f;
};

// ═══════════════════════════════════════════════════════════════════════════
// HarmonicVisualizer — Signature BTZ visual element (3 styles)
// ═══════════════════════════════════════════════════════════════════════════

class HarmonicVisualizer : public juce::Component, private juce::Timer {
public:
    enum class Style { Bloom, Bars, Spectrum };
    static constexpr int kMaxBars = 16;
    static constexpr int kMaxBins = 256;

    HarmonicVisualizer(Style s = Style::Bloom) : style(s) {
        setOpaque(false);
        startTimerHz(anim::fps);
        current.fill(0.0f);
        target.fill(0.0f);
    }

    void setStyle(Style s) { style = s; repaint(); }

    void setMagnitudes(const float* mags, int count) {
        const int n = juce::jmin(count, kMaxBars);
        for (int i = 0; i < n; ++i)
            target[static_cast<size_t>(i)] = juce::jlimit(0.0f, 1.0f, mags[i]);
        for (int i = n; i < kMaxBars; ++i)
            target[static_cast<size_t>(i)] = 0.0f;
    }

    void setSpectrumData(const float* before, const float* after, int numBins) {
        const int n = juce::jmin(numBins, kMaxBins);
        for (int i = 0; i < n; ++i) {
            specBefore[i] = before ? before[i] : 0.0f;
            specAfter[i]  = after  ? after[i]  : 0.0f;
        }
        activeBins = n;
    }

    void paint(juce::Graphics& g) override {
        const auto bounds = getLocalBounds().toFloat();

        // Background well
        paintWell(g, bounds);

        switch (style) {
            case Style::Bloom:    paintBloom(g, bounds);    break;
            case Style::Bars:     paintBars(g, bounds);     break;
            case Style::Spectrum: paintSpectrum(g, bounds); break;
        }

        // Subtle border
        g.setColour(juce::Colour(palette::border));
        g.drawRoundedRectangle(bounds.reduced(0.5f), radius::md, 0.5f);
    }

private:
    Style style;
    std::array<float, kMaxBars> target;
    std::array<float, kMaxBars> current;
    float specBefore[kMaxBins] = {};
    float specAfter[kMaxBins] = {};
    int activeBins = 0;

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

    void paintBloom(juce::Graphics& g, juce::Rectangle<float> b) {
        // Circular harmonic bloom (Simple Mode signature)
        const float cx = b.getCentreX();
        const float cy = b.getCentreY();
        const float maxR = juce::jmin(b.getWidth(), b.getHeight()) * 0.42f;

        for (int i = kMaxBars - 1; i >= 0; --i) {
            const float val = current[static_cast<size_t>(i)];
            if (val < 0.005f) continue;
            const float r = maxR * (0.25f + 0.75f * (float)(i + 1) / (float)kMaxBars);
            const float alpha = val * 0.5f;
            g.setColour(harmonicColour(i).withAlpha(alpha));
            g.fillEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f);
        }

        // Center warm glow
        g.setColour(juce::Colour(palette::orange).withAlpha(0.12f));
        g.fillEllipse(cx - maxR * 0.18f, cy - maxR * 0.18f, maxR * 0.36f, maxR * 0.36f);
    }

    void paintBars(juce::Graphics& g, juce::Rectangle<float> b) {
        // Horizontal harmonic bars (Standard Mode)
        const float barW = 5.0f;
        const float gap  = 3.0f;
        const float totalW = kMaxBars * barW + (kMaxBars - 1) * gap;
        const float padY = 10.0f;
        const float maxH = b.getHeight() - padY * 2.0f - 14.0f;
        float x = b.getCentreX() - totalW * 0.5f;

        for (int i = 0; i < kMaxBars; ++i) {
            const auto colour = harmonicColour(i);

            // Track (faint)
            g.setColour(colour.withAlpha(0.08f));
            g.fillRoundedRectangle(x, padY + b.getY(), barW, maxH, 2.0f);

            // Active bar
            const float h = current[static_cast<size_t>(i)] * maxH;
            if (h > 0.5f) {
                const float y = padY + b.getY() + maxH - h;
                g.setColour(colour);
                g.fillRoundedRectangle(x, y, barW, h, 2.0f);
            }

            // Index label
            g.setColour(juce::Colour(palette::inkFaint));
            g.setFont(juce::Font(type::mono(), type::micro, juce::Font::plain));
            g.drawText(juce::String(i + 1),
                       juce::Rectangle<float>(x - 1.0f, b.getBottom() - 14.0f, barW + 2.0f, 12.0f),
                       juce::Justification::centred, false);

            x += barW + gap;
        }
    }

    void paintSpectrum(juce::Graphics& g, juce::Rectangle<float> b) {
        // Before/after spectrum overlay (Advanced Mode)
        if (activeBins < 2) {
            paintBars(g, b);  // Fallback to bars if no spectrum data
            return;
        }

        const float w = b.getWidth() - 8.0f;
        const float h = b.getHeight() - 8.0f;
        const float ox = b.getX() + 4.0f;
        const float oy = b.getY() + 4.0f;
        const float binW = w / (float)activeBins;

        // Before (gray line)
        juce::Path beforePath;
        beforePath.startNewSubPath(ox, oy + h);
        for (int i = 0; i < activeBins; ++i) {
            const float x = ox + (float)i * binW;
            const float y = oy + h - specBefore[i] * h;
            beforePath.lineTo(x, y);
        }
        g.setColour(juce::Colour(palette::analyzerBefore).withAlpha(0.5f));
        g.strokePath(beforePath, juce::PathStrokeType(1.0f));

        // After (sage fill)
        juce::Path afterPath;
        afterPath.startNewSubPath(ox, oy + h);
        for (int i = 0; i < activeBins; ++i) {
            const float x = ox + (float)i * binW;
            const float y = oy + h - specAfter[i] * h;
            afterPath.lineTo(x, y);
        }
        afterPath.lineTo(ox + w, oy + h);
        afterPath.closeSubPath();
        g.setColour(juce::Colour(palette::analyzerAfter).withAlpha(0.2f));
        g.fillPath(afterPath);
        g.setColour(juce::Colour(palette::analyzerAfter));
        g.strokePath(afterPath, juce::PathStrokeType(1.5f));
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
        paintWell(g, bounds);

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
        g.setColour(juce::Colour(palette::orange).withAlpha(0.12f));
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
        paintWell(g, bounds);

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

// The editor's Standard/Advanced spectrum views are DirectManipSpectrum
// instances; keep the descriptive name the editor expects.
using SpectrumDisplay = DirectManipSpectrum;

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
// TabBar — pill-style mode selector (Simple/Standard/Advanced)
// ═══════════════════════════════════════════════════════════════════════════

class TabBar : public juce::Component {
public:
    void setTabs(const juce::StringArray& names) { tabNames = names; repaint(); }
    void setActive(int idx) { activeIdx = idx; repaint(); }
    int getActive() const { return activeIdx; }
    std::function<void(int)> onTabChanged;

    void paint(juce::Graphics& g) override {
        const auto bounds = getLocalBounds().toFloat().reduced(2.0f, 2.0f);
        const float tabW = bounds.getWidth() / (float)tabNames.size();

        for (int i = 0; i < tabNames.size(); ++i) {
            auto tabRect = bounds.withX(bounds.getX() + tabW * (float)i)
                                 .withWidth(tabW).reduced(1.0f, 0.0f);

            if (i == activeIdx) {
                // Active tab: orange pill
                g.setColour(juce::Colour(palette::orange));
                g.fillRoundedRectangle(tabRect, radius::pill);
                g.setColour(juce::Colour(palette::surface));
            } else {
                g.setColour(juce::Colour(palette::inkMuted));
            }

            g.setFont(juce::Font(type::sans(), type::label, juce::Font::bold));
            g.drawText(tabNames[i].toUpperCase(), tabRect, juce::Justification::centred);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        if (tabNames.isEmpty()) return;
        const float tabW = (float)getWidth() / (float)tabNames.size();
        const int idx = juce::jlimit(0, tabNames.size() - 1,
                                     (int)(e.position.x / tabW));
        if (idx != activeIdx) {
            activeIdx = idx;
            if (onTabChanged) onTabChanged(idx);
            repaint();
        }
    }

private:
    juce::StringArray tabNames;
    int activeIdx = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// PresetBrowser — minimal preset list with categories
// ═══════════════════════════════════════════════════════════════════════════

class PresetBrowser : public juce::Component {
public:
    struct Entry { juce::String name; juce::String category; };

    void setPresets(const std::vector<Entry>& list) { presets = list; repaint(); }
    void setSelected(int idx) { selectedIdx = idx; repaint(); }
    int getSelected() const { return selectedIdx; }
    std::function<void(int)> onSelect;

    void paint(juce::Graphics& g) override {
        const auto bounds = getLocalBounds();
        paintWell(g, bounds.toFloat());

        if (presets.empty()) {
            g.setColour(juce::Colour(palette::inkFaint));
            g.setFont(juce::Font(type::sans(), type::body, juce::Font::plain));
            g.drawText("No presets loaded", bounds, juce::Justification::centred);
            return;
        }

        const int rowH = 22;
        auto area = bounds.reduced(space::sm);

        int startIdx = juce::jmax(0, scrollOffset);
        int endIdx = juce::jmin((int)presets.size(), startIdx + area.getHeight() / rowH);

        for (int i = startIdx; i < endIdx; ++i) {
            auto row = area.removeFromTop(rowH);
            if (row.getY() > bounds.getBottom()) break;

            if (i == selectedIdx) {
                g.setColour(juce::Colour(palette::orange).withAlpha(0.10f));
                g.fillRoundedRectangle(row.toFloat(), radius::sm);
                g.setColour(juce::Colour(palette::orange));
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
        if (presets.empty()) return;
        const int rowH = 22;
        const int idx = scrollOffset + (e.getPosition().getY() - space::sm) / rowH;
        if (idx >= 0 && idx < static_cast<int>(presets.size())) {
            selectedIdx = idx;
            if (onSelect) onSelect(idx);
            repaint();
        }
    }

    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& w) override {
        scrollOffset = juce::jlimit(0, juce::jmax(0, (int)presets.size() - 10),
                                    scrollOffset - (int)(w.deltaY * 3.0f));
        repaint();
    }

private:
    std::vector<Entry> presets;
    int selectedIdx = -1;
    int scrollOffset = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// SafetyIndicator — subtle alert badges (true peak, clip, correlation)
// ═══════════════════════════════════════════════════════════════════════════

class SafetyIndicator : public juce::Component {
public:
    enum class State { Safe, Caution, Warning };

    // Ivory System editor refers to this as "Level"; keep both names in sync.
    using Level = State;

    void setState(State s, const juce::String& msg = {}) {
        state = s; message = msg; repaint();
    }

    // Compatibility wrapper for the editor's setLevel() call.
    void setLevel(Level s, const juce::String& msg = {}) { setState(s, msg); }

    void paint(juce::Graphics& g) override {
        if (state == State::Safe) return;

        auto b = getLocalBounds().toFloat().reduced(1.0f);
        juce::Colour c = (state == State::Warning)
            ? juce::Colour(palette::clay)
            : juce::Colour(palette::gold);

        g.setColour(c.withAlpha(0.12f));
        g.fillRoundedRectangle(b, radius::sm);
        g.setColour(c);
        g.drawRoundedRectangle(b, radius::sm, 0.5f);

        g.setFont(juce::Font(type::sans(), type::micro, juce::Font::plain));
        g.drawText(message, b, juce::Justification::centred);
    }

private:
    State state = State::Safe;
    juce::String message;
};

// ═══════════════════════════════════════════════════════════════════════════
// MeterBar — Vertical level meter with multi-colour segments
// ═══════════════════════════════════════════════════════════════════════════

class MeterBar : public juce::Component, private juce::Timer {
public:
    MeterBar(bool gainReduction = false) : isGR(gainReduction) { startTimerHz(anim::fps); }

    void setLevel(float newLevel) { targetLevel = juce::jlimit(0.0f, 1.0f, newLevel); }
    void setPeakHold(float peak) { peakHold = peak; }

    void paint(juce::Graphics& g) override {
        paintMeter(g, getLocalBounds(), displayLevel, isGR);

        // Peak hold indicator
        if (peakHold > 0.01f && !isGR) {
            const int peakY = getHeight() - static_cast<int>(getHeight() * peakHold);
            g.setColour(juce::Colour(palette::meterClip).withAlpha(0.8f));
            g.fillRect(0, peakY, getWidth(), 1);
        }
    }

private:
    void timerCallback() override {
        if (displayLevel < targetLevel)
            displayLevel = targetLevel;
        else
            displayLevel = juce::jmax(0.0f, displayLevel - anim::meterDecay);
        repaint();
    }

    float targetLevel = 0.0f;
    float displayLevel = 0.0f;
    float peakHold = 0.0f;
    bool isGR;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MeterBar)
};

// ═══════════════════════════════════════════════════════════════════════════
// CorrelationMeter — Horizontal stereo correlation indicator
// ═══════════════════════════════════════════════════════════════════════════

class CorrelationMeter : public juce::Component {
public:
    void setCorrelation(float c) { correlation = juce::jlimit(-1.0f, 1.0f, c); repaint(); }

    void paint(juce::Graphics& g) override {
        auto b = getLocalBounds().toFloat().reduced(2.0f, 0.0f);
        const float midX = b.getCentreX();
        const float h = b.getHeight();

        // Track
        g.setColour(juce::Colour(palette::meterTrack));
        g.fillRoundedRectangle(b, radius::sm);

        // Indicator
        const float pos = midX + (correlation * b.getWidth() * 0.5f);
        juce::Colour c = (correlation < 0.0f) ? juce::Colour(palette::clay)
                       : (correlation < 0.5f) ? juce::Colour(palette::meterActive)
                       : juce::Colour(palette::meterSafe);
        g.setColour(c);
        g.fillRoundedRectangle(juce::jmin(midX, pos), b.getY(),
                               std::abs(pos - midX), h, radius::sm);

        // Center line
        g.setColour(juce::Colour(palette::border));
        g.fillRect(midX - 0.5f, b.getY(), 1.0f, h);
    }

private:
    float correlation = 1.0f;
};

} // namespace btz
