# UI System

## Plugin UI
- Implemented with JUCE editor components.
- Main files:
  - `BTZ/Source/PluginEditor.h`
  - `BTZ/Source/PluginEditor.cpp`

## Non-Plugin UI Assets
- Repo contains web/UI code (`src/` etc.) not currently wired into plugin runtime by default.
- Treat web UI and JUCE editor as separate systems unless explicit integration is planned.

## Design Constraints
- Maintain host compatibility and plugin stability over visual complexity.
- Avoid introducing UI dependencies that alter audio thread safety.

## TODO
- [TODO] Document editor component hierarchy.
- [TODO] Define style/UX consistency rules for plugin UI updates.
