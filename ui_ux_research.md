# BTZ UI/UX Research Findings — May 2026

## Current BTZ UI State (from BTZTheme.h + PluginEditor)
- Dark theme with accent colors (cyan, magenta, gold, green)
- Custom LookAndFeel v3 with hand-painted rotary knobs
- 3-page tabbed layout (Main, Spark, Advanced)
- Fixed 900x600 window (resizable declared but basic)
- Gradient backgrounds, rounded rectangles
- Meter strips, spectrum display, GR history
- Preset browser bar at top
- A/B, Undo/Redo buttons

## Industry Leaders — What Makes Them Visually Iconic

### FabFilter (Pro-Q, Saturn 2, Pro-L 2)
- **Direct manipulation**: Click/drag on spectrum to create EQ nodes
- **Smooth 60fps animations**: All meters, spectrums, curves animate fluidly
- **Minimal chrome**: Almost no borders, panels float over spectrum
- **Color-coded nodes**: Each band gets a distinct, muted color
- **Contextual controls**: Parameters appear only when relevant
- **Typography**: Clean sans-serif, high contrast, small but readable
- **Resizable**: Full resolution-independent rendering
- **Dark background with vibrant accents**: Deep charcoal with bright spectrum colors

### Goodhertz (Vulf Compressor, Tiltshift, Lossy)
- **Retro-modern hybrid**: Vintage typography (serif/display fonts) + clean modern layout
- **Distinctive brand identity**: Red/maroon + cream color scheme is instantly recognizable
- **Minimal controls on front page**: Only essential parameters visible
- **Advanced page hidden behind click**: Reduces cognitive load
- **Cultural identity**: The "Vulf" brand is inseparable from the sound
- **Light AND dark mode**: User choice
- **Internationalization**: 10 languages supported
- **Consistent across all products**: Same layout DNA

### Output (Portal)
- **Cinematic dark aesthetic**: Near-black background with glowing cyan/blue accents
- **Central visualization**: Large circular granular display is the focal point
- **Particle animations**: Real-time grain visualization
- **Minimal text**: Icons and spatial layout communicate function
- **Premium feel**: The dark + glow aesthetic signals "high-end"

### Spectrasonics (Omnisphere)
- **Deep blue/purple palette**: Cosmic, immersive atmosphere
- **Layered depth**: Multiple panels with transparency create Z-depth
- **Rich textures**: Subtle noise/grain in backgrounds
- **Orb/sphere motifs**: Central animated orb is iconic
- **Information density done right**: Lots of data, but well-organized

### Arturia (Analog Lab, V Collection)
- **3D rendered hardware**: Photorealistic knobs, switches, panels
- **Warm lighting**: Simulated studio lighting on surfaces
- **Material textures**: Brushed metal, wood, leather
- **Skeuomorphic but modern**: Hardware accuracy with modern UX additions

## Key Design Principles for "Greatest Plugin" Status

### 1. Glassmorphism (Frosted Glass)
- Semi-transparent panels with backdrop blur
- Creates depth hierarchy without heavy borders
- Used successfully by Sample Logic, Ujam, Apple
- CAUTION: Must maintain contrast for accessibility
- Best for: Panel overlays, modal dialogs, tooltip backgrounds

### 2. Direct Manipulation
- FabFilter's #1 innovation: click directly on the visualization to control
- For BTZ: Click on spectrum to adjust saturation per-band
- For BTZ: Drag on GR meter to set threshold
- Reduces cognitive distance between visual feedback and control

### 3. Micro-interactions & Animation
- Smooth easing on all value changes (not instant snaps)
- Knob hover glow / highlight
- Parameter value tooltip that follows cursor
- Subtle pulse on active processing indicators
- Gain reduction "breathing" animation

### 4. Typography Hierarchy
- Recommended: Inter, SF Pro, or custom geometric sans-serif
- 3 sizes max: Header (14-16pt), Label (10-12pt), Value (9-10pt)
- High contrast: White/light text on dark, or vice versa
- Monospace for numerical values (alignment)

### 5. Color System
- Primary dark background: #1A1A2E or #0F0F1A (not pure black)
- Accent gradient: Cyan → Magenta (modern) or Gold → Amber (warm)
- Semantic colors: Green=safe, Yellow=caution, Red=clip
- Muted pastels for non-active states
- High saturation only for active/selected elements

### 6. Resizable + HiDPI
- Resolution-independent rendering (vector or high-res raster)
- Smooth scaling from 50% to 200%
- Constraint-based layout (not pixel-positioned)
- Retina/4K support with proper DPI detection

### 7. Accessibility
- Color-blind friendly: Don't rely solely on red/green distinction
- Minimum 4.5:1 contrast ratio for text
- Keyboard navigation for all controls
- Screen reader labels (JUCE AccessibilityHandler)
- Tooltip on every control with parameter name + value + range

### 8. Contextual Complexity
- Simple Mode: 3 knobs, one visualization
- Standard Mode: Full controls, organized by section
- Advanced Mode: All parameters, modulation routing visible
- Progressive disclosure: complexity reveals on demand

## Specific Recommendations for BTZ

### Immediate Impact (Visual Identity)
1. **Signature visualization**: A central "tone zone" display that shows the harmonic content being shaped — like a glowing waveform that morphs as you add saturation
2. **Brand color**: Pick ONE signature color that IS BTZ. Suggestion: Deep amber/gold (#F5A623) — warm, analog, premium, unique in the plugin space
3. **Custom typeface**: Use a distinctive display font for "BTZ" branding, clean sans for controls
4. **Dark gradient background**: Not flat black — subtle radial gradient from center (slightly lighter) to edges (darker)

### Workflow Innovations
1. **Hover-to-preview**: Hovering over a saturation model temporarily auditions it
2. **Double-click to reset**: Any parameter returns to default
3. **Right-click context menus**: MIDI learn, copy value, set to exact number
4. **Drag between A/B**: Drag a parameter from A to B to copy just that value
5. **Undo history panel**: Visual timeline of changes (like Photoshop)

### Visual Feedback
1. **Harmonic overtone display**: Show the harmonic series being generated in real-time
2. **Before/after waveform**: Split-screen showing input vs output waveform
3. **Heat map**: Show where in the frequency spectrum saturation is most active
4. **Gain reduction "ribbon"**: Flowing ribbon showing compression over time
5. **Phase correlation scope**: Small Lissajous figure for stereo health

### Animation & Polish
1. **Knob rotation with inertia**: Slight overshoot on fast movements
2. **Value change ripple**: Subtle ring animation when a value changes
3. **Processing indicator**: Gentle pulse/glow on active modules
4. **Smooth meter ballistics**: 60fps with proper peak hold and decay
5. **Startup animation**: Brief logo reveal (< 500ms) when plugin opens
