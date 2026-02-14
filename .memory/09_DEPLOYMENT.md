# Deployment

## Current Deployment Mode
- Local/manual deployment for development testing.

## Windows Deployment
1. Build VST3 bundle.
2. Copy `BTZ.vst3` bundle to:
   - `C:\Program Files\Common Files\VST3\`
3. Rescan plugins in FL Studio.

## Artifact
- Expected bundle structure:
  - `BTZ.vst3/Contents/Resources/moduleinfo.json`
  - `BTZ.vst3/Contents/x86_64-win/BTZ.vst3`

## Verification Checklist
- Build success without linker/toolchain errors.
- Bundle copied to VST3 destination.
- Plugin appears and loads in host.
- Audio passes through without crash.

## TODO
- [TODO] Add release packaging/version stamping.
- [TODO] Add CI artifact export and signed build strategy.
