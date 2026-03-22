# Macro subsystem — Code structure (quick reference)

Purpose
-------
A one-page, maintainer-focused map of the macro subsystem: where the data lives,
where the UI projects it, where file I/O is handled, and what duplicate-resolution
policies apply. Use this when changing merge semantics, undo/redo, or drag/drop.

Components & responsibilities
-----------------------------
- Source of truth (durable storage):
  - `CMacroTable` — `ukengine/mactab.h` / `ukengine/mactab.cpp`
  - Key methods: `loadFromFile()`, `writeToFile()`, `removeDuplicates()`
  - Behaviour: sorts entries and removes duplicates; canonical runtime semantics
    are enforced here (see Duplicate matrix below).

- UI projection & helpers (editable list):
  - `GtkListStore` model used by the macro tree view
  - Helpers: `setup/macro_utils.h` / `setup/macro_utils.cpp`
    - `unikey_macro_to_store()`, `unikey_store_to_macro()`
    - `list_store_append_fast()`, `list_store_append()`, `list_store_add_null_item()`
  - Behaviour: holds the editable view and sentinel null row used for inline adds.

- File I/O / format tooling:
  - `setup/macro_file_io.h` / `setup/macro_file_io.cpp`
    - `macro_table_load_any_format()` — parse file into `CMacroTable`
    - `macro_table_write_any_format()` — serialize `CMacroTable` into supported formats
  - Tools for testing: `tools/macro_probe.cpp`, `tools/stress_macro_test.sh`

- Dialog & UX wiring:
  - `setup/signal_handlers.h` / `setup/signal_handlers.cpp`
    - Entry point: `on_btn_macroedit_clicked()` handles load→edit→save
    - Import/export callbacks, and other UI handlers live here

Duplicate-resolution matrix (who wins?)
---------------------------------------
- Engine / persistent files (`CMacroTable` / `ukengine/mactab`):
  - Policy: "dictionary" semantics — after full load the table is sorted and
    duplicates removed; **the last occurrence wins** (this is enforced in
    `removeDuplicates()` / `loadFromFile()`). This is the canonical runtime rule.

- Setup UI append/import (`list_store_append()` in `setup/macro_utils.cpp`):
  - Policy: KeepExisting — when appending into an already-populated `GtkListStore`,
    existing keys are preserved (incoming keys are skipped using a case-insensitive
    unordered set).

- Planned/centralized behaviour (design):
  - The design note proposes `list_store_merge()` + `MergeMode` to explicitly
    select semantics per operation: `KeepExisting`, `OverwriteExisting`, `ReplaceAll`,
    `ReplaceSelected`. Use the design note for implementation guidance:
    `project_planning/UI/macros/20260308_203348_drag_drop_merge_design.md`.

Why the split matters
---------------------
- Runtime engine relies on sorted, deduped `CMacroTable` for fast binary-search
  lookup and deterministic expansion.
- The UI-level append-skipping behaviour preserves the user's current edits when
  importing incrementally; it intentionally differs from engine semantics.
- When adding new ingest paths (drag-in, paste, bulk import), prefer parsing
  into a `CMacroTable` and then merging via a centralized helper so semantics are
  consistent and auditable.

PR checklist when changing merge semantics
-----------------------------------------
When updating duplicate/merge semantics, update these locations and tests:
- `ukengine/mactab.{h,cpp}` — document runtime semantics and tests for `loadFromFile()`
- `setup/macro_utils.{h,cpp}` — implement/adjust `list_store_merge()` and document the chosen default merge behaviour
- `setup/macro_file_io.{h,cpp}` — ensure format reads/writes still round-trip through `CMacroTable`
- Add/extend unit/integration tests using `tools/macro_probe.cpp` or `tools/stress_macro_test.sh` for large-file cases
- Update `project_planning/UI/macros/*` design docs and maintainer notes (this directory)

Quick grep commands
-------------------
Use these locally to find entry points:

```bash
git grep -n 'CMacroTable\|loadFromFile\|removeDuplicates\|list_store_append' || true
git grep -n 'macro_table_load_any_format\|macro_table_write_any_format' || true
```

Links
-----
- Design note: `project_planning/UI/macros/20260308_203348_drag_drop_merge_design.md`
- Maintainer note: `project_planning/UI/macros/20260301_170054_macros_improvements_and_code_comments.md`

---

If you want, I can:
- add this file to the repository now (done), and
- update `ukengine/README` and `setup/macro_utils.h` with two short cross-references; say "yes" and I'll apply small edits.
