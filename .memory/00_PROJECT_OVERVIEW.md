# BTZ Project Overview

## Purpose
- Build and ship a JUCE-based audio plugin (`BTZ`) with reliable local build/install workflows and maintainable project memory across chat sessions.
- Keep implementation continuity across sessions by storing architecture, decisions, and session history in `.memory/`.

## Target User
- Primary: producer/engineer using FL Studio and VST3 plugins on Windows.
- Secondary: developer iterating DSP/plugin code in Cursor with CMake + JUCE.

## Core Functionality
- Build a VST3 plugin from `btz-sonic-alchemy-main/BTZ`.
- Install plugin bundle to `C:\Program Files\Common Files\VST3\BTZ.vst3`.
- Rescan and load in FL Studio.
- Preserve long-term AI context via structured memory files.

## Constraints
- Windows-first workflow (MSVC + CMake + Ninja).
- JUCE source expected at `C:/Users/marcm/OneDrive/Desktop/JUCE-8.0.6`.
- Project currently uses nested repo layout (`btz-sonic-alchemy-main/btz-sonic-alchemy-main/BTZ`).
- No unapproved architecture-breaking changes.

## Non-Goals
- No full DSP redesign during setup/debug flow.
- No migration to different plugin frameworks.
- No deployment automation beyond local build/install scripts (yet).

## Current State Snapshot
- Build path stabilized with x64 VS 2022 toolchain scripts.
- VST3 bundle build + install flow now functioning.
- `.memory` system initialized for anti-drift continuity.

## TODO
- [TODO] Add official product positioning/branding language.
- [TODO] Add explicit release criteria (quality/performance thresholds).
