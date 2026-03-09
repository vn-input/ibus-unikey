# UI macro tests

This directory contains focused tests and fixtures for macro setup UI behavior.

## What is covered

Current automated coverage focuses on remembered import/export locations stored
through GSettings:

- when no saved location exists, both choosers default to `$HOME/Documents`
- import remembers the parent directory of the selected file
- export remembers the parent directory of the selected file
- import/export state is stored independently
- the latest selected directory overwrites the previous saved value
- missing or invalid saved directories fall back to `$HOME/Documents`

## Files

- `macro_dialog_state_test.cpp` — unit tests for remembered chooser folders
- `unikey_macro.yaml` — YAML fixture for macro import/export workflows

## Build

From the repository root:

```bash
cmake -S . -B build
cmake --build build
```

## Run tests

```bash
ctest --test-dir build --output-on-failure
```

To run only this suite:

```bash
ctest --test-dir build -R macro-dialog-state-test --output-on-failure
```

## Notes

- The tests use `GSETTINGS_BACKEND=memory`, so they do not modify the user's
  real desktop settings.
- The schema is loaded from the build tree (`build/gsettings`).
- GTK chooser bookmark UI is left to GTK itself; these tests focus on the
  application-owned remembered last directories.
