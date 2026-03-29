# Macro editor: Drag-and-drop merge design

Date: 2026-03-08

This document formalizes the design for adding per-drop merge behaviour to the macro editor (drag‑in and drag‑out), including API changes, UI wiring, undo/redo snapshots, and verification steps. It follows the project development guidelines: modular small functions, explicit boolean naming, no magic numbers, and clear documentation.

---

## Summary

We will stop re-using the "append-only" logic for drag‑and‑drop and implement a dedicated merge behaviour for drops. The default for drag‑in will be OverwriteExisting, while the existing Import… button retains KeepExisting semantics. Modifier keys during drop (Shift / Ctrl) will switch to ReplaceAll / ReplaceSelected. A small undo/redo snapshot stack will be added for simplicity.

## Checklist (what I will implement / document here)

- [x] Define a `MergeMode` enum in `setup/macro_utils.h`.
- [x] Add `list_store_merge()` declaration and implement it in `setup/macro_utils.cpp`.
- [x] Refactor `list_store_append()` to call `list_store_merge(..., KeepExisting)`.
- [x] Add drag signal handler declarations in `setup/signal_handlers.h` and implementation notes for `signal_handlers.cpp`.
- [x] Describe UI changes for `setup/ibus-unikey.ui` (multi-select and context menu) and keyboard accelerators.
- [x] Specify a small undo helper module `setup/macro_undo.{h,cpp}` and its API.
- [x] Provide verification steps and mapping to development guidelines.

---

## 1. Merge modes (API)

Add the enum near other macro/list-store declarations in `setup/macro_utils.h`:

```c++
enum class MergeMode {
    KeepExisting,      // Import button behaviour: skip duplicates
    OverwriteExisting, // Default for drag-in: update existing keys, append new ones
    ReplaceAll,        // Clear store then append
    ReplaceSelected    // Delete selected rows then append
};
```

Rationale: keep an explicit, central enum so behaviour is discoverable and easy to extend. Place it in `macro_utils.h` so UI, signal handlers and utils can share it.

## 2. Utility routines

Declare and implement:

- `void list_store_merge(GtkListStore* store, CMacroTable* macro, MergeMode mode, GtkTreeSelection* sel = nullptr);`

Behavioral summary:

- KeepExisting: same as existing `list_store_append()` (skip duplicate keys, case-insensitive).
- OverwriteExisting: for each incoming key, find row (case-insensitive) and update its value; if not found, append.
- ReplaceAll: clear the list store, call `list_store_append_fast()` then `list_store_add_null_item()`.
- ReplaceSelected: delete rows referenced by `sel` (if non-null and has selection), then append all entries.

Implementation notes and constraints:

- Keep functions short: extract helpers for "convert CMacroTable entry to UTF-8 pair" and for "find iter by case-insensitive key".
- Use the existing CaseInsensitiveHash/Equal utilities when building temporary sets.
- Use `g_object_ref()`/`g_object_unref()` around bulk ops to prevent UI churn when appropriate.
- Preserve sentinel null row behaviour: ensure `list_store_add_null_item()` is called when needed.
- Refactor `list_store_append()` to call `list_store_merge(..., MergeMode::KeepExisting)` for DRY compliance.

## 3. Signal handlers & drag setup

Add declarations to `setup/signal_handlers.h` (or extend existing ones):

```c++
G_MODULE_EXPORT void on_tree_macro_drag_data_get(GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data, guint info, guint time, gpointer user_data);
G_MODULE_EXPORT void on_tree_macro_drag_data_received(GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint info, guint time, gpointer user_data);
G_MODULE_EXPORT gboolean on_tree_macro_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
```

Initialization (called from `on_btn_macroedit_clicked` or builder init in `setup/main.cpp`):

- `gtk_drag_source_set(tree_macro, GDK_BUTTON1_MASK, targets_for_uri_list, n_targets, GDK_ACTION_COPY | GDK_ACTION_MOVE);`
- `gtk_drag_dest_set(tree_macro, GTK_DEST_DEFAULT_ALL, targets_for_uri_list, n_targets, GDK_ACTION_COPY | GDK_ACTION_MOVE);`
- Connect `drag-data-get` -> `on_tree_macro_drag_data_get` and `drag-data-received` -> `on_tree_macro_drag_data_received`.

`on_tree_macro_drag_data_get()`:

- Determine whether to export selected rows (if any) or all rows.
- Create a temporary file (`g_file_open_tmp()` or `g_mkstemp()`) and write chosen rows using existing IO helper: `macro_table_write_any_format()` (inferred from current project code base). If needed, write in the internal text format (one `key:replacement` per line).
- Fill `selection_data` with a URI list pointing to the temp file.
- Ensure temporary files are cleaned when drag completes or on app exit (document lifecycle).

`on_tree_macro_drag_data_received()`:

- Parse the incoming URI list and read files into a `CMacroTable` via existing read helpers (or add `macro_table_read_any_format()` if missing).
- Determine `MergeMode`:
  - Default: `OverwriteExisting`.
  - If the drag event's modifier state contains `GDK_SHIFT_MASK` → `ReplaceAll`.
  - If `GDK_CONTROL_MASK` and there is a non-empty selection on the tree → `ReplaceSelected`.
  - Optionally consult a saved preference `ibus_unikey_config_get_string("macro_drag_mode")` to override.
- Call `list_store_merge(store, &incoming_macro, chosen_mode, selection)`.
- Trigger `push_undo_state()`.
- Resort / update UI counts and call `list_store_add_null_item()` as needed.
- On error, show a brief dialog describing the failure.

Edge cases and notes:

- If `ReplaceSelected` is chosen but selection is empty, fall back to `OverwriteExisting` and inform user if helpful.
- Be careful when reading URIs from remote locations — support only file:// URIs at first.

## 4. Selection, context menu & keyboard shortcuts

UI changes:

- In `setup/ibus-unikey.ui` set the tree selection mode to `multiple` for `tree_macro`, or call `gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE)` on init.
- Add (programmatically or in UI file) a context `GtkMenu` with items: `Copy`, `Cut`, `Paste`, `Delete`, separator, `Undo`, `Redo`.
- Add `on_tree_macro_button_press()` to popup the menu on right-click and map menu items to handlers.
- Register keyboard accelerators via an `GtkAccelGroup` or within the UI file:
  - `Ctrl+A` -> select all rows
  - `Ctrl+C` -> copy
  - `Ctrl+X` -> cut
  - `Ctrl+V` -> paste
  - `Ctrl+Z` / `Ctrl+Y` -> undo / redo

Copy/Paste behaviour:

- Copy writes selected rows to the clipboard using the plain-text internal format (one `key:replacement` per line).
- Paste reads from the clipboard, parses into a `CMacroTable` and calls `list_store_merge(..., MergeMode::OverwriteExisting)` for deterministic behaviour (simple rule: pastes overwrite existing keys).
- Cut = Copy then Delete selected rows (push undo between these operations appropriately).

## 5. Undo/Redo snapshot stack

Add `setup/macro_undo.h` / `setup/macro_undo.cpp` implementing a small snapshot stack API:

- `void push_undo_state(const CMacroTable& snapshot);
- `bool can_undo(); bool can_redo();
- `void apply_undo(GtkListStore* store); void apply_redo(GtkListStore* store);
- `void clear_history();`

Notes:

- Snapshots store full `CMacroTable` objects for simplicity.
- `push_undo_state()` should be called before each mutating operation (or after, depending on chosen model) — choose before change so `undo` reverts to prior state.
- `apply_undo()` / `apply_redo()` re-populate the given `GtkListStore` using `unikey_macro_to_store()`.
- Keep the module small and self-contained; avoid global state beyond the helper's internal static vector.

## 6. Documentation and comments

- Add an "Advanced editing" section to the setup README describing multi-select, context menu, keyboard shortcuts, and drag behaviour (default OverwriteExisting for drag-in and modifiers for ReplaceAll/ReplaceSelected).
- Add inline comments in `setup/signal_handlers.cpp` explaining modifier logic, MergeMode semantics, and where preference overrides are read/written.
- Create this planning file at `project_planning/UI/macros/drag_drop_merge_design.md` (this file).

## 7. File-level change plan

Implement the following per-file edits (small, focused changes):

- `setup/macro_utils.h`:
  - Add `enum class MergeMode { ... }` and declaration for `list_store_merge()`.

- `setup/macro_utils.cpp`:
  - Implement `list_store_merge()` with small helper functions for conversion and iter find.
  - Refactor `list_store_append()` to call `list_store_merge(..., MergeMode::KeepExisting)`.

- `setup/signal_handlers.h`:
  - Add drag handler declarations and new callbacks for context menu and keyboard handlers.

- `setup/signal_handlers.cpp` (or similarly named file):
  - Add initialization code to set drag-source/dest on `tree_macro` during dialog init.
  - Implement `on_tree_macro_drag_data_get()` and `on_tree_macro_drag_data_received()`.
  - Implement `on_tree_macro_button_press()` and handlers for copy/cut/paste/delete/select all.
  - Add undo/redo callbacks that call into `macro_undo` helper.

- `setup/ibus-unikey.ui`:
  - Set `GtkTreeSelection` mode to multiple (or do it programmatically) and optionally add a `GtkMenu` template.

- `setup/macro_undo.h` and `setup/macro_undo.cpp`:
  - Implement the undo snapshot stack API.

- `project_planning/UI/macros/drag_drop_merge_design.md`:
  - Add this file (done).

## 8. Verification steps (developer)

1. Build `setup/ibus-setup-unikey` using provided build system.
2. Run the setup binary and open Macro editor.
3. Verify multiple selection and keyboard shortcut `Ctrl+A`.
4. Right-click shows menu; test `Copy`, `Cut`, `Paste`, `Delete`, `Undo`, `Redo` on a subset of rows.
5. Edit several rows then drag a file into the macro list:
   - Normal drop: existing keys overwritten.
   - Hold Shift during drop: ReplaceAll (previous macros cleared, new appended).
   - Hold Ctrl with rows selected during drop: ReplaceSelected (selected rows deleted then new appended).
   - Ensure undo stack records each operation and `Undo/Redo` restores previous states.
6. Drag from the list to a file manager; open the generated file and verify selected/all entries are exported.
7. Paste from clipboard containing `a:b\nc:d` and verify behaviour (existing keys overwritten).

## 9. Mapping to development guidelines

- Modularity: each new capability (merge logic, drag handlers, undo) is implemented in a small focused file/module.
- Readable booleans & names: use `is_`, `has_` prefixes in temporary flags; name the enum `MergeMode` and helper `list_store_merge` for clarity.
- No magic numbers: use named constants and documented behavior.
- Documentation: this planning file plus inline comments in code and README updates.
- Avoid global state: `macro_undo` encapsulates its state; other state remains local or passed as parameters.

## 10. Decisions required (please confirm)

1. Implement the optional "remember this mode" dialog now? (Yes/No). If Yes — preference persisted via `ibus_unikey_config_set_string("macro_drag_mode", "OverwriteExisting" | "ReplaceAll" | ...`).
2. Should the existing Import… button adopt `OverwriteExisting` or retain `KeepExisting`? (Current recommendation: keep Import… as `KeepExisting` to preserve backwards compatibility.)

---

If you confirm the two decision points above I will produce a concrete patch plan with exact edits for each file (diff-style instructions) and then implement the textual/planning files (and — if you want — implement the code changes next).
If you want, I can add a per-file, chronological diff summary (patch-stat style) for each commit to this document; let me know if you prefer the short summary above or a verbatim commit-by-commit diff included inline (longer).
