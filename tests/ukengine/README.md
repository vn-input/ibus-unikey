# ukengine Tests

This directory contains regression tests for the core Vietnamese input engine
(`ukengine` library).

## Purpose

These tests protect the engine behavior during refactoring and decomposition
work. They were added as part of the legacy cleanup mitigation plan (Phase 1)
to establish a safety net before structural changes to `ukengine.cpp` and
related files.

## Test Groups

### inputproc_classification_test

Tests character classification behavior in `UkcMap`:
- Vietnamese vowels and consonants → `ukcVn`
- Non-Vietnamese letters (J, F, W) → `ukcNonVn`
- Digits → `ukcNonVn`
- Control characters → `ukcReset`

### inputproc_wordbreak_test

Tests word-break detection:
- Punctuation, spaces, quotes → `ukcWordBreak`
- Brackets, math operators → `ukcWordBreak`
- Composition chars (~, ^, `) are NOT word-break (preserved for Vietnamese input)
- Consistency between `WordBreakSyms` array and `UkcMap`

## Building and Running

Build from the project root:

```bash
cd build
cmake ..
make
```

Run all ukengine tests:

```bash
ctest -R inputproc --output-on-failure
```

Or run individually:

```bash
./tests/ukengine/test-inputproc-classification
./tests/ukengine/test-inputproc-wordbreak
```

## Adding New Tests

1. Create a new `*_test.cpp` file in this directory
2. Add the executable and test registration in `CMakeLists.txt`
3. Follow the GLib test framework pattern used in existing tests
4. Ensure tests can run independently (initialize tables as needed)

## Related Documentation

- `project_planning/UI/macros/20260315_100647_legacy_cleanup_mitigation_plan.md`
- Phase 1 of the cleanup plan describes the test scaffolding goals
