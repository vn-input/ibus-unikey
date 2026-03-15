# Legacy cleanup mitigation plan

Branch context: `feature/macro-unlimited-crud-sort`
Cleanup branch: `feature/legacy-cleanup-phase1`
Related maintainer note: `project_planning/UI/macros/20260301_170054_macros_improvements_and_code_comments.md`
Related design notes:
- `project_planning/UI/macros/20260301_170054_macros_improvements_and_code_comments_design.md`
- `project_planning/UI/macros/20260308_203348_drag_drop_merge_design.md`
- `project_planning/UI/macros/code_structure.md`

## Implementation Status

| Phase | Description | Status |
|-------|-------------|--------|
| 0 | Baseline capture | ✅ Completed |
| 1 | Test scaffolding | ✅ Completed |
| 2 | Centralize constants | ✅ Completed |
| 3 | Decompose ukengine.cpp | ⏸️ Deferred (high-risk) |
| 4 | Decompose setup callbacks | ⏸️ Deferred (complex) |
| 5 | Contain global state | ✅ Completed |
| 6 | Error handling | ✅ Completed |
| 7 | Documentation | ✅ Completed |
| 8 | Macro dialog UX | 🔜 Not started |

### Completed Work Summary

**Phase 0-2** (commit `0c0b019`):
- Verified baseline: 3 tests passing
- Created `tests/ukengine/` with 2 test executables
- Created `ukengine/uk_shared_constants.h` with canonical `UK_WORD_BREAK_SYMS`
- Fixed drift bug in `src/engine.cpp` (incorrectly treated ~, `, ^ as word breaks)
- Renamed `ClassifierTableInitialized` → `is_classifier_table_initialized`

**Phase 5-6** (current work):
- Created `setup/app_context.h/cpp` to contain global UI state
- Added error handling to `ibus_unikey_config_set_string/boolean()` with return values
- Added error handling to `init_gtk_builder()` with failure path
- Fixed memory leak in icon path handling

**Deferred Work**:
- Phase 3 (ukengine.cpp decomposition): Internal static functions with complex dependencies
- Phase 4 (signal_handlers.cpp split): 1364-line file requires careful surgery

## Purpose

This document turns the current code-review observations into a staged cleanup
plan that mitigates the remaining guideline gaps across legacy engine and setup
code.

The goal is not a rewrite. The goal is to reduce maintenance risk while keeping
behavior stable, preserving the recently landed macro work, and improving the
parts of the codebase that are still least aligned with the development
standards: oversized files/functions, duplicated constants, broad global state,
inline complex conditions, magic numbers, unchecked return paths, and limited
core test coverage.

## Summary of the shortcomings to mitigate

The current issues fall into seven broad categories:

1. monolithic legacy files and routines in `ukengine` and setup UI code;
2. duplicated constants and classification logic (`WordBreakSyms` drift);
3. heavy use of global state across setup, config, and engine layers;
4. complex inline boolean logic in older code paths;
5. inconsistent naming and vague temporaries in legacy areas;
6. scattered magic numbers and raw buffer/table sizes;
7. ignored error returns and incomplete cleanup/failure handling;
8. insufficient automated coverage for core engine behavior compared with the
   newer macro UI work.

## Guiding principles for this cleanup

This work should follow the project development guidelines already used in newer
setup/macro code:

- keep modules small and responsibility-focused;
- extract repeated logic into shared helpers;
- precompute complex conditions into descriptive booleans;
- minimize global-state reach;
- replace magic numbers with named constants;
- handle error returns explicitly;
- add tests before high-risk structural refactors;
- document ownership boundaries as the structure evolves.

## Priority order

The cleanup should proceed in this order:

1. establish regression tests for core behavior;
2. centralize shared constants and duplicated classification data;
3. split `ukengine.cpp` by behavior seams;
4. decompose the setup callback layer (starting with `setup/signal_handlers.cpp`,
   ending with per-dialog handlers under `setup/gui/handlers/`);
5. reduce global-state reach through explicit context and wrappers;
6. normalize boolean clarity, naming, and magic-number usage;
7. tighten error handling and cleanup paths;
8. update planning/maintenance documentation after each structural milestone.

That order reduces regression risk while making later refactors easier and more
mechanical.

## Workstreams

### 1. Build a safety net before major refactors

#### Goals

- protect existing engine behavior before splitting large routines;
- keep the macro feature branch stable while cleanup proceeds;
- allow extraction work to happen in small reviewable batches.

#### Actions

- Add a new parallel core test area such as `tests/ukengine/`.
- Wire it into `tests/CMakeLists.txt` in the same style as the existing
  `tests/UI/macros` tree.
- Add focused tests for these high-risk engine entry points:
  - `UkEngine::appendVowel()`
  - `UkEngine::appendConsonnant()`
  - `UkEngine::process()`
  - tone/roof/hook behavior around the current mark-processing paths
- Add regression tests for word-break and character classification behavior so
  later constant-sharing changes are protected.
- Preserve the existing macro UI test suite and keep it green throughout.

#### Deliverables

- new `tests/ukengine/CMakeLists.txt`;
- initial fixture-driven unit/regression tests for core engine behavior;
- test cases for classification and buffer edge cases.

### 2. Centralize shared constants and duplicated logic

#### Problem

`WordBreakSyms` is duplicated in both engine-facing code paths and the copies do
not match exactly, which creates drift risk.

There are also scattered literal sizes and ASCII ranges in multiple files.

#### Actions

- Move `WordBreakSyms` into a single shared constant definition reachable from
  both `ukengine/inputproc.cpp` and `src/engine.cpp`.
- Introduce named constants for repeated classifier table sizes and ASCII range
  boundaries in `ukengine/inputproc.cpp`.
- Introduce named constants for raw buffer sizes currently used in
  `ukengine/unikey.cpp` and `src/engine.cpp`.
- Rename legacy boolean globals such as `ClassifierTableInitialized` toward a
  guideline-compliant boolean name, e.g. `is_classifier_table_initialized`.

#### Suggested structure

Possible central locations:

- a small shared constants header/source under `ukengine/`; or
- a narrow common header included by both `ukengine` and `src` targets.

The exact placement should minimize dependency spread and avoid creating a new
catch-all utility dump.

#### Deliverables

- one source of truth for `WordBreakSyms`;
- one source of truth for relevant table-size and range constants;
- updated call sites and regression tests confirming unchanged behavior.

### 3. Split `ukengine.cpp` by responsibility

#### Problem

`ukengine/ukengine.cpp` remains a monolithic legacy implementation with several
large multi-responsibility routines. The biggest maintenance risk is not just
file size, but the fact that mutation, validation, sequence lookup, tone
repositioning, and output handling are still intertwined.

#### Actions

- Keep the public `UkEngine` interface stable first.
- Extract private helper functions or internal implementation units around these
  responsibilities:
  - vowel append logic;
  - consonant append logic;
  - tone/mark mutation;
  - vowel-sequence transformation;
  - validation of consonant-vowel-consonant combinations;
  - change tracking and buffer mutation bookkeeping;
  - output generation and revert handling.
- Use the existing seams already visible around routines such as:
  - `processRoof()`
  - `processHook()`
  - `processTone()`
  - `appendVowel()`
  - `appendConsonnant()`
  - `process()`
- Prefer extraction into small internal helpers first, then move helpers into
  new translation units once tests prove the boundaries are correct.
- Avoid mixing behavioral changes with structural changes in the same patch.

#### Suggested phase breakdown

Phase 3A:
- extract pure or near-pure helper logic for validation and sequence lookup.

Phase 3B:
- extract buffer mutation helpers and tone reposition helpers.

Phase 3C:
- split append/process code into smaller source files if the helper boundaries
  are stable.

#### Deliverables

- smaller focused helpers inside `ukengine`;
- reduced function length in the largest routines;
- optional new internal source files added to `ukengine/CMakeLists.txt`;
- unchanged observable behavior confirmed by tests.

### 4. Decompose setup callback layer (currently `setup/signal_handlers.cpp`)

#### Problem

The setup callback layer (currently `setup/signal_handlers.cpp`) is acting as
an oversized callback bucket. In
particular, `on_btn_macro_export_clicked()` currently mixes dialog flow,
filename normalization, overwrite policy, list-store traversal, persistence, and
user interaction.

#### Actions

- Break the export path into smaller helpers with clear ownership, for example:
  - chooser creation/configuration;
  - export filename normalization;
  - overwrite confirmation policy;
  - conversion from UI list model to macro table;
  - persistence through the file I/O layer;
  - post-save UI notification/state update.
- Reuse existing focused modules where possible:
  - `setup/macro_export_filename.cpp`
  - `setup/macro_file_io.cpp`
  - `setup/macro_dialog_state.cpp`
  - `setup/macro_utils.cpp`
- If the remaining callback layer remains too broad after helper extraction,
  split it by feature area (or complete the `setup/gui/handlers/` migration),
  such as:
  - macro dialog lifecycle and edit/save flow;
  - import/export callbacks;
  - generic setup/config callbacks.
- Rename vague temporaries such as `b` to descriptive names.
- Normalize complex conditions into named booleans before `if` blocks.

#### Deliverables

- a smaller `on_btn_macro_export_clicked()`;
- reduced callback complexity across setup UI code;
- better separation between UI plumbing and persistence logic;
- preserved macro import/export behavior and chooser-directory state behavior.

### 5. Reduce global-state reach incrementally

#### Problem

Global state still exists across setup UI globals, config singleton access,
input classification tables, and core runtime globals.

This is a long-term maintainability problem, but it should be addressed
incrementally to avoid destabilizing the application.

#### Actions

- In setup code, replace direct use of broad globals in `setup/main.cpp` with a
  small context object or explicit accessors passed into callback wiring.
- In config code, keep the existing singleton behavior initially but introduce a
  narrower interface around `unikey_config` access so more call sites depend on
  the interface rather than the singleton implementation.
- In `ukengine/inputproc.cpp`, isolate classifier-table initialization and make
  ownership/initialization clearer.
- In `ukengine/unikey.cpp`, identify which globals are true shared runtime state
  versus historical convenience state and wrap access behind explicit helper
  functions where practical.
- Favor containment first, elimination second.

#### Deliverables

- fewer direct references to global state from feature code;
- clearer ownership of setup and engine state;
- groundwork for later dependency injection or context passing if needed.

### 6. Normalize boolean clarity, naming, and magic numbers

#### Problem

Older engine code often writes complex conditions inline, and some legacy names
and literals do not match the style already used in the newer macro modules.

#### Actions

- Refactor complex inline conditions in `ukengine.cpp` and `unikey.cpp` into
  descriptive intermediate booleans.
- Prefer names like `is_...`, `has_...`, `can_...`, and `should_...` for boolean
  state.
- Replace one-letter temporaries unless they are conventional loop indexes.
- Introduce named constants for repeated values such as table sizes, ASCII
  ranges, and buffer sizes.
- Avoid performing this as a repo-wide style sweep; instead, tie naming/boolean
  cleanup to each touched hotspot so the changes stay reviewable.

#### Deliverables

- clearer conditional logic in touched engine/setup files;
- more consistent boolean naming;
- fewer unexplained numeric literals.

### 7. Tighten error handling and cleanup paths

#### Problem

Several calls currently ignore return values or leave cleanup/ownership concerns
unresolved.

#### Actions

- Handle or propagate the return value of `g_settings_set_value()` in
  `src/config/unikey_config.cpp`.
- Resolve the outstanding ownership/TODO area associated with
  `g_signal_connect()` setup in `src/config/unikey_config.cpp`.
- Add explicit failure handling for GTK resource loading in `setup/main.cpp`,
  including builder file loading and icon file loading.
- Ensure temporary allocations and intermediate objects are cleaned up on both
  success and failure paths.
- Add tests for invalid/missing settings values where practical.

#### Deliverables

- explicit error handling in config persistence paths;
- clearer startup failure behavior in setup code;
- fewer silent failures and fewer ownership ambiguities.

### 8. Expand automated coverage beyond UI macro tests

#### Problem

The project already has solid targeted tests for recent macro UI work, but core
logic is still under-tested relative to its complexity.

#### Actions

- Preserve and keep extending `tests/UI/macros` where the macro UI behavior is
  still evolving.
- Add core tests for:
  - word-break classification;
  - buffer edge cases;
  - tone reposition behavior;
  - roof/hook application and removal;
  - duplicate-removal/runtime macro semantics where relevant to engine storage;
  - legacy behavior that must remain stable during refactors.
- Consider small fixture-based engine tests in the same spirit as the existing
  macro file I/O fixtures.
- Add narrow tests whenever a large helper is extracted from `ukengine.cpp` or
  from the setup callback layer (`setup/signal_handlers.cpp` /
  `setup/gui/handlers/*`).

#### Deliverables

- better balance between UI-layer and core-layer test coverage;
- safer future cleanup work in `ukengine` and `src`.

## Detailed phased implementation guide

This section is intended to be executable by another developer without needing
to reconstruct the cleanup strategy from commit history.

Each phase should be delivered in one or more small mergeable patches. Do not
start a later phase until the exit criteria of the current phase are met.

## Target sub-module creation and extraction map

This cleanup should not only shrink functions. It should create clearer module
boundaries so duplicated logic is centralized and future feature work reuses the
same code paths.

The module plan below is the preferred target structure. It does not need to
land in one patch, but each extraction should move toward this shape.

### A. Shared constants and classification modules

#### A1. Shared character/classification constants

Purpose:

- provide one source of truth for character-classification tables and repeated
  constants;
- remove drift between engine-side and IBus-side processing logic;
- make numeric literals searchable and documented in one place.

Suggested files:

- `ukengine/uk_constants.h`
- `ukengine/uk_constants.cpp`

Suggested ownership:

- `WordBreakSyms` canonical definition;
- shared table-size constants;
- shared buffer-size constants that genuinely apply across modules;
- character range constants and common classification limits.

Rules:

- do not dump unrelated constants here;
- keep module-local constants local unless they are reused in more than one
  component;
- prefer domain names over generic names like `MAX_BUFFER` when the scope is
  actually narrower.

#### A2. Input classification helpers

Purpose:

- isolate initialization and lookup responsibilities currently mixed into
  `ukengine/inputproc.cpp`;
- reduce duplication and global-state reach around classifier tables.

Suggested files:

- `ukengine/uk_input_classify.h`
- `ukengine/uk_input_classify.cpp`

Suggested ownership:

- classifier-table initialization;
- read-only lookup helpers for character classes;
- word-break checks and helper predicates reused by both input processing and
  higher-level engine paths.

Likely code to move first:

- table initialization helpers;
- `WordBreakSyms` consumers;
- small classification predicates that are currently file-local and repeated in
  spirit.

### B. `ukengine` decomposition modules

The main goal here is to turn `ukengine.cpp` from a monolithic implementation
unit into a coordinator over smaller internal modules.

#### B1. Sequence analysis module

Purpose:

- centralize read-only logic for vowel/consonant sequence interpretation;
- make sequence decisions reusable across roof, hook, tone, and append flows.

Suggested files:

- `ukengine/uk_sequence_analysis.h`
- `ukengine/uk_sequence_analysis.cpp`

Suggested ownership:

- sequence lookup wrappers;
- tone-position selection helpers;
- helpers that derive `vStart`, `vEnd`, and related sequence boundaries;
- validation-oriented read-only helpers that interpret current buffer state.

DRY benefit:

- the same sequence-analysis rules stop being recomputed inline in multiple
  mark-handling routines.

#### B2. Validation module

Purpose:

- centralize phonetic and structural validation logic so append and mark flows
  do not each carry their own local validation setup.

Suggested files:

- `ukengine/uk_validation.h`
- `ukengine/uk_validation.cpp`

Suggested ownership:

- consonant-vowel-consonant validity checks;
- precondition helpers used before mutations;
- small boolean helpers that answer domain questions such as whether a
  particular mark can be applied at the current position.

DRY benefit:

- validity rules live in one module instead of being rebuilt around `c1`, `c2`,
  and `newVs` inside several large functions.

#### B3. Buffer mutation module

Purpose:

- isolate write-side logic that mutates the engine buffer;
- make mutation operations composable and easier to test.

Suggested files:

- `ukengine/uk_buffer_ops.h`
- `ukengine/uk_buffer_ops.cpp`

Suggested ownership:

- tone movement helpers;
- vowel-sequence replacement helpers;
- mark-application helpers;
- repeated `markChange()`-adjacent write sequences;
- localized revert bookkeeping helpers.

Rules:

- prefer explicit parameters over helpers that silently depend on many globals;
- keep mutation helpers small and named after the domain effect they perform.

#### B4. Append-processing module

Purpose:

- split append-oriented control flow away from unrelated mark-handling logic;
- reduce the size of `appendVowel()` and `appendConsonnant()`.

Suggested files:

- `ukengine/uk_append_flow.h`
- `ukengine/uk_append_flow.cpp`

Suggested ownership:

- append-specific flow decomposition;
- shared helper decisions between vowel and consonant append paths;
- pre-append validation and post-append bookkeeping orchestration.

#### B5. Mark-processing module

Purpose:

- group roof, hook, and tone orchestration into a coherent subsystem.

Suggested files:

- `ukengine/uk_mark_flow.h`
- `ukengine/uk_mark_flow.cpp`

Suggested ownership:

- shared mark-processing orchestration;
- common prechecks for applying/removing marks;
- logic reused by `processRoof()`, `processHook()`, and `processTone()`.

DRY benefit:

- repeated patterns around locating tone position, selecting change position,
  validating applicability, and reapplying tone become shared helpers instead
  of per-function boilerplate.

#### B6. Output/finalization module

Purpose:

- isolate output preparation and final event result construction from the
  decision logic that determines what should happen.

Suggested files:

- `ukengine/uk_output_flow.h`
- `ukengine/uk_output_flow.cpp`

Suggested ownership:

- output buffer preparation;
- final composition result shaping;
- helper logic at the end of `UkEngine::process()` that does not need to remain
  interleaved with event classification.

### C. Setup macro UI extraction modules

The setup area already has some useful helper modules. The next step is to make
the callback layer thinner and give each macro workflow a clear home.

#### C0. Create a `setup/gui/` directory and split dialogs into modules

Current state:

- the setup UI is largely defined in one Glade file: `setup/ibus-unikey.ui`.
- that file already contains multiple top-level widgets/dialogs, including:
  - `main_window` (GtkWindow)
  - `macro_dialog` (GtkDialog)
  - `add_macro_dialog` (GtkDialog) - **dual-purpose**: reused for both "Add" and
    "Edit" modes via `run_macro_editor_dialog(initial_key, initial_value)`.
- event handlers are currently registered by GTK builder and mostly implemented
  in `setup/signal_handlers.{h,cpp}` (before the `setup/gui/` split).

Target state:

- create a dedicated GUI directory so dialogs are discoverable and independent;
- place each dialog’s UI definition in its own `.ui` file;
- split handler code per dialog so one module owns one dialog’s callbacks;
- keep UI definitions (`.ui`) separate from event handler implementation
  (`*.cpp`) and orchestration/controller code.

Suggested directory layout:

- `setup/gui/`
  - `setup/gui/ui/`
    - `main_window.ui`
    - `macro_dialog.ui`
    - `add_macro_dialog.ui` (reused for both add and edit modes; layout adjusted
      in Phase 8 (post-cleanup UX work) to support rich-text editor and
      top-aligned label)
  - `setup/gui/handlers/`
    - `main_window_handlers.cpp` / `main_window_handlers.h`
    - `macro_dialog_handlers.cpp` / `macro_dialog_handlers.h`
    - `add_macro_dialog_handlers.cpp` / `add_macro_dialog_handlers.h`
  - `setup/gui/builder/`
    - `gui_builder.cpp` / `gui_builder.h`
  - `setup/gui/dialogs/`
    - optional “thin UI access” wrappers such as `macro_dialog_ui.{h,cpp}`
  - shared macro dialog UX helpers (see section C-UX):
    - `setup/gui/macro_search.cpp` / `setup/gui/macro_search.h`
    - `setup/gui/macro_selection.cpp` / `setup/gui/macro_selection.h`
    - `setup/gui/macro_focus_helper.cpp` / `setup/gui/macro_focus_helper.h`
    - `setup/gui/macro_drag_drop.cpp` / `setup/gui/macro_drag_drop.h`
    - `setup/gui/macro_edit_dialog.cpp` / `setup/gui/macro_edit_dialog.h`

Notes on GTK builder wiring:

- GTK builder binds callbacks by handler name strings in the `.ui` file.
- Splitting handlers does not require renaming callbacks; it only requires the
  functions to remain linkable symbols in the final executable.
- Migration should avoid changing handler names unless necessary; keep the same
  function names initially and move the implementations.

Incremental migration strategy:

1. Introduce the `setup/gui/` directory and build it into the setup executable.
2. Move handler implementations (not signatures) from `setup/signal_handlers.cpp`
   into per-dialog handler modules while keeping the existing `.ui` file.
3. Split `setup/ibus-unikey.ui` into `setup/gui/ui/*.ui` per dialog.
4. Update builder loading so it loads multiple `.ui` files into one builder.
5. When stable, shrink `setup/signal_handlers.*` into an umbrella header or
   retire it in favor of per-dialog handler headers.

Deliverables:

- `setup/gui/` exists with clear UI/handler separation;
- each dialog is owned by one module;
- build/install uses multiple `.ui` files;
- callbacks remain behavior-compatible and tested.

#### C1. Macro dialog controller module

Purpose:

- hold high-level orchestration for opening, loading, editing, and saving macro
  dialogs;
- remove long workflow assembly from the setup callback layer (initially
  `setup/signal_handlers.cpp`, later `setup/gui/handlers/*`).

Suggested files:

- `setup/macro_dialog_controller.h`
- `setup/macro_dialog_controller.cpp`

Suggested ownership:

- load-edit-save dialog flow;
- bridge between UI widgets and lower-level macro utility modules;
- top-level success/failure flow for macro editor actions.

Relationship to `setup/gui/` split:

- the controller should orchestrate the macro dialog workflow;
- the per-dialog handler module should be thin and delegate to the controller.

#### C2. Macro chooser helpers module

Purpose:

- centralize chooser creation and remembered-directory handling for both import
  and export paths.

Suggested files:

- `setup/macro_chooser_helpers.h`
- `setup/macro_chooser_helpers.cpp`

Suggested ownership:

- chooser construction and configuration;
- default/remembered directory resolution;
- chooser-side validation helpers;
- shortcut/bookmark setup if needed.

DRY benefit:

- import and export no longer repeat chooser setup and directory-state logic.

#### C3. Macro import/export controller module

Purpose:

- move workflow-specific orchestration out of raw GTK callbacks while reusing
  the existing file I/O code.

Suggested files:

- `setup/macro_transfer_controller.h`
- `setup/macro_transfer_controller.cpp`

Suggested ownership:

- import flow orchestration;
- export flow orchestration;
- overwrite policy handling;
- high-level mapping between chooser result, model conversion, and file I/O.

Rules:

- keep file parsing and writing in `macro_file_io.cpp`;
- keep filename normalization in `macro_export_filename.cpp`;
- keep controller logic focused on sequencing those helpers.

#### C4. Macro model adapter module

Purpose:

- centralize conversion between `GtkListStore`, tree selection state, and
  `CMacroTable` operations.

Suggested files:

- `setup/macro_model_adapter.h`
- `setup/macro_model_adapter.cpp`

Suggested ownership:

- store-to-table conversion;
- table-to-store population;
- selected-row extraction helpers;
- reusable row traversal and bulk-update helpers.

DRY benefit:

- the same model traversal logic can be reused by import, export, edit, sort,
  clear, and future drag/drop flows.

#### C5. Macro action policy module

Purpose:

- centralize reusable policy decisions that are currently buried in callbacks.

Suggested files:

- `setup/macro_action_policy.h`
- `setup/macro_action_policy.cpp`

Suggested ownership:

- overwrite confirmation rules;
- duplicate/merge policy decisions for UI actions;
- small policy booleans for whether an action should be enabled or blocked.

DRY benefit:

- policy logic is reused by buttons, menus, shortcuts, and future drag/drop
  features instead of being reimplemented per entry point.

### C-UX. Macro dialog UX enhancement modules

These modules support the user-experience improvements requested for the macro
editor: search/navigation, multi-select state tracking, focus restoration, and
drag/drop reordering.

#### CUX1. Macro search and navigation module

Purpose:

- provide search-box and first/prev/next/last navigation for the macro table;
- search both "Word" and "Replace with" columns with the same pattern;
- keep search UI and logic separate from the main macro dialog control flow.

Suggested files:

- `setup/gui/macro_search.h`
- `setup/gui/macro_search.cpp`

Suggested ownership:

- search state tracking (pattern, current match index);
- helper functions to find next/previous/first/last matches across both columns;
- helper to highlight or scroll to a matched row;
- cache/iterator management to avoid rescanning the full table on each nav click.

Entry points:

- `on_macro_search_entry_changed()` - triggered by text-changed signal on search box;
- `on_btn_search_first_clicked()`, `on_btn_search_prev_clicked()`,
  `on_btn_search_next_clicked()`, `on_btn_search_last_clicked()` - navigation buttons.

#### CUX2. Macro selection state module

Purpose:

- track multi-select state, including the "first selection before multi" anchor
  so Esc can restore;
- handle Ctrl+click for toggle selection, Ctrl+A for select all, Esc to clear;
- integrate with the macro model adapter for bulk operations.

Suggested files:

- `setup/gui/macro_selection.h`
- `setup/gui/macro_selection.cpp`

Suggested ownership:

- selection anchor point (the row before multi-select began);
- helpers to compute multi-select via Ctrl+click;
- Ctrl+A all-selection helper;
- Esc clear-selection with anchor restoration;
- reusable predicates for whether an action is valid on current selection;
- signal handlers for key-press-event on tree_macro.

Entry points:

- `on_tree_macro_key_press_event()` - handle Ctrl+A and Esc;
- `on_tree_macro_button_press_event()` - handle Ctrl+click for toggle multi-select;
- `get_selection_anchor()`, `restore_anchor_selection()` - restore first selection.

#### CUX3. Macro focus/scroll helper module

Purpose:

- scroll the tree view to center on a selected row ("Focus" button behavior);
- abstract the GTK scroll-to-cell API so it can be called after the user
  scrolls away.

Suggested files:

- `setup/gui/macro_focus_helper.h`
- `setup/gui/macro_focus_helper.cpp`

Suggested ownership:

- scroll-to-selection logic with centering;
- path-validation before scrolling (in case selection was cleared).

Entry points:

- `on_btn_macro_focus_clicked()` - triggered by new "Focus" button.

#### CUX4. Macro dialog drag/drop module

Purpose:

- enable drag-reorder of selected rows within the macro table;
- provide visual feedback (cursor, drop-highlight) so users see where rows will land.

Suggested files:

- `setup/gui/macro_drag_drop.h`
- `setup/gui/macro_drag_drop.cpp`

Suggested ownership:

- GTK drag-source setup for selected rows;
- GTK drag-dest setup for drop zones;
- drag-motion handlers to show insert-row line/highlight;
- drag-drop handler to reorder rows in the list store and underlying model;
- visual cursor/highlight feedback.

Entry points:

- `on_tree_macro_drag_begin()`, `on_tree_macro_drag_motion()`,
  `on_tree_macro_drag_drop()`, `on_tree_macro_drag_end()` - drag/drop signal handlers.

Notes:

- coordinate with the macro model adapter so row reordering is reflected back to
  the `CMacroTable` when saved;
- keep the original sort order metadata if users later re-sort; allow undo of
  manual reorders via the planned macro_undo module if drag/drop is reversible.

#### CUX5. Macro edit dialog rich-text module

Purpose:

- replace the plain-text entry in `add_macro_dialog` with a rich-text view;
- redesign the dialog layout so the "With:" label aligns to top-left (not
  centered as it is currently in the grid).

Suggested files:

- `setup/gui/macro_edit_dialog.h`
- `setup/gui/macro_edit_dialog.cpp`

Suggested ownership:

- layout redesign: replace GtkEntry with GtkTextView in a scrolled window;
- preserve text content across add/edit mode switches;
- label alignment adjustment in the Glade `.ui` file or runtime property set;
- text-wrap and scrollbar auto-show on overflow.

### D. Setup application context and config modules

#### D1. Setup application context

Purpose:

- reduce wide global access in `setup/main.cpp` and callback code.

Suggested files:

- `setup/app_context.h`
- `setup/app_context.cpp`

Suggested ownership:

- shared widget references that must survive beyond one callback;
- builder-owned application state;
- helper accessors for common setup-time dependencies.

Rules:

- this should be a narrow context, not a new global dump object;
- only stable shared UI dependencies should live here.

#### D2. Config facade module

Purpose:

- narrow the usage surface of the config singleton before any deeper redesign.

Suggested files:

- `src/config/unikey_config_facade.h`
- `src/config/unikey_config_facade.cpp`

Suggested ownership:

- focused get/set helpers for values used by setup and engine call sites;
- checked persistence wrappers;
- helper functions that convert raw config access into smaller domain-level
  operations.

DRY benefit:

- multiple callers stop duplicating error handling and value-conversion logic
  around the same settings keys.

### E. Test sub-modules mirroring production modules

To maximize maintainability, the test tree should reflect the extracted module
boundaries.

Suggested test layout:

- `tests/ukengine/classification/`
- `tests/ukengine/sequence/`
- `tests/ukengine/mutation/`
- `tests/ukengine/process/`
- `tests/setup/macros/chooser/`
- `tests/setup/macros/model/`
- `tests/setup/macros/transfer/`
- `tests/setup/gui/macro_search/`
- `tests/setup/gui/macro_selection/`
- `tests/setup/gui/macro_focus/`
- `tests/setup/gui/macro_drag_drop/`
- `tests/setup/gui/macro_edit_dialog/`

Guideline:

- when a production helper module is created, add at least one narrow test file
  that targets that module’s behavior.

### F. Extraction order for the sub-modules

Create and fill the sub-modules in this order:

1. `uk_constants`;
2. `uk_input_classify`;
3. `uk_sequence_analysis`;
4. `uk_validation`;
5. `uk_buffer_ops`;
6. `uk_mark_flow`;
7. `uk_append_flow`;
8. `uk_output_flow`;
9. `macro_model_adapter`;
10. `macro_chooser_helpers`;
11. `macro_transfer_controller`;
12. `macro_dialog_controller`;
13. `macro_action_policy`;
14. `app_context`;
15. `unikey_config_facade`;
16. `macro_search` (CUX1);
17. `macro_selection` (CUX2);
18. `macro_focus_helper` (CUX3);
19. `macro_drag_drop` (CUX4);
20. `macro_edit_dialog` (CUX5).

This order prioritizes shared reusable logic before higher-level orchestration.
That keeps later extractions thin and DRY instead of producing another layer of
duplicated wrapper code. The UX enhancement modules (16–20) depend on the macro
logic modules being in place, so they are extracted last in Phase 8
(post-cleanup UX work).

### Cross-phase execution rules

Apply these rules in every phase:

- keep behavior-preserving refactors separate from functional fixes;
- keep each patch small enough to review in isolation;
- run the relevant tests after every structural change;
- if a phase reveals a behavioral bug, record it separately instead of folding
  it into the structural patch unless the fix is required to continue;
- update planning notes only after the new structure stabilizes.

Recommended patch sizing:

- one patch for build/test scaffolding;
- one patch per constant-sharing or naming cleanup cluster;
- one patch per extracted `ukengine` helper cluster;
- one patch per extracted setup UI helper cluster;
- one patch for documentation refresh.

### Phase 0: Baseline capture and worktree preparation

#### Objective

Freeze a reliable before-state so later phases can prove they preserved
behavior.

#### Files to inspect or touch

- `CMakeLists.txt`
- `tests/CMakeLists.txt`
- `tests/UI/CMakeLists.txt`
- `tests/UI/macros/CMakeLists.txt`
- `ukengine/CMakeLists.txt`

#### Tasks

1. Build the tree from a clean state and record whether the current branch
  builds successfully.
2. Run the current test suite and save the output in the commit message, PR
  description, or local engineering notes.
3. Record the main cleanup targets and current line-heavy functions:
  - `ukengine/ukengine.cpp`
  - `setup/signal_handlers.cpp`
  - `ukengine/inputproc.cpp`
  - `src/engine.cpp`
  - `src/config/unikey_config.cpp`
  - `setup/main.cpp`
  - `ukengine/unikey.cpp`
4. Confirm which current tests protect macro import/export and chooser folder
  behavior so later setup refactors do not silently regress them.

#### Outputs

- a known-good baseline build result;
- a known-good baseline test result;
- a short list of exact hotspots for the next phases.

#### Exit criteria

- the branch builds or the known baseline failures are explicitly recorded;
- existing tests are run and their status is documented.

### Phase 1: Test scaffolding for core engine behavior

#### Objective

Create a parallel core test structure so `ukengine` refactors can proceed with a
real safety net rather than relying only on manual verification.

#### Files to create or modify

- `tests/CMakeLists.txt`
- `tests/ukengine/CMakeLists.txt` (new)
- `tests/ukengine/README.md` (new)
- one or more new test sources under `tests/ukengine/`
- optionally fixture files under `tests/ukengine/fixtures/`

#### Implementation steps

1. Create `tests/ukengine/` as a parallel tree beside `tests/UI/`.
2. Add `add_subdirectory(ukengine)` to `tests/CMakeLists.txt`.
3. Add a focused `tests/ukengine/CMakeLists.txt` that builds narrow test
  executables instead of one large catch-all binary.
4. Start with regression tests that require the least internal exposure:
  - character classification behavior;
  - word-break behavior;
  - buffer-processing edge cases reachable through public entry points;
  - tone/roof/hook regressions that can be driven from existing public APIs.
5. Add a short `README.md` describing build/run expectations and fixture usage.

#### Suggested first test groups

- `test-inputproc-classification`
- `test-ukengine-tone-processing`
- `test-ukengine-vowel-consonant-processing`

#### Validation

- new tests build under CMake;
- existing `tests/UI/macros` still build and run;
- new tests pass before any engine decomposition begins.

#### Exit criteria

- `tests/ukengine` exists and is wired into the build;
- at least one regression test protects classification behavior;
- at least one regression test protects mark/tone behavior;
- test instructions are documented locally in the new test README.

### Phase 2: Centralize constants and remove obvious DRY violations

#### Objective

Eliminate duplicated lookup data and replace repeated literals with named
constants before deeper refactoring starts.

#### Primary targets

- `ukengine/inputproc.cpp`
- `src/engine.cpp`
- `ukengine/unikey.cpp`
- any new shared constants header/source introduced for this phase

#### Implementation steps

1. Compare the two current `WordBreakSyms` definitions and decide the canonical
  sequence to preserve.
2. Create `ukengine/uk_constants.{h,cpp}` as the first shared-constants module.
3. Introduce a single source of truth for that sequence and other genuinely
  shared constants in that module.
4. Create `ukengine/uk_input_classify.{h,cpp}` for classifier initialization
  and lookup helpers that currently live inline in `inputproc.cpp`.
5. Update both `ukengine/inputproc.cpp` and `src/engine.cpp` to consume the new
  shared constant/classification entry points.
6. Extract repeated classifier-related numeric literals into named constants in
  the smallest scope that still avoids duplication.
7. Extract repeated buffer-size literals into named constants near the owning
  module rather than creating one global numeric dump.
8. Rename legacy boolean state such as `ClassifierTableInitialized` only in the
  files touched by this phase.

#### Review checklist

- no behavior change is mixed with the constant move;
- shared constants are discoverable and narrowly scoped;
- names follow the project boolean/constant conventions;
- tests confirm the canonical `WordBreakSyms` behavior did not change.

#### Validation

- build passes;
- classification-related tests pass;
- no unrelated style churn appears in the patch.

#### Exit criteria

- `WordBreakSyms` has exactly one maintained source of truth;
- major repeated literals touched in this phase are named;
- classification behavior remains unchanged according to tests.

### Phase 3: Decompose `ukengine.cpp` in safe slices

#### Objective

Reduce the size and responsibility of the largest engine routines without
changing public behavior.

#### Primary targets

- `ukengine/ukengine.cpp`
- `ukengine/ukengine.h` if internal declarations must move
- `ukengine/CMakeLists.txt` if new implementation files are introduced
- new internal helper source/header files as needed

#### General rule for this phase

Extract one responsibility at a time. Do not extract validation, mutation, and
output generation in the same patch unless they are trivially inseparable.

#### Subphase 3A: Extract read-only or near-pure helpers

Targets:

- sequence lookup helpers;
- tone-position calculation helpers;
- consonant-vowel validity checks;
- small decision helpers that currently live inline.

Steps:

1. Identify helper logic that reads state but performs little or no mutation.
2. Create `ukengine/uk_sequence_analysis.{h,cpp}` and move the most stable
  read-only sequence helpers there.
3. Create `ukengine/uk_validation.{h,cpp}` for reusable validation predicates
  once they can be expressed without hidden side effects.
4. Leave anything still tightly coupled in `ukengine.cpp` temporarily as local
  private helpers until it is safe to move.
5. Add or extend tests that exercise those paths through public APIs.
6. Only after tests pass, move additional stable helpers into those internal
  files.

Exit criteria:

- repeated decision logic is no longer duplicated inline in the largest
  routines;
- helper names describe the domain behavior clearly.

#### Subphase 3B: Extract mutation and bookkeeping helpers

Targets:

- tone reposition logic;
- `markChange()`-adjacent bookkeeping patterns;
- vowel-sequence mutation blocks;
- revert/undo-like local engine state transitions.

Steps:

1. Create `ukengine/uk_buffer_ops.{h,cpp}` for mutation-heavy helper routines.
2. Isolate repeated mutation sequences into narrow helpers.
3. Keep helper parameters explicit rather than letting helpers read broad hidden
  global state where possible.
4. Extract common mark-oriented orchestration into `ukengine/uk_mark_flow.{h,cpp}`
  when roof/hook/tone helpers begin sharing the same preparation and cleanup
  patterns.
5. Retest after each extraction.
6. If helper signatures become too broad, pause and split responsibilities
  further rather than introducing a vague parameter bag.

Exit criteria:

- the largest routines lose meaningful chunks of mutation boilerplate;
- mutation helpers remain understandable and domain-named.

#### Subphase 3C: Split file-level responsibilities if the seams are stable

Candidate module splits:

- mark/tone handling helpers in `uk_mark_flow`;
- append-processing helpers in `uk_append_flow`;
- validation helpers in `uk_validation`;
- engine-output helpers in `uk_output_flow`.

Steps:

1. Create `ukengine/uk_append_flow.{h,cpp}` once append-specific helpers have a
  clear boundary.
2. Create `ukengine/uk_output_flow.{h,cpp}` for end-of-process output shaping
  and finalization helpers that do not belong in append/mark logic.
3. Move only stable helper groups into new files.
4. Update `ukengine/CMakeLists.txt`.
5. Keep private APIs internal to `ukengine` where possible.
6. Avoid exporting new public types unless there is a clear long-term need.

#### Validation

- all old and new engine tests pass after every subphase;
- public `UkEngine` behavior remains unchanged for existing callers;
- the diff shows decomposition rather than a hidden rewrite.

#### Exit criteria

- `appendVowel()`, `appendConsonnant()`, and `process()` are materially smaller
  or delegate clear responsibilities to helpers;
- helper boundaries are understandable enough that another developer can modify
  one area without scanning the entire file.

### Phase 4: Decompose setup callback flows

#### Objective

Turn the oversized setup callback module into a smaller orchestration layer that
delegates work to focused helpers.

#### Primary targets

- `setup/signal_handlers.cpp`
- `setup/signal_handlers.h`
- `setup/macro_export_filename.cpp`
- `setup/macro_file_io.cpp`
- `setup/macro_dialog_state.cpp`
- `setup/macro_utils.cpp`
- `setup/CMakeLists.txt` if new helper files are added
- `setup/ibus-unikey.ui` during the UI split
- new GUI directory contents under `setup/gui/`

#### Subphase 4A0: Split handler implementations per dialog (no `.ui` split yet)

Objective:

- reduce the size of `setup/signal_handlers.cpp` immediately without introducing a UI
  file migration at the same time.

Steps:

1. Create `setup/gui/handlers/` and add per-dialog handler files:
  - `main_window_handlers.*`
  - `macro_dialog_handlers.*`
  - `add_macro_dialog_handlers.*`
2. Move callback implementations into the relevant per-dialog handler file while
  keeping function signatures and handler names unchanged.
3. Keep `setup/signal_handlers.h` as the exported declarations initially, but
  update the build so the implementations are no longer centralized in one
  translation unit.
4. Keep `setup/signal_handlers.cpp` as a thin file that includes or forwards to
  the per-dialog modules until the migration is complete.

Validation:

- build succeeds;
- macro UI tests still pass;
- runtime behavior unchanged.

Exit criteria:

- the largest handler bodies are moved out of `setup/signal_handlers.cpp`;
- each dialog’s callbacks have a clear owning file.

#### Subphase 4A: Export flow decomposition

Start with `on_btn_macro_export_clicked()` because it currently combines the
most unrelated work.

Steps:

1. Create `setup/macro_chooser_helpers.{h,cpp}` for chooser creation and
  remembered-directory handling.
2. Create `setup/macro_model_adapter.{h,cpp}` for store/table traversal and
  conversion logic.
3. Create `setup/macro_transfer_controller.{h,cpp}` for high-level import/export
  orchestration.
4. Separate chooser/dialog creation from export execution.
5. Separate export filename normalization from overwrite checks.
6. Separate list-store to macro-table conversion from file serialization.
7. Move persistence and format-selection logic into the existing file I/O layer
  wherever possible.
8. Keep the callback as a short orchestration function that reads like a flow
  summary.

Target outcome:

- callback opens dialog;
- helper normalizes path;
- helper decides overwrite policy;
- helper serializes/export writes;
- callback reports result and updates remembered directory state.

#### Subphase 4B: Import and edit-flow decomposition

Steps:

1. Create `setup/macro_dialog_controller.{h,cpp}` if the dialog lifecycle logic
  is still spread across too many callback bodies.
2. Create `setup/macro_action_policy.{h,cpp}` for reusable overwrite, duplicate,
  and action-availability policy decisions.
3. Identify import paths and edit/save flows that duplicate pieces of the export
  flow or model-conversion logic.
4. Reuse extracted helpers instead of creating export-only utilities.
5. Group related callbacks by feature area if `setup/signal_handlers.cpp` remains too
  large.

#### Naming and style cleanup during this phase

- rename vague locals such as `b`;
- replace inline compound conditions with named booleans;
- avoid mixing broad UI behavior changes with structural extraction.

#### Validation

- existing macro UI tests still pass;
- chooser folder memory behavior still matches current documented behavior;
- import/export fixtures still round-trip correctly.

#### Exit criteria

- `on_btn_macro_export_clicked()` becomes a short coordinator rather than a
  multi-stage implementation bucket;
- shared import/export logic lives in helper modules, not duplicated callbacks.

#### Subphase 4C: Split the monolithic `.ui` file into one `.ui` per dialog

Objective:

- make each dialog independently maintainable and easier to reason about;
- reduce merge conflicts in the UI layer;
- align code modules with UI file ownership.

Steps:

1. Create `setup/gui/ui/`.
2. Split `setup/ibus-unikey.ui` into:
  - `setup/gui/ui/main_window.ui`
  - `setup/gui/ui/macro_dialog.ui`
  - `setup/gui/ui/add_macro_dialog.ui`
3. Keep object IDs stable to avoid churn in lookup code (e.g. keep
  `main_window`, `macro_dialog`, `add_macro_dialog`, `tree_macro`, and related
  IDs unchanged).
4. Create `setup/gui/builder/gui_builder.{h,cpp}` to load the builder and
  register signals.
5. Update `setup/main.cpp` to call the GUI builder helper and to fetch the
  top-level windows/dialogs from the builder.
6. Update `setup/CMakeLists.txt` install rules to install all `.ui` files under
  `${PKGDATADIR}/ui`.
7. Keep a temporary compatibility path (if needed) where the builder loads the
  old `ibus-unikey.ui` while the new files are introduced, but prefer a clean
  switch once the per-dialog files are complete.

Validation:

- setup executable loads all dialogs from the new per-dialog `.ui` files;
- macro UI tests still pass;
- there are no duplicate ID collisions across the loaded `.ui` set.

Exit criteria:

- `setup/ibus-unikey.ui` is either retired or reduced to a minimal shim;
- each dialog’s UI lives in `setup/gui/ui/<dialog>.ui`;
- builder-loading code is centralized in `setup/gui/builder/gui_builder.*`.

#### Subphase 4D: Replace `signal_handlers.*` with per-dialog headers

Objective:

- complete the handler split by making the per-dialog handler headers the
  canonical declarations.

Steps:

1. Move declarations out of `setup/signal_handlers.h` into:
  - `setup/gui/handlers/main_window_handlers.h`
  - `setup/gui/handlers/macro_dialog_handlers.h`
  - `setup/gui/handlers/add_macro_dialog_handlers.h`
2. Keep a short umbrella header for external inclusion if needed.
3. Ensure all exported handler symbols remain visible and match the `.ui` file
  handler names.

Exit criteria:

- `setup/signal_handlers.h` is no longer the central declaration bucket;
- each dialog has a dedicated handler header and source.

Note:

- Macro dialog UX enhancements (and their schedule) are deferred to Phase 8 so
  dialog work happens after the cleanup phases stabilize.

### Phase 5: Contain global state and narrow ownership

#### Objective

Reduce the maintenance cost of global state without forcing a destabilizing
architecture rewrite.

#### Primary targets

- `setup/main.cpp`
- `src/config/unikey_config.cpp`
- `ukengine/inputproc.cpp`
- `ukengine/unikey.cpp`

#### Implementation steps

1. In setup code, identify which globals represent UI widgets, builder state,
  settings state, and mutable workflow state.
2. Create `setup/app_context.{h,cpp}` as the narrow home for stable shared
  setup dependencies.
3. Introduce a small context object or accessor layer for setup-specific state.
4. Update callback wiring to consume that context gradually instead of replacing
  every global in one patch.
5. In config code, create `src/config/unikey_config_facade.{h,cpp}` and wrap
  singleton access behind that smaller interface before
  changing the singleton itself.
6. In engine code, isolate initialization state and access helpers around
  classifier/runtime globals.
7. Stop after containment if full elimination would require a broad API change;
  record the remaining work instead of forcing it into this cleanup.

#### Validation

- setup still initializes correctly;
- existing callers do not require a broad mechanical rewrite;
- state ownership is more explicit in touched areas.

#### Exit criteria

- direct global access is reduced in the worst hotspots;
- new code introduced in this phase does not add fresh global dependencies.

### Phase 6: Error handling, cleanup paths, and legacy condition clarity

#### Objective

Close the silent-failure gaps and bring touched legacy logic closer to current
readability standards.

#### Primary targets

- `src/config/unikey_config.cpp`
- `setup/main.cpp`
- touched portions of `ukengine/ukengine.cpp`
- touched portions of `ukengine/unikey.cpp`

#### Implementation steps

1. Handle the return values of `g_settings_set_value()` and decide whether each
  failure should be logged, surfaced, or propagated.
2. Resolve the ownership/TODO area around the current `g_signal_connect()`
  usage in config code.
3. Add explicit error checking for builder and icon loading in setup startup.
4. Clean up temporary allocations on both success and failure paths.
5. While touching these functions, replace the hardest-to-read inline boolean
  chains with descriptive intermediate booleans.
6. Rename vague touched variables to descriptive names.

#### Validation

- startup/resource-loading failure behavior is explicit and testable;
- persistence failures are no longer silently ignored;
- touched legacy conditions are easier to read without changing semantics.

#### Exit criteria

- major ignored-return hotspots are handled;
- touched startup/config functions have clear failure paths;
- the readability of touched condition-heavy code is visibly improved.

### Phase 7: Final documentation and maintenance handoff

#### Objective

Leave the cleanup in a state where a later contributor can continue without
reverse-engineering the refactor.

#### Files to update

- this plan file;
- `project_planning/UI/macros/code_structure.md` if module ownership changed;
- the relevant maintainer note(s) under `project_planning/UI/macros/`;
- any new test README files added during the cleanup.

#### Implementation steps

1. Record the final module boundaries introduced by the cleanup.
2. List any consciously deferred issues, especially remaining globals or large
  functions that were not safe to finish in this cycle.
3. Document new test targets and when they should be run.
4. Update the immediate next actions section so future work starts from the new
  reality rather than the pre-cleanup structure.

#### Exit criteria

- planning notes match the code structure that actually landed;
- a new contributor can identify where engine, setup, config, and test
  responsibilities now live.

### Phase 8 (post-cleanup): Macro dialog UX enhancements

#### Objective

Schedule and implement the macro dialog UX enhancements only after the cleanup
phases above have landed and stabilized.

#### Scope

- resizable dialogs;
- search + navigation;
- multi-select state management (Ctrl+A, Ctrl+click, Esc restore);
- focus/scroll helper;
- drag/drop reordering;
- rich-text edit dialog.

#### Dependencies

- Phase 4 (handler split completed, per-dialog `.ui` files in place);
- macro helper modules exist (`macro_model_adapter`, `macro_dialog_controller`,
  etc.) so UX logic remains thin and testable.

#### Steps

1. **8.1: Resizable dialogs**
   - Set `resizable="true"` on `macro_dialog`, `add_macro_dialog`, `main_window`
     in the `.ui` files.
   - Set reasonable `default_width="600"` and `default_height="400"` on
     `macro_dialog`.
   - Optionally add GTK window-state settings to save/restore size in GSettings.

2. **8.2: Search box and navigation UI**
   - Add a horizontal box above the `tree_macro` in `macro_dialog.ui` with:
     - GtkSearchEntry `search_entry_macro`
     - Buttons: `btn_search_first`, `btn_search_prev`, `btn_search_next`,
       `btn_search_last` (with labels or icons "First", "Prev", "Next", "Last")
   - Use `gtk_search_entry_get_text()` on text-changed signal.
   - Implement `setup/gui/macro_search.{h,cpp}` from module CUX1.

3. **8.3: Focus button**
   - Add "Focus" button to the macro dialog button box (after Add, Delete buttons).
   - Implement `setup/gui/macro_focus_helper.{h,cpp}` from module CUX3.
   - Handler `on_btn_macro_focus_clicked()` centers tree on current selection.

4. **8.4: Multi-select and Esc handling**
   - Add key-press-event handler to `tree_macro` for Ctrl+A (select all) and
     Esc (clear with anchor restoration).
   - Add button-press-event handler to `tree_macro` for Ctrl+click (toggle
     selection).
   - Implement `setup/gui/macro_selection.{h,cpp}` from module CUX2.
   - Add selection-changed signal handler to track anchor point.

5. **8.5: Edit dialog layout and rich-text editor**
   - In `add_macro_dialog.ui`:
     - Replace the `entry_macro_with` GtkEntry with a scrolled GtkTextView.
     - Change the "With:" label from `xalign="0.5"` (center) to `xalign="0.0"`
       (left-align).
     - Adjust the grid layout so the textview grows with the window.
   - Implement `setup/gui/macro_edit_dialog.{h,cpp}` from module CUX5.
   - Preserve text content when switching between add/edit modes.

6. **8.6: Drag/drop reordering**
   - Set up GTK drag-source on `tree_macro` for selected rows.
   - Set up GTK drag-dest on `tree_macro` to accept rows as drop targets.
   - Implement `setup/gui/macro_drag_drop.{h,cpp}` from module CUX4.
   - Show visual feedback (insert-line, cursor change) during drag.
   - On drop, reorder rows in list store and update the underlying CMacroTable.

7. **8.7: Test scaffolding for UX features**
   - Create `tests/setup/gui/macro_search/` with fixtures for search logic.
   - Create `tests/setup/gui/macro_selection/` for multi-select and anchor tests.
   - Create `tests/setup/gui/macro_focus/` for scroll-to-cell tests.
   - Create `tests/setup/gui/macro_drag_drop/` for reorder and visual feedback.
   - Create `tests/setup/gui/macro_edit_dialog/` for rich-text dialog behavior.
   - Hook these into the main CMakeLists.txt under tests/setup/gui/.

8. **8.8: Integration and validation**
   - Ensure new handlers are declared in the per-dialog headers from Phase 4.
   - Run full macro test suite to verify no regressions in add/edit/delete flow.
   - Test search across both columns; verify anchor restoration on Esc.
   - Test drag/drop ordering is reflected in CMacroTable on save.
   - Test keyboard shortcuts: Ctrl+A, Esc, Ctrl+click.

Schedule (after cleanup stabilizes):

- 8.1–8.3 (UI setup + search): 1–2 days.
- 8.4 (multi-select + anchor): 2–3 days.
- 8.5 (rich-text editor): 1 day.
- 8.6 (drag/drop): 3–4 days.
- 8.7 (test scaffolding): 2 days.
- 8.8 (integration and validation): 1–2 days.

Total: 10–14 days.

Exit criteria:

- All dialogs are resizable with sensible default sizes.
- Search box searches both columns; navigation buttons jump to matches.
- Focus button scrolls tree to center on selection.
- Ctrl+A selects all rows; Esc clears selection and restores first row focus.
- Ctrl+click toggles row selection for multi-select.
- Edit dialog has top-aligned label and scrollable rich-text textview.
- Drag/drop reordering works with visual feedback.
- New UX handlers are declared in per-dialog headers.
- New test modules pass; no regressions in existing macro tests.

### Recommended PR / patch sequence

For reviewability, the implementation should ideally land in this order:

1. baseline notes and `tests/ukengine` scaffolding;
2. first core regression tests;
3. `WordBreakSyms` centralization and constant cleanup;
4. first `ukengine` helper extraction patch;
5. second `ukengine` helper extraction patch or file split;
6. export-flow helper extraction in setup;
7. import/edit-flow helper cleanup in setup;
8. global-state containment patch;
9. error-handling patch;
10. documentation refresh patch.

Optional follow-up (post-cleanup):

11. macro dialog UX enhancements (Phase 8).

If a patch grows too large, split it again rather than carrying multiple
responsibilities in one review.

## Risk management

Key risks:

- accidental engine behavior changes during decomposition;
- import/export regressions while simplifying callback code;
- global-state cleanup expanding into a rewrite;
- broad style-only churn making review harder.

Mitigations:

- require tests before large refactors;
- do structural refactors in small batches;
- separate behavioral changes from mechanical extraction work;
- keep public interfaces stable until internal seams are proven;
- update documentation only after implementation boundaries settle.

## Definition of done

This mitigation effort can be considered successful when:

- duplicated constants such as `WordBreakSyms` have a single source of truth;
- `ukengine.cpp` no longer concentrates the same level of monolithic behavior in
  its largest routines;
- the setup callback layer (`setup/signal_handlers.cpp` / `setup/gui/handlers/*`)
  no longer carries multi-stage macro flows in single oversized callbacks
  (logic lives in focused helpers; UI handlers stay thin);
- key global-state access points are wrapped or narrowed;
- major legacy condition chains in touched code are expressed through named
  booleans;
- ignored persistence/resource-loading failures are explicitly handled;
- core tests exist alongside the existing UI macro tests;
- the updated structure is documented in `project_planning` notes.

## Immediate next actions

1. create `tests/ukengine` and add the first regression tests for engine
   behavior;
2. centralize `WordBreakSyms` and related shared constants;
3. extract the first internal helper set from `ukengine.cpp`;
4. decompose `on_btn_macro_export_clicked()` into focused helpers;
5. add follow-up notes once the first cleanup phase lands.
