# Macro maintenance notes

Branch context: `feature/macro-unlimited-crud-sort`
Related design note: `project_planning/UI/macros/20260308_203348_drag_drop_merge_design.md`

## Intent

This note is for developers working on macro editing in the setup tool.
It replaces a report-style branch summary with a code-facing view of what changed,
where the main entry points are, and what assumptions now matter when extending
macro CRUD, sorting, import/export, or future drag-and-drop support.

## Scope of the landed work

The branch introduced a larger macro-editor upgrade centered on four themes:

1. **Macro storage became less constrained at the UI/workflow level**
   - The branch updated macro handling around `CMacroTable` and the setup UI so
     larger macro sets can be loaded, edited, sorted, imported, and exported.

2. **The setup dialog became the main CRUD surface**
   - The macro editor in `setup/ibus-unikey.ui` and
     `setup/signal_handlers.cpp` now carries more responsibility for loading,
     showing, sorting, counting, editing, importing, and exporting macros.

3. **Import/export became a first-class workflow**
   - `setup/macro_file_io.{h,cpp}` and related handlers added multi-format load
     and write helpers, plus review/debug tooling in `tools/`.
   - The import/export choosers now also remember their last-used directories
     via GSettings and expose those remembered folders back in the chooser
     sidebar as shortcuts.

4. **The codebase was heavily re-commented**
   - Many follow-up commits were documentation-focused. They matter because the
     macro path now crosses UI, conversion, file I/O, and `ukengine` storage,
     so maintainers need accurate comments to follow the data path safely.

## Code map: where to start

### 1. Dialog entry and save cycle

Primary entry point:
- `setup/signal_handlers.cpp` → `on_btn_macroedit_clicked()`

This is the main setup-time flow:
- resolve macro file path via `get_macro_file()`
- create/init `CMacroTable`
- load persisted macros with `CMacroTable::loadFromFile()`
- detach the tree model for bulk UI update
- populate the tree with `unikey_macro_to_store()`
- sort and refresh derived UI state
- on OK, convert tree rows back with `unikey_store_to_macro()`
- persist via `CMacroTable::writeToFile()`

Why this matters:
- If you change editor semantics, this is the first place to inspect because it
  defines the setup dialog's load-edit-save contract.
- Bulk model detachment is intentional and should be preserved for large tables.

### 2. Tree model helpers and conversion boundary

Primary files:
- `setup/macro_utils.h`
- `setup/macro_utils.cpp`

Key functions:
- `list_store_check_exists()`
- `list_store_add_null_item()`
- `list_store_append_fast()`
- `list_store_append()`
- `unikey_macro_to_store()`
- `unikey_store_to_macro()`

What this layer does:
- bridges `CMacroTable` and `GtkListStore`
- keeps case-insensitive key uniqueness in the UI
- converts text between internal Vietnamese encoding and UTF-8 for display
- maintains the sentinel row `STR_NULL_ITEM` used for inline row creation

Why this matters:
- Most future macro-editor behaviour changes should be expressed here first,
  not duplicated across multiple UI handlers.
- The sentinel row is part of the current editing contract. Any change to row
  deletion, sorting, import merge behaviour, or multi-selection must account
  for it explicitly.

### 3. Import/export path

Primary files:
- `setup/macro_dialog_state.h`
- `setup/macro_dialog_state.cpp`
- `setup/macro_file_io.h`
- `setup/macro_file_io.cpp`
- `setup/signal_handlers.cpp`

Key functions:
- `macro_table_load_any_format()`
- `macro_table_write_any_format()`

Current operational role:
- import handlers load external macro definitions into `CMacroTable`
- export handlers serialize the current macro table to supported formats
- chooser-state helpers restore the last import/export folder from GSettings
- chooser-state helpers add remembered folders as GTK shortcut entries when valid
- errors are surfaced through `GError**`, which is the right extension point
  for richer UX around drag/drop or format-specific validation later

Why this matters:
- New ingest paths such as drag-in, paste-from-clipboard, or batch merge should
  reuse this layer instead of implementing ad hoc parsers in signal handlers.
- New export paths should also stay format-agnostic by routing through the same
  write helper.

### 4. Core macro storage

Primary files:
- `ukengine/mactab.h`
- `ukengine/mactab.cpp`

Key symbol:
- `CMacroTable`

Current role:
- owns runtime macro entries
- loads persisted macros from disk
- writes canonical macro data back to disk
- provides the data that the setup UI mirrors into `GtkListStore`

Why this matters:
- The setup layer should keep using `CMacroTable` as the source of truth.
- Undo/redo, drag/drop merge, or clipboard workflows should snapshot or rebuild
  through `CMacroTable` rather than treating the tree model as the only store.

### 5. Tooling for validation and review

Primary files:
- `tools/macro_probe.cpp`
- `tools/stress_macro_test.sh`

Use these when:
- validating import/export format handling
- checking behaviour on larger macro files
- reproducing issues without launching the full setup dialog

## Key design decisions already embodied in the branch

### Keep UI sync logic centralized

The branch already moved important behaviour into `setup/macro_utils.cpp` and
kept `CMacroTable` as the durable model. That split is worth preserving:
- `CMacroTable` owns persistent macro state
- `GtkListStore` remains the editable UI projection
- `macro_utils` is the synchronization boundary

### Preserve bulk-load performance

`on_btn_macroedit_clicked()` detaches and reattaches the tree model during bulk
population. That should remain the default pattern for any operation that
rebuilds the full macro table.

### Preserve case-insensitive uniqueness

`list_store_check_exists()` and related append logic assume keys are unique
without case sensitivity. New merge behaviour must respect the same rule or the
UI and persisted storage will diverge semantically.

### Treat import/export as reusable infrastructure

`macro_table_load_any_format()` and `macro_table_write_any_format()` should be
considered shared infrastructure, not one-off button handlers. Future drop,
paste, and scripted workflows should reuse them.

### Treat the sentinel row as part of editor behaviour

The placeholder row using `STR_NULL_ITEM` is not decorative. It is part of the
current add-row UX. Any feature that clears, replaces, reorders, or filters rows
must restore this invariant.

## What changed in practice

From a maintenance perspective, the branch changed these developer-facing areas:

- `setup/ibus-unikey.ui`
  - macro editor UI layout and interaction surface changed

- `setup/macro_dialog_state.{h,cpp}`
  - chooser remembered-location behavior is centralized instead of being folded
    directly into the button handlers

- `setup/signal_handlers.{h,cpp}`
  - macro dialog open/save flow expanded
  - editing, sorting, counting, import, and export callbacks became more central

- `setup/macro_utils.{h,cpp}`
  - list-store and `CMacroTable` conversion logic became a clear extension point

- `setup/macro_file_io.{h,cpp}`
  - file-format handling became explicit and reusable

- `ukengine/mactab.{h,cpp}`
  - storage-layer behaviour matters more because more setup workflows now round-trip through it

- `tools/macro_probe.cpp`, `tools/stress_macro_test.sh`
  - macro workflows gained dedicated test/debug entry points

## Development notes for future changes

### If you change editor behaviour

Start in this order:
1. `setup/signal_handlers.cpp`
2. `setup/macro_utils.cpp`
3. `ukengine/mactab.cpp`
4. `setup/macro_file_io.cpp`

Reason:
- UI semantics usually start in handlers
- data-shape and merge semantics belong in `macro_utils`
- persistence rules belong in `CMacroTable`
- external file compatibility belongs in `macro_file_io`

### If you add merge semantics

Prefer this layering:
- parse/load into `CMacroTable`
- merge through a helper in `setup/macro_utils.cpp`
- rebuild or update the `GtkListStore`
- preserve sentinel row
- persist only after UI confirmation

That keeps logic DRY and avoids hidden divergences between button import,
clipboard paste, and future drag-and-drop.

### If you add undo/redo

Use `CMacroTable` snapshots or a helper derived from it.
Do not make the undo model depend only on raw `GtkTreeModel` row order, because
sorting and sentinel-row behaviour make the tree a less stable source of truth.

### If you add drag-and-drop

Use the dedicated design note:
- `project_planning/UI/macros/20260308_203348_drag_drop_merge_design.md`

That note assumes:
- import button keeps `KeepExisting` semantics
- drag-in defaults to overwrite semantics
- merge logic belongs in `setup/macro_utils.cpp`
- undo/redo should be introduced as snapshots rather than fine-grained edits

## Review hotspots

When debugging or reviewing a macro change, check these invariants first:

- the tree still ends with `STR_NULL_ITEM`
- duplicate keys are still rejected case-insensitively
- bulk reload still detaches the model during rebuilds
- UTF-8 display conversion still mirrors internal encoding correctly
- import/export still round-trips through `CMacroTable`
- import/export choosers still restore the last GSettings-backed folder
- save still happens only on dialog confirmation

## Git traceability

The branch contains many documentation commits, but for macro functionality the
most relevant commits are these:

- `e70a2c2` — macro storage, CRUD/sort UI, docs, and tools
- `4d2f661` — updated macro-file headers / touched macro-related files
- `dda4607` — broader routine documentation across setup and engine files
- `6dd2e55` — cleanup of placeholder parameter docs in touched files
- `faac9cf` — charset comments relevant to conversion-heavy macro paths

Useful lookup commands:

```bash
git log feature/macro-unlimited-crud-sort --no-merges --name-status --pretty=format:'COMMIT %h %ad %an %s' --date=short

git show --name-status e70a2c2

git grep -n 'on_btn_macroedit_clicked\|unikey_store_to_macro\|macro_table_load_any_format\|macro_table_write_any_format\|loadFromFile'
```

## Bottom line

Treat this branch as the point where macro editing stopped being a small dialog
and became a maintained subsystem spanning setup UI, list-store sync, format I/O,
and engine-backed persistence.

If you extend it, keep responsibilities split the same way:
- UI flow in `signal_handlers`
- merge/sync logic in `macro_utils`
- file compatibility in `macro_file_io`
- durable state in `CMacroTable`
- larger UX proposals in the separate design notes

## Duplicate resolution summary

- Engine / persistent files (`ukengine/mactab.cpp` / `CMacroTable`) use "dictionary" semantics: after a full load the table is sorted and duplicates are removed so the *last occurrence wins* (see `CMacroTable::removeDuplicates()` / `loadFromFile()`). This is the canonical behaviour used at runtime for macro expansion.

- Setup UI import/append paths currently use a different policy: `setup/macro_utils.cpp::list_store_append()` builds a case-insensitive set of existing keys and *skips* incoming keys (i.e. KeepExisting). This preserves values already present in the UI when appending.

- The repository contains both behaviours. The planned `list_store_merge()` + `MergeMode` design (see the related design note) explicitly acknowledges this and provides a single place to choose semantics (KeepExisting, OverwriteExisting, ReplaceAll, ReplaceSelected). Until that helper is implemented, be aware which path your code uses: direct `CMacroTable` loads get last-wins, while `list_store_append()` retains existing.

## Code-structure & redocumentation

- The branch already included many documentation commits and the note above maps responsibilities across subsystems. To make code structure and ownership explicit for future contributors, add a short, focused code-structure document at `project_planning/UI/macros/code_structure.md` (1–2 pages) that lists:
  - Source-of-truth: `CMacroTable` — file: `ukengine/mactab.{h,cpp}` and canonical methods (`loadFromFile`, `writeToFile`, `removeDuplicates`).
  - UI projection & helpers: `GtkListStore` and `setup/macro_utils.{h,cpp}` (`unikey_macro_to_store`, `unikey_store_to_macro`, `list_store_*`).
  - I/O helpers: `setup/macro_file_io.{h,cpp}` (`macro_table_load_any_format`, `macro_table_write_any_format`).
  - Dialog & UX wiring: `setup/signal_handlers.{h,cpp}` (`on_btn_macroedit_clicked` and import/export callbacks).
  - Tooling: `tools/macro_probe.cpp`, `tools/stress_macro_test.sh`.

- This code-structure doc should also state the duplicate-resolution matrix (engine vs UI) and point to the design note for planned merge semantics. Add a short link/PR checklist that requires updating these three locations when changing semantics:
  - `ukengine/mactab.*` (persistence & runtime behaviour)
  - `setup/macro_utils.*` (UI merge/sync helpers)
  - `setup/macro_file_io.*` (format read/write helpers)

- If you want, I can create `project_planning/UI/macros/code_structure.md` now with the suggested content and cross-references; say "yes" and I'll add it.
