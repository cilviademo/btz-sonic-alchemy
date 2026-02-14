# Roadmap

## Near-Term (Now -> Next)
- Confirm BTZ loads and processes audio in FL Studio reliably.
- Validate parameter automation and basic stability under real project load.
- Clean remaining compile warnings.

## Mid-Term
- Add regression build checks for Windows script flow.
- Improve installer feedback (verify destination bundle integrity post-copy).
- Document parameter behavior and signal-flow expectations.

## Long-Term
- Add optional CI pipeline for Windows build artifact validation.
- Support more portable JUCE dependency strategy (`JUCE_DIR`/submodule/FetchContent).
- Establish release versioning discipline in `.memory/versions/`.

## Risks
- Environment-specific hardcoded paths.
- Toolchain drift between VS versions.
- Context drift if memory files are not updated consistently.

## TODO
- [TODO] Add dated milestones and owners.
