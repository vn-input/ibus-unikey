# Formal design — macro editor improvements and code comments

Document status: Landed design snapshot
Primary branch context: `feature/macro-unlimited-crud-sort`
Companion maintainer note: `project_planning/UI/macros/20260301_170054_macros_improvements_and_code_comments.md`
Related future-work design: `project_planning/UI/macros/20260308_203348_drag_drop_merge_design.md`

## 1. Purpose

This document formalizes the shipped macro-editor changes that landed on
`feature/macro-unlimited-crud-sort`.

It is not a proposal for future work. It documents the current implementation so
another developer can:
- understand the architecture and ownership boundaries,
- recreate the same changes in another branch or port,
- preserve the current invariants when extending the subsystem, and
- distinguish landed behavior from future design work.

## 2. Problem statement

The historical macro editor was too small in scope for the current needs of the
setup application. The landed work addressed four practical problems:

1. Macro storage had to support larger data sets without the old fixed-size
   assumptions.
2. The setup dialog needed complete CRUD behavior instead of acting as a thin
   wrapper around a file.
3. Import/export had to support more than the native UniKey format.
4. The code path across UI, model sync, storage, and charset conversion needed
   clearer comments so maintainers could safely extend it.

## 3. Goals

The shipped design achieves these goals:

- support effectively unlimited macro counts through dynamic `CMacroTable`
  storage,
- make the setup dialog the main editable macro surface,
- support add/delete/clear/import/export/sort/count workflows in the UI,
- preserve runtime lookup performance by keeping the engine table sorted,
- normalize duplicate keys deterministically in the engine,
- provide format adapters for external macro file formats, and
- improve code-level documentation around the macro path.

## 4. Non-goals

These items are explicitly **not** part of the landed implementation documented
here:

- drag-and-drop import/export,
- undo/redo,
- multi-selection context menus,
- explicit overwrite-vs-replace merge modes in the setup list store,
- a persistent preference for drag/drop merge behavior.

Those are tracked separately in:
- `project_planning/UI/macros/20260308_203348_drag_drop_merge_design.md`

## 5. High-level architecture

The shipped implementation is split into five layers.

### 5.1 Durable macro storage

Files:
- `ukengine/mactab.h`
- `ukengine/mactab.cpp`

Responsibility:
- Own all macro entries in memory.
- Load and write the canonical UniKey macro file.
- Keep entries sorted for lookup.
- Remove duplicates after load, keeping the last occurrence.

Primary type:
- `CMacroTable`

### 5.2 UI projection and synchronization

Files:
- `setup/macro_utils.h`
- `setup/macro_utils.cpp`

Responsibility:
- Convert `CMacroTable` data into a `GtkListStore` for editing.
- Convert edited `GtkListStore` rows back into `CMacroTable`.
- Preserve case-insensitive uniqueness in the UI append path.
- Maintain the trailing sentinel row `STR_NULL_ITEM` used as the add-entry
  affordance in the tree view.

### 5.3 Dialog behavior and user actions

Files:
- `setup/signal_handlers.h`
- `setup/signal_handlers.cpp`
- `setup/main.cpp`
- `setup/ibus-unikey.ui`

Responsibility:
- Load macro data when the editor opens.
- Support add, edit, delete, clear, import, export, sort, and count.
- Persist the edited table only when the dialog returns `GTK_RESPONSE_OK`.

### 5.4 External format adapters

Files:
- `setup/macro_file_io.h`
- `setup/macro_file_io.cpp`

Responsibility:
- Load macros from multiple file formats into `CMacroTable`.
- Write `CMacroTable` data back to multiple formats.
- Centralize format-specific parsing and error reporting.

### 5.5 Validation tools

Files:
- `tools/macro_probe.cpp`
- `tools/stress_macro_test.sh`

Responsibility:
- Provide lightweight inspection and stress validation of macro files and large
  macro tables.

## 6. End-to-end data flow

### 6.1 Open editor

Entry point:
- `setup/signal_handlers.cpp` → `on_btn_macroedit_clicked()`

Flow:
1. Resolve the user macro file path with `get_macro_file()`.
2. Create and initialize `CMacroTable`.
3. Load persisted macros via `CMacroTable::loadFromFile()`.
4. Obtain the tree model from `tree_macro`.
5. Temporarily detach the model from the view to avoid per-row redraw costs.
6. Populate the `GtkListStore` through `unikey_macro_to_store()`.
7. Reattach the model.
8. Sort by key ascending.
9. Refresh column header indicators and macro count.
10. Run the dialog.

### 6.2 Edit in the dialog

Supported landed edit operations:
- add via shared editor dialog (`on_btn_macro_add_clicked()`)
- edit via row activation / double-click / Enter (`on_macro_row_activated()`)
- delete selected row (`on_btn_macro_del_clicked()`)
- clear all (`on_btn_macro_clear_clicked()`)
- sort by key/value column header (`on_column_key_clicked()`,
  `on_column_value_clicked()`)
- import file (`on_btn_macro_import_clicked()`)
- export file (`on_btn_macro_export_clicked()`)

### 6.3 Save on confirm

When the dialog returns `GTK_RESPONSE_OK`:
1. Convert visible rows back into `CMacroTable` using `unikey_store_to_macro()`.
2. Ensure the destination directory exists.
3. Persist with `CMacroTable::writeToFile()`.

If the dialog is cancelled or hidden:
- no persistence occurs.

## 7. Detailed design by subsystem

## 7.1 `CMacroTable` storage redesign
Primary implementation commit:
- `e70a2c24bfff16090e755de0744577fd5c593510`

Files changed:
- `ukengine/mactab.h`
- `ukengine/mactab.cpp`

### Design intent

Move macro storage away from historical fixed-size assumptions and keep the
engine table suitable for fast lookup even with large macro sets.

### Landed behavior

- Storage grows dynamically through internal capacity helpers.
- Macro replacement text capacity is raised to `MAX_MACRO_TEXT_LEN = 8192` for
  larger corporate-style expansions.
- `loadFromFile()` clears existing state, parses lines, and converts input into
  internal encoding.
- After reading, the table is sorted with `qsort()`.
- `removeDuplicates()` collapses adjacent duplicate keys and keeps the later
  definition.
- `writeToFile()` writes canonical UTF-8 `key:value` pairs.

### Important shipped rule

Duplicate keys in the engine behave with **dictionary semantics**:
- same key appearing multiple times in the source file,
- later occurrence overrides earlier occurrence,
- runtime lookup sees only the final definition.

This rule is implemented by:
- `CMacroTable::loadFromFile()`
- `CMacroTable::removeDuplicates()`

### Why this structure was chosen

- runtime macro lookup requires sorted data,
- large tables require dynamic allocation rather than a hard ceiling,
- duplicate normalization must happen at the engine level so persisted files and
  runtime lookup agree.

## 7.2 `GtkListStore` synchronization helpers
Primary implementation commit:
- `e70a2c24bfff16090e755de0744577fd5c593510`

Files changed:
- `setup/macro_utils.h`
- `setup/macro_utils.cpp`

### Design intent

Introduce a dedicated synchronization layer between `CMacroTable` and GTK so
UI code does not duplicate conversion, duplicate checking, or sentinel-row
handling.

### Landed behavior

- `list_store_check_exists()` performs case-insensitive duplicate detection in
  the UI model.
- `list_store_append_fast()` appends rows to an empty model without duplicate
  checks.
- `list_store_append()` appends into a non-empty model while skipping duplicate
  keys using a case-insensitive `std::unordered_set`.
- `unikey_macro_to_store()` clears the store, appends all macros, and restores
  the sentinel row.
- `unikey_store_to_macro()` rebuilds `CMacroTable` from visible rows while
  skipping the sentinel row.
- `list_store_add_null_item()` guarantees that the final row is the add-entry
  placeholder.

### Important shipped rule

The setup UI now has two distinct duplicate policies:
- dialog add/edit save uses **dictionary/upsert semantics**,
- import append uses **KeepExisting** semantics,
- all duplicate checks remain case-insensitive.

### Why this structure was chosen

- UI editing needs a stable placeholder row for row-activated creation.
- Conversion between internal encoding and UTF-8 should live in one place.
- Import into an existing UI list should not silently overwrite current visible
  edits in the landed implementation.

## 7.3 Setup dialog behavior
Primary implementation commits:
- `e70a2c24bfff16090e755de0744577fd5c593510`
- `c63c7d9ca15bd7470ade4e3254ade23b27fb0bf5`

Files changed:
- `setup/main.cpp`
- `setup/signal_handlers.h`
- `setup/signal_handlers.cpp`
- `setup/ibus-unikey.ui`

### Design intent

Make the setup dialog the primary user-facing macro editor while preserving good
performance on large macro sets.

### Landed behavior

#### Open/load/save
- `on_btn_macroedit_clicked()` now owns the dialog load-edit-save cycle.
- The tree model is detached during bulk loads and reattached afterward.
- Initial sort is by key ascending.
- Macro count label is updated after load and after mutating actions.

#### Add/delete/clear
- Add and edit both use the same modal dialog.
- Saving a dialog entry behaves like a dictionary upsert: the edited or newly
  entered key replaces any existing visible row with the same case-insensitive
  key.
- After save, the modified entry is reselected and scrolled into view.
- Delete removes the selected row unless it is the sentinel.
- Clear resets the store to a single sentinel row.

#### Editing interaction
- Double-clicking a row or pressing Enter opens the shared Add/Edit dialog.
- The trailing sentinel row opens that same dialog in add mode.
- Inline cell editing is disabled both in the UI definition and in widget
  initialization code.
- The replacement field in the dialog is a scrollable multi-line text view
  rather than a single-line entry.

#### Sorting
- Sorting is handled with `GtkTreeSortable` and a custom comparison function.
- The sentinel row is always forced to the end.
- Column headers show sort indicators and preserve active sort state.

#### Import/export
- Import uses file chooser + `macro_table_load_any_format()`.
- Import temporarily removes the sentinel row, appends imported macros, then
  restores the sentinel row.
- Export rebuilds a temporary `CMacroTable` from visible rows and routes through
  `macro_table_write_any_format()`.

### Why this structure was chosen

- bulk load must avoid per-row redraw overhead,
- the UI must keep the placeholder row invariant even after sort/import/clear,
- file-format handling should remain outside the widget callbacks.

## 7.4 External format support
Primary implementation commit:
- `c63c7d9ca15bd7470ade4e3254ade23b27fb0bf5`

Files changed:
- `setup/macro_file_io.h`
- `setup/macro_file_io.cpp`

### Design intent

Support macro import/export beyond the native UniKey format without changing the
engine storage API.

### Landed behavior

The file-I/O adapter supports:
- native UniKey macro files,
- Espanso-style YAML,
- macOS text substitutions plist.

Public API:
- `macro_table_load_any_format(const char* filename, CMacroTable* macro, GError** error)`
- `macro_table_write_any_format(const char* filename, CMacroTable* macro, GError** error)`

Format selection on write is based on filename extension:
- `.yml` / `.yaml` → YAML
- `.plist` → Apple plist
- otherwise → native UniKey format

### Why this structure was chosen

- the setup UI needs import/export workflows that are independent of the engine
  on-disk format,
- external format support belongs in one adapter layer instead of spreading
  parsers across signal handlers.

## 7.5 Build and tooling integration
Primary implementation commits:
- `e70a2c24bfff16090e755de0744577fd5c593510`
- `c63c7d9ca15bd7470ade4e3254ade23b27fb0bf5`

Files changed:
- `setup/CMakeLists.txt`
- `tools/macro_probe.cpp`
- `tools/stress_macro_test.sh`

### Landed behavior

`setup/CMakeLists.txt` builds the setup binary from:
- `main.cpp`
- `signal_handlers.cpp`
- `macro_utils.cpp`
- `macro_file_io.cpp`

Installed UI resource:
- `setup/ibus-unikey.ui`

Tooling added for validation:
- `tools/macro_probe.cpp`
- `tools/stress_macro_test.sh`

### Why this matters

These files are part of the landed design, not optional examples. They are the
expected validation hooks when reviewing changes in macro import/export or large
macro-file behavior.

## 8. Duplicate-resolution matrix

The landed implementation intentionally contains two duplicate policies.

### 8.1 Engine / persisted file policy

Layer:
- `CMacroTable`

Semantics:
- later definition wins,
- duplicates are removed after sorting,
- runtime macro expansion uses the surviving final definition.

Primary code:
- `ukengine/mactab.cpp` → `loadFromFile()`
- `ukengine/mactab.cpp` → `removeDuplicates()`

### 8.2 Setup UI save/import policies

Layer:
- `GtkListStore` dialog-save and import paths

Semantics:
- dialog save: last saved visible value wins for that key,
- import append: existing visible keys win,
- duplicate detection is case-insensitive in both paths.

Primary code:
- `setup/signal_handlers.cpp` → `save_macro_entry()`
- `setup/signal_handlers.cpp` → `on_btn_macro_add_clicked()`
- `setup/signal_handlers.cpp` → `on_macro_row_activated()`
- `setup/macro_utils.cpp` → `list_store_append()`
- `setup/macro_utils.cpp` → `list_store_check_exists()`
- `setup/signal_handlers.cpp` → `on_btn_macro_import_clicked()`

### 8.3 Design consequence

Developers must decide which policy they want before adding a new ingest path.
Current shipped behavior is:
- full engine load → last occurrence wins,
- dialog save into setup list → saved entry wins,
- import append into existing setup list → keep existing entry.

Future merge-mode work should not be mixed into this landed design document.
That work lives in the separate drag/drop design note.

## 9. Invariants to preserve

Any recreation or extension of this implementation must preserve these rules:

1. `CMacroTable` remains the durable source of truth.
2. `GtkListStore` is only the editable UI projection.
3. The final row in the tree store is always `STR_NULL_ITEM`.
4. Duplicate checks in the setup UI are case-insensitive.
5. Engine lookup data stays sorted before runtime use.
6. Macro count excludes the sentinel row.
7. Sorting must never move the sentinel row above real entries.
8. UTF-8 is used for UI display and persisted export output.
9. Save happens only when the dialog is accepted.
10. Bulk UI rebuilds detach the model during mass updates.
11. Double-click and Enter open the shared Add/Edit dialog, not inline cell
  editing.
12. The dialog replacement control must support large multi-line text.

## 10. File-by-file recreation checklist

Use this section to recreate the landed feature set in another branch.

### 10.1 Storage layer

Update `ukengine/mactab.h` and `ukengine/mactab.cpp` to:
- use dynamic allocation helpers for macro entries and backing memory,
- remove fixed macro-count limits,
- allow larger macro replacement bodies,
- sort loaded macros after file read,
- implement duplicate collapse with last-occurrence-wins semantics,
- keep `writeToFile()` emitting canonical UTF-8 `key:value` lines.

### 10.2 Setup model helpers

Update `setup/macro_utils.h` and `setup/macro_utils.cpp` to:
- define macro store columns and `STR_NULL_ITEM`,
- provide `CMacroTable` ↔ `GtkListStore` conversion helpers,
- add case-insensitive duplicate checking for the UI list,
- use fast append for empty-store rebuilds,
- use duplicate-skipping append for imports into a populated list,
- always restore the sentinel row after rebuild/import/save.

### 10.3 Setup dialog flow

Update `setup/main.cpp`, `setup/signal_handlers.h`, and
`setup/signal_handlers.cpp` to:
- load the macro dialog widgets from `setup/ibus-unikey.ui`,
- expose `dlgMacro`, `tree_macro`, and `builder` for signal handlers,
- implement load-edit-save logic in `on_btn_macroedit_clicked()`,
- add handlers for shared dialog add/edit/delete/clear/import/export,
- add custom sortable behavior that keeps the sentinel last,
- maintain a visible macro count label,
- force cell renderers into inert mode so row activation opens the dialog.

### 10.4 UI definition

Update `setup/ibus-unikey.ui` to:
- include the macro dialog,
- include the add-macro subdialog,
- expose named widgets expected by signal handlers,
- use a shared Save action for add/edit,
- use a scrollable multi-line replacement text control,
- provide clickable key/value columns for sorting,
- include a label for macro count,
- wire row activation to open the shared dialog.

### 10.5 Format adapters

Add `setup/macro_file_io.h` and `setup/macro_file_io.cpp` to:
- read UniKey native files,
- read Espanso YAML,
- read macOS plist text substitutions,
- write the same formats through one public API,
- surface failures through `GError**`.

### 10.6 Build and tooling

Update `setup/CMakeLists.txt` to include `macro_file_io.cpp` in the setup binary.
Add tooling files:
- `tools/macro_probe.cpp`
- `tools/stress_macro_test.sh`

### 10.7 Documentation

Update these docs to make the shipped structure understandable:
- `README.md`
- `ukengine/README`
- macro-related headers and source comments

## 11. Physical code delta against `master`

This repository uses `master` as the baseline branch.
The excerpts below are curated from:
- `master..feature/macro-unlimited-crud-sort`

Rules used in this appendix:
- only macro-related files are included,
- only real changed lines are shown,
- no comments are included inside diff blocks,
- no placeholder `...` lines are used.

### 11.1 Build wiring — `setup/CMakeLists.txt`
Primary implementation commit: `c63c7d9ca15bd7470ade4e3254ade23b27fb0bf5`

```diff
 add_executable(ibus-setup-unikey
     main.cpp
     signal_handlers.cpp
     macro_utils.cpp
+    macro_file_io.cpp
 )
+target_link_options(ibus-setup-unikey PRIVATE -rdynamic)
```

### 11.2 Dynamic storage contract — `ukengine/mactab.h`
Primary implementation commit: `e70a2c24bfff16090e755de0744577fd5c593510`

```diff
-    MacroDef m_table[MAX_MACRO_ITEMS];
-    char m_macroMem[MACRO_MEM_SIZE];
-    int m_count;
-    int m_memSize, m_occupied;
+    bool ensureItemCapacity(int minCapacity);
+    bool ensureMemCapacity(int minCapacity);
+    void removeDuplicates();
+    MacroDef *m_table;
+    char *m_macroMem;
+    int m_count;
+    int m_itemCapacity;
+    int m_memSize, m_occupied;
```

### 11.3 Dynamic allocation and dedupe — `ukengine/mactab.cpp`
Primary implementation commit: `e70a2c24bfff16090e755de0744577fd5c593510`

```diff
+#define INITIAL_MACRO_ITEMS_CAPACITY 128
+#define INITIAL_MACRO_MEM_SIZE (1024*128)
+
+CMacroTable::CMacroTable()
+{
+  m_table = NULL;
+  m_macroMem = NULL;
+  m_count = 0;
+  m_itemCapacity = 0;
+  m_memSize = 0;
+  m_occupied = 0;
+}
+
+CMacroTable::~CMacroTable()
+{
+  if (m_table) {
+    free(m_table);
+    m_table = NULL;
+  }
+  if (m_macroMem) {
+    free(m_macroMem);
+    m_macroMem = NULL;
+  }
+}
```

```diff
+bool CMacroTable::ensureItemCapacity(int minCapacity)
+{
+  if (minCapacity <= m_itemCapacity)
+    return true;
+
+  int newCapacity = (m_itemCapacity > 0) ? m_itemCapacity : INITIAL_MACRO_ITEMS_CAPACITY;
+  while (newCapacity < minCapacity) {
+    if (newCapacity > (1 << 29))
+      return false;
+    newCapacity *= 2;
+  }
+
+  MacroDef *newTable = (MacroDef *)realloc(m_table, sizeof(MacroDef) * newCapacity);
+  if (!newTable)
+    return false;
+
+  m_table = newTable;
+  m_itemCapacity = newCapacity;
+  return true;
+}
+
+bool CMacroTable::ensureMemCapacity(int minCapacity)
+{
+  if (minCapacity <= m_memSize)
+    return true;
+
+  int newSize = (m_memSize > 0) ? m_memSize : INITIAL_MACRO_MEM_SIZE;
+  while (newSize < minCapacity) {
+    if (newSize > (1 << 29))
+      return false;
+    newSize *= 2;
+  }
+
+  char *newMem = (char *)realloc(m_macroMem, newSize);
+  if (!newMem)
+    return false;
+
+  m_macroMem = newMem;
+  m_memSize = newSize;
+  return true;
+}
```

```diff
 void CMacroTable::init()
 {
-  m_memSize = MACRO_MEM_SIZE;
+  if (!m_table) {
+    ensureItemCapacity(INITIAL_MACRO_ITEMS_CAPACITY);
+  }
+  if (!m_macroMem) {
+    ensureMemCapacity(INITIAL_MACRO_MEM_SIZE);
+  }
   m_count = 0;
   m_occupied = 0;
 }
```

```diff
   MacCompareStartMem = m_macroMem;
   qsort(m_table, m_count, sizeof(MacroDef), macCompare);
+  removeDuplicates();
   if (version != UKMACRO_VERSION_UTF8) {
     writeToFile(fname);
   }
```

```diff
+void CMacroTable::removeDuplicates()
+{
+    if (m_count <= 1)
+        return;
+
+    MacCompareStartMem = m_macroMem;
+    int writeIdx = 0;
+    for (int readIdx = 1; readIdx < m_count; readIdx++) {
+        if (macCompare(&m_table[writeIdx], &m_table[readIdx]) != 0) {
+            writeIdx++;
+            if (writeIdx != readIdx) {
+                m_table[writeIdx] = m_table[readIdx];
+            }
+        } else {
+            if (m_table[readIdx].keyOffset > m_table[writeIdx].keyOffset) {
+                m_table[writeIdx] = m_table[readIdx];
+            }
+        }
+    }
+    m_count = writeIdx + 1;
+}
```

```diff
 int CMacroTable::addItem(const void *key, const void *text, int charset)
 {
-  char *p = m_macroMem + offset;
-
-  if (m_count >= MAX_MACRO_ITEMS)
+  char *p;
+
+  if (!ensureItemCapacity(m_count + 1))
     return -1;
+
+  int needed = offset + MAX_MACRO_KEY_LEN * (int)sizeof(StdVnChar) +
+               MAX_MACRO_TEXT_LEN * (int)sizeof(StdVnChar);
+  if (!ensureMemCapacity(needed))
+    return -1;
+
+  p = m_macroMem + offset;
```

### 11.4 New synchronization API — `setup/macro_utils.h`
Primary implementation commit: `e70a2c24bfff16090e755de0744577fd5c593510`

```diff
+#include <gtk/gtk.h>
+#include "mactab.h"
+
+enum {COL_KEY = 0, COL_VALUE};
+
+#define STR_NULL_ITEM "..."
+
+gboolean list_store_check_exists(GtkListStore* store, gchar* check_key);
+void list_store_add_null_item(GtkListStore* list);
+void list_store_append_fast(GtkListStore* list, CMacroTable* macro);
+void list_store_append(GtkListStore* list, CMacroTable* macro);
+void unikey_macro_to_store(CMacroTable* macro, GtkListStore* store);
+void unikey_store_to_macro(GtkListStore* store, CMacroTable* macro);
```

### 11.5 Faster rebuilds and UI-side dedupe — `setup/macro_utils.cpp`
Primary implementation commit: `e70a2c24bfff16090e755de0744577fd5c593510`

```diff
+#include <unordered_set>
+#include <string>
+
+struct CaseInsensitiveHash {
+    size_t operator()(const std::string& s) const {
+        size_t h = 0;
+        for (char c : s) h = h * 31 + tolower((unsigned char)c);
+        return h;
+    }
+};
+
+struct CaseInsensitiveEqual {
+    bool operator()(const std::string& a, const std::string& b) const {
+        return strcasecmp(a.c_str(), b.c_str()) == 0;
+    }
+};
```

```diff
+void list_store_append_fast(GtkListStore* list, CMacroTable* macro)
+{
+    gchar key[MAX_MACRO_KEY_LEN*3];
+    gchar value[MAX_MACRO_TEXT_LEN*3];
+    UKBYTE* p;
+    GtkTreeIter iter;
+
+    for (int i = 0 ; i < macro->getCount(); i++)
+    {
+        p = (UKBYTE*)macro->getKey(i);
+        int inLen = -1;
+        int maxOutLen = sizeof(key);
+        int ret = VnConvert(CONV_CHARSET_VNSTANDARD, CONV_CHARSET_XUTF8,
+                        p, (UKBYTE*)key,
+                        &inLen, &maxOutLen);
+        if (ret != 0)
+            continue;
+
+        p = (UKBYTE*)macro->getText(i);
+        inLen = -1;
+        maxOutLen = sizeof(value);
+        ret = VnConvert(CONV_CHARSET_VNSTANDARD, CONV_CHARSET_XUTF8,
+                        p, (UKBYTE*)value,
+                        &inLen, &maxOutLen);
+        if (ret != 0)
+            continue;
+
+        gtk_list_store_append(list, &iter);
+        gtk_list_store_set(list, &iter, COL_KEY, key, COL_VALUE, value, -1);
+    }
+}
```

```diff
 void list_store_append(GtkListStore* list, CMacroTable* macro)
 {
+    std::unordered_set<std::string, CaseInsensitiveHash, CaseInsensitiveEqual> seen;
+    GtkTreeIter iter;
+    auto model = GTK_TREE_MODEL(list);
+    if (gtk_tree_model_get_iter_first(model, &iter)) {
+        do {
+            gchar* existing_key;
+            gtk_tree_model_get(model, &iter, COL_KEY, &existing_key, -1);
+            if (existing_key) {
+                seen.insert(existing_key);
+                g_free(existing_key);
+            }
+        } while (gtk_tree_model_iter_next(model, &iter));
+    }
```

```diff
-        if (list_store_check_exists(list, key))
+        if (seen.count(key))
             continue;
+        seen.insert(key);
```

```diff
 void unikey_macro_to_store(CMacroTable* macro, GtkListStore* store)
 {
+    g_object_ref(store);
     gtk_list_store_clear(store);
-    list_store_append(store, macro);
+    list_store_append_fast(store, macro);
     list_store_add_null_item(store);
+    g_object_unref(store);
 }
```

### 11.6 Setup bootstrap changes — `setup/main.cpp`
Primary implementation commit: `e70a2c24bfff16090e755de0744577fd5c593510`

```diff
-GtkWidget* mwin;
-GtkWidget* dlgMacro;
-GtkTreeView* tree_macro;
+GtkWidget* mwin;
+GtkWidget* dlgMacro;
+GtkTreeView* tree_macro;
+GtkBuilder* builder;
```

```diff
 void init_gtk_builder()
 {
-    GtkBuilder* builder = gtk_builder_new();
-    gtk_builder_add_from_file(builder, PKGDATADIR "/ui/ibus-unikey.ui", NULL);
+    builder = gtk_builder_new();
+    const gchar *override_dir_2 = g_getenv("IBUS_UNIKEY_PKGDATADIR");
+    const gchar *data_dir2 = override_dir_2 ? override_dir_2 : PKGDATADIR;
+    gtk_builder_add_from_file(builder, g_strconcat(data_dir2, "/ui/ibus-unikey.ui", NULL), NULL);
```

```diff
-    g_object_unref(builder);
```

```diff
-    gtk_window_set_default_icon_from_file(PKGDATADIR "/icons/ibus-unikey.svg", NULL);
+    const gchar *override_dir = g_getenv("IBUS_UNIKEY_PKGDATADIR");
+    const gchar *data_dir = override_dir ? override_dir : PKGDATADIR;
+    gtk_window_set_default_icon_from_file(g_strconcat(data_dir, "/icons/ibus-unikey.svg", NULL), NULL);
```

### 11.7 Multi-format I/O API — `setup/macro_file_io.h`
Primary implementation commit: `c63c7d9ca15bd7470ade4e3254ade23b27fb0bf5`

```diff
+#include <glib.h>
+#include "mactab.h"
+
+bool macro_table_load_any_format(const char* filename, CMacroTable* macro, GError** error);
+bool macro_table_write_any_format(const char* filename, CMacroTable* macro, GError** error);
```

### 11.8 Multi-format load/write implementation — `setup/macro_file_io.cpp`
Primary implementation commit: `c63c7d9ca15bd7470ade4e3254ade23b27fb0bf5`

```diff
+enum class MacroFileFormat {
+    NATIVE,
+    YAML,
+    PLIST,
+};
+
+struct MacroFileEntry {
+    std::string trigger;
+    std::string replace;
+};
```

```diff
+static MacroFileFormat detect_macro_file_format(const char* filename)
+{
+    const std::string lower_name = lowercase_ascii(filename);
+    if (lower_name.size() >= 5 && lower_name.compare(lower_name.size() - 5, 5, ".yaml") == 0) {
+        return MacroFileFormat::YAML;
+    }
+    if (lower_name.size() >= 4 && lower_name.compare(lower_name.size() - 4, 4, ".yml") == 0) {
+        return MacroFileFormat::YAML;
+    }
+    if (lower_name.size() >= 6 && lower_name.compare(lower_name.size() - 6, 6, ".plist") == 0) {
+        return MacroFileFormat::PLIST;
+    }
+    return MacroFileFormat::NATIVE;
+}
```

```diff
+bool macro_table_load_any_format(const char* filename, CMacroTable* macro, GError** error)
+{
+    if (macro == NULL || filename == NULL) {
+        g_set_error_literal(error,
+                            macro_file_error_quark(),
+                            1,
+                            "Macro import received an invalid file path.");
+        return false;
+    }
+
+    macro->init();
+    switch (detect_macro_file_format(filename)) {
+        case MacroFileFormat::YAML:
+            return load_yaml_file(filename, macro, error);
+        case MacroFileFormat::PLIST:
+            return load_plist_file(filename, macro, error);
+        case MacroFileFormat::NATIVE:
+        default:
+            if (macro->loadFromFile(filename)) {
+                return true;
+            }
+            g_set_error(error,
+                        macro_file_error_quark(),
+                        1,
+                        "Failed to open macro file '%s'.",
+                        filename);
+            return false;
+    }
+}
```

```diff
+bool macro_table_write_any_format(const char* filename, CMacroTable* macro, GError** error)
+{
+    if (macro == NULL || filename == NULL) {
+        g_set_error_literal(error,
+                            macro_file_error_quark(),
+                            1,
+                            "Macro export received an invalid file path.");
+        return false;
+    }
+
+    switch (detect_macro_file_format(filename)) {
+        case MacroFileFormat::YAML: {
+            std::string content;
+            if (!build_yaml_content(macro, content, error)) {
+                return false;
+            }
+            return g_file_set_contents(filename, content.c_str(), content.size(), error);
+        }
+        case MacroFileFormat::PLIST: {
+            std::string content;
+            if (!build_plist_content(macro, content, error)) {
+                return false;
+            }
+            return g_file_set_contents(filename, content.c_str(), content.size(), error);
+        }
+        case MacroFileFormat::NATIVE:
+        default:
+            if (macro->writeToFile(filename)) {
+                return true;
+            }
+            g_set_error(error,
+                        macro_file_error_quark(),
+                        1,
+                        "Failed to write macro file '%s'.",
+                        filename);
+            return false;
+    }
+}
```

### 11.9 Handler wiring and UX behavior — `setup/signal_handlers.cpp`
Primary implementation commits:
- `e70a2c24bfff16090e755de0744577fd5c593510`
- `c63c7d9ca15bd7470ade4e3254ade23b27fb0bf5`

```diff
+#include "macro_file_io.h"
+
+extern GtkBuilder* builder;
+
+static GtkSortType sort_key_order = GTK_SORT_ASCENDING;
+static GtkSortType sort_value_order = GTK_SORT_ASCENDING;
+static int current_sort_column = COL_KEY;
```

```diff
+static void update_macro_count_label(void)
+{
+    GtkLabel* lbl = GTK_LABEL(gtk_builder_get_object(builder, "lbl_macro_count"));
+    if (lbl == NULL) return;
+
+    auto model = gtk_tree_view_get_model(tree_macro);
+    if (model == NULL) return;
+
+    int count = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model), NULL);
+    if (count > 0) count--;
+
+    gchar* text = g_strdup_printf(ngettext("%d macro", "%d macros", count), count);
+    gtk_label_set_text(lbl, text);
+    g_free(text);
+}
```

```diff
+static int compare_store_items(GtkTreeModel* model,
+                               GtkTreeIter* a,
+                               GtkTreeIter* b,
+                               gpointer user_data)
+{
+    gchar *key_a, *key_b;
+    int col = GPOINTER_TO_INT(user_data);
+    gtk_tree_model_get(model, a, col, &key_a, -1);
+    gtk_tree_model_get(model, b, col, &key_b, -1);
+
+    if (g_strcmp0(key_a, STR_NULL_ITEM) == 0) {
+        g_free(key_a);
+        g_free(key_b);
+        return 1;
+    }
+    if (g_strcmp0(key_b, STR_NULL_ITEM) == 0) {
+        g_free(key_a);
+        g_free(key_b);
+        return -1;
+    }
+
+    int result = g_utf8_collate(key_a ? key_a : "", key_b ? key_b : "");
+    g_free(key_a);
+    g_free(key_b);
+    return result;
+}
+
+static void sort_list_store(int column, GtkSortType order)
+{
+    auto store = GTK_LIST_STORE(gtk_tree_view_get_model(tree_macro));
+    auto sortable = GTK_TREE_SORTABLE(store);
+    gtk_tree_sortable_set_sort_func(sortable, column, compare_store_items, GINT_TO_POINTER(column), NULL);
+    gtk_tree_sortable_set_sort_column_id(sortable, column, order);
+    current_sort_column = column;
+}
```

```diff
     auto store = GTK_LIST_STORE(gtk_tree_view_get_model(tree_macro));
+
+    g_object_ref(store);
+    gtk_tree_view_set_model(tree_macro, NULL);
 
     unikey_macro_to_store(&macro, store);
+
+    gtk_tree_view_set_model(tree_macro, GTK_TREE_MODEL(store));
+    g_object_unref(store);
+
+    sort_list_store(COL_KEY, GTK_SORT_ASCENDING);
+    sort_key_order = GTK_SORT_ASCENDING;
+    current_sort_column = COL_KEY;
+    update_column_headers();
+    update_macro_count_label();
```

```diff
+    add_macro_file_filters(GTK_FILE_CHOOSER(file));
```

```diff
-        macro.init();
-        macro.loadFromFile(fn);
+        GError* error = NULL;
+        const gboolean is_loaded = macro_table_load_any_format(fn, &macro, &error);
+        if (!is_loaded)
+        {
+            show_macro_file_error_dialog(_("Failed to import macros."), error);
+            g_free(fn);
+            gtk_widget_destroy(file);
+            return;
+        }
```

```diff
+        g_object_ref(store);
+        gtk_tree_view_set_model(tree_macro, NULL);
   ...
+        gtk_tree_view_set_model(tree_macro, GTK_TREE_MODEL(store));
+        g_object_unref(store);
+
+        sort_list_store(current_sort_column,
+            current_sort_column == COL_KEY ? sort_key_order : sort_value_order);
+        update_macro_count_label();
```

```diff
+    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(file), TRUE);
+    add_macro_file_filters(GTK_FILE_CHOOSER(file));
   ...
-        macro.writeToFile(fn);
+        GError* error = NULL;
+        if (!macro_table_write_any_format(fn, &macro, &error))
+        {
+            show_macro_file_error_dialog(_("Failed to export macros."), error);
+            g_free(fn);
+            gtk_widget_destroy(file);
+            return;
+        }
```

```diff
+void on_column_key_clicked(GtkTreeViewColumn* column, gpointer user_data)
+{
+    if (current_sort_column == COL_KEY) {
+        sort_key_order = (sort_key_order == GTK_SORT_ASCENDING) ? GTK_SORT_DESCENDING : GTK_SORT_ASCENDING;
+    } else {
+        sort_key_order = GTK_SORT_ASCENDING;
+    }
+    sort_list_store(COL_KEY, sort_key_order);
+    update_column_headers();
+    scroll_to_selection_or_top();
+}
+
+void on_column_value_clicked(GtkTreeViewColumn* column, gpointer user_data)
+{
+    if (current_sort_column == COL_VALUE) {
+        sort_value_order = (sort_value_order == GTK_SORT_ASCENDING) ? GTK_SORT_DESCENDING : GTK_SORT_ASCENDING;
+    } else {
+        sort_value_order = GTK_SORT_ASCENDING;
+    }
+    sort_list_store(COL_VALUE, sort_value_order);
+    update_column_headers();
+    scroll_to_selection_or_top();
+}
```

```diff
+void on_btn_macro_add_clicked(GtkButton* button, gpointer user_data)
+{
+    GtkDialog* add_dialog = GTK_DIALOG(gtk_builder_get_object(builder, "add_macro_dialog"));
+    GtkEntry* entry_key = GTK_ENTRY(gtk_builder_get_object(builder, "entry_macro_key"));
+    GtkEntry* entry_value = GTK_ENTRY(gtk_builder_get_object(builder, "entry_macro_value"));
+
+    if (add_dialog == NULL || entry_key == NULL || entry_value == NULL) {
+        g_warning("Add macro dialog widgets not found");
+        return;
+    }
+
+    gtk_entry_set_text(entry_key, "");
+    gtk_entry_set_text(entry_value, "");
+    gtk_window_set_transient_for(GTK_WINDOW(add_dialog), GTK_WINDOW(dlgMacro));
+    gtk_widget_show_all(GTK_WIDGET(add_dialog));
+
+    int response = gtk_dialog_run(add_dialog);
+    gtk_widget_hide(GTK_WIDGET(add_dialog));
+
+    if (response == GTK_RESPONSE_OK) {
+        const gchar* new_key = gtk_entry_get_text(entry_key);
+        const gchar* new_value = gtk_entry_get_text(entry_value);
+        if (strlen(new_key) == 0) {
+            return;
+        }
+
+        gchar key_buf[MAX_MACRO_KEY_LEN];
+        gchar value_buf[MAX_MACRO_TEXT_LEN];
+        strncpy(key_buf, new_key, MAX_MACRO_KEY_LEN - 1);
+        key_buf[MAX_MACRO_KEY_LEN - 1] = '\0';
+        strncpy(value_buf, new_value, MAX_MACRO_TEXT_LEN - 1);
+        value_buf[MAX_MACRO_TEXT_LEN - 1] = '\0';
+
+        auto store = GTK_LIST_STORE(gtk_tree_view_get_model(tree_macro));
+        if (list_store_check_exists(store, key_buf)) {
+            GtkWidget* msg = gtk_message_dialog_new(GTK_WINDOW(dlgMacro),
+                GTK_DIALOG_MODAL,
+                GTK_MESSAGE_WARNING,
+                GTK_BUTTONS_OK,
+                _("Macro '%s' already exists."), key_buf);
+            gtk_dialog_run(GTK_DIALOG(msg));
+            gtk_widget_destroy(msg);
+            return;
+        }
+
+        GtkTreeIter iter;
+        int n = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(store), NULL);
+        if (n > 0) {
+            gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store), &iter, NULL, n - 1);
+            gchar* last_key;
+            gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, COL_KEY, &last_key, -1);
+            if (g_strcmp0(last_key, STR_NULL_ITEM) == 0) {
+                gtk_list_store_remove(store, &iter);
+            }
+            g_free(last_key);
+        }
+
+        GtkTreeIter new_iter;
+        gtk_list_store_append(store, &new_iter);
+        gtk_list_store_set(store, &new_iter, COL_KEY, key_buf, COL_VALUE, value_buf, -1);
+        list_store_add_null_item(store);
+
+        sort_list_store(current_sort_column,
+            current_sort_column == COL_KEY ? sort_key_order : sort_value_order);
+
+        while (gtk_events_pending())
+            gtk_main_iteration();
+
+        update_macro_count_label();
+    }
+}
```

### 11.10 Public callback surface — `setup/signal_handlers.h`
Primary implementation commit: `e70a2c24bfff16090e755de0744577fd5c593510`

```diff
+    G_MODULE_EXPORT void on_btn_macro_add_clicked(GtkButton *button, gpointer user_data);
+    G_MODULE_EXPORT void on_column_key_clicked(GtkTreeViewColumn *column, gpointer user_data);
+    G_MODULE_EXPORT void on_column_value_clicked(GtkTreeViewColumn *column, gpointer user_data);
```

### 11.11 UI resource changes — `setup/ibus-unikey.ui`
Primary implementation commit: `e70a2c24bfff16090e755de0744577fd5c593510`

```diff
-    <property name="width_request">600</property>
-    <property name="height_request">360</property>
+    <property name="width_request">700</property>
+    <property name="height_request">450</property>
```

```diff
-              <object class="GtkScrolledWindow" id="scrollMacros">
-                <property name="can_focus">True</property>
-                  <object class="GtkTreeView" id="tree_macro">
+              <object class="GtkBox" id="box_table_area">
+                <property name="can_focus">False</property>
+                <property name="orientation">vertical</property>
+                <property name="spacing">4</property>
+                  <object class="GtkScrolledWindow" id="scrollMacros">
+                      <object class="GtkTreeView" id="tree_macro">
+                        <property name="visible">True</property>
+                        <property name="can_focus">True</property>
+                        <property name="model">list_macro</property>
+                        <property name="search_column">0</property>
+                        <property name="enable_search">True</property>
```

```diff
-                        <property name="title" translatable="yes">Word</property>
-                        <property name="clickable">True</property>
+                        <property name="sizing">fixed</property>
+                        <property name="fixed_width">180</property>
+                        <property name="title" translatable="yes">Word</property>
+                        <property name="clickable">True</property>
+                        <property name="sort_indicator">True</property>
+                        <property name="sort_column_id">0</property>
+                        <signal name="clicked" handler="on_column_key_clicked" swapped="no"/>
```

```diff
-                        <property name="title" translatable="yes">Replace with</property>
-                        <property name="clickable">True</property>
+                        <property name="expand">True</property>
+                        <property name="title" translatable="yes">Replace with</property>
+                        <property name="clickable">True</property>
+                        <property name="sort_indicator">True</property>
+                        <property name="sort_column_id">1</property>
+                        <signal name="clicked" handler="on_column_value_clicked" swapped="no"/>
```

```diff
+                  <object class="GtkBox" id="box_bottom_buttons">
+                    <property name="visible">True</property>
+                    <property name="can_focus">False</property>
+                    <property name="spacing">4</property>
+                    <child>
+                      <object class="GtkButton" id="btn_macro_add">
+                        <property name="label">gtk-add</property>
+                        <property name="visible">True</property>
+                        <property name="can_focus">True</property>
+                        <property name="receives_default">True</property>
+                        <property name="use_stock">True</property>
+                        <signal name="clicked" handler="on_btn_macro_add_clicked" swapped="no"/>
+                      </object>
+                    </child>
+                    <child>
+                      <object class="GtkButton" id="btn_macro_remove">
+                        <property name="label">gtk-remove</property>
+                        <property name="visible">True</property>
+                        <property name="can_focus">True</property>
+                        <property name="receives_default">True</property>
+                        <property name="use_stock">True</property>
+                        <signal name="clicked" handler="on_btn_macro_del_clicked" swapped="no"/>
+                      </object>
+                    </child>
+                    <child>
+                      <object class="GtkLabel" id="lbl_macro_count">
+                        <property name="visible">True</property>
+                        <property name="can_focus">False</property>
+                        <property name="hexpand">True</property>
+                        <property name="xalign">1</property>
+                        <property name="label" translatable="yes">0 macros</property>
+                      </object>
+                    </child>
+                  </object>
```

```diff
+  <object class="GtkDialog" id="add_macro_dialog">
+    <property name="can_focus">False</property>
+    <property name="title" translatable="yes">Add Macro</property>
+    <property name="modal">True</property>
+    <child internal-child="vbox">
+      <object class="GtkBox">
+        <child>
+          <object class="GtkGrid" id="grid_add_macro">
+            <child>
+              <object class="GtkEntry" id="entry_macro_key">
+                <property name="visible">True</property>
+                <property name="can_focus">True</property>
+                <property name="hexpand">True</property>
+                <property name="max_length">15</property>
+                <property name="activates_default">True</property>
+              </object>
+            </child>
+            <child>
+              <object class="GtkEntry" id="entry_macro_value">
+                <property name="visible">True</property>
+                <property name="can_focus">True</property>
+                <property name="hexpand">True</property>
+                <property name="width_chars">40</property>
+                <property name="activates_default">True</property>
+              </object>
+            </child>
+          </object>
+        </child>
+      </object>
+    </child>
+    <action-widgets>
+      <action-widget response="-6">btn_add_cancel</action-widget>
+      <action-widget response="-5">btn_add_ok</action-widget>
+    </action-widgets>
+  </object>
```

### 11.12 Validation tooling additions — `tools/macro_probe.cpp` and `tools/stress_macro_test.sh`
Primary implementation commit: `e70a2c24bfff16090e755de0744577fd5c593510`

```diff
+int main(int argc, char** argv) {
+  if (argc != 3) {
+    std::cerr << "usage: macro_probe <in-file> <out-file>\n";
+    return 2;
+  }
+
+  CMacroTable table;
+  table.init();
+
+  if (!table.loadFromFile(argv[1])) {
+    std::cerr << "load_failed\n";
+    return 1;
+  }
+
+  std::cout << "loaded_count=" << table.getCount() << "\n";
+
+  if (!table.writeToFile(argv[2])) {
+    std::cerr << "write_failed\n";
+    return 1;
+  }
+
+  table.resetContent();
+
+  if (!table.loadFromFile(argv[2])) {
+    std::cerr << "reload_failed\n";
+    return 1;
+  }
+
+  std::cout << "reloaded_count=" << table.getCount() << "\n";
+  return 0;
+}
```

```diff
+ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
+BUILD_DIR="${1:-$ROOT_DIR/build}"
+MACRO_COUNT="${2:-50000}"
+
+INPUT_FILE="$WORK_DIR/macro_input.txt"
+OUTPUT_FILE="$WORK_DIR/macro_output.txt"
+PROBE_CPP="$WORK_DIR/macro_probe.cpp"
+PROBE_BIN="$WORK_DIR/macro_probe"
+
+g++ -std=c++17 -I"$ROOT_DIR/ukengine" "$PROBE_CPP" "$BUILD_DIR/ukengine/libukengine.a" -o "$PROBE_BIN"
+
+"$PROBE_BIN" "$INPUT_FILE" "$OUTPUT_FILE" "$MACRO_COUNT"
```

## 12. Validation plan

A recreated implementation should be validated in this order.

### 12.1 Build

- configure and build the project,
- confirm `ibus-setup-unikey` links successfully,
- confirm `setup/ibus-unikey.ui` is installed.

### 12.2 Setup UI smoke test

Open the macro editor and verify:
- existing macro file loads,
- rows display in UTF-8,
- the final row is the sentinel placeholder,
- macro count is correct,
- sorting by key and value works,
- sorting keeps sentinel at the end.

### 12.3 CRUD verification

Verify:
- add rejects duplicates,
- delete does not delete the sentinel row,
- clear leaves exactly one sentinel row,
- save writes updates only after dialog confirmation,
- cancel discards unsaved edits.

### 12.4 Import/export verification

Verify:
- native UniKey import works,
- YAML import/export works,
- plist import/export works,
- import into a populated list preserves existing keys,
- export output round-trips back into `CMacroTable`.

### 12.5 Large-file verification

Use:
- `tools/macro_probe.cpp`
- `tools/stress_macro_test.sh`

Verify:
- large macro files load without UI stalls beyond expected rebuild cost,
- resulting macro files still load into `CMacroTable`,
- duplicate rules remain deterministic.

## 13. Git traceability

These commits are the main trace points for the shipped design:

- `e70a2c2` — macro storage, CRUD/sort UI, docs, and tools
- `4d2f661` — macro-file headers and related macro-file touchpoints
- `dda4607` — routine documentation coverage across touched setup/engine files
- `6dd2e55` — descriptive parameter-doc cleanup across touched files
- `faac9cf` — charset comments relevant to macro conversion behavior

Useful lookup commands:

```bash
git log feature/macro-unlimited-crud-sort --no-merges --name-status --pretty=format:'COMMIT %h %ad %an %s' --date=short

git show --name-status e70a2c2

git grep -n 'on_btn_macroedit_clicked\|unikey_macro_to_store\|unikey_store_to_macro\|macro_table_load_any_format\|macro_table_write_any_format\|removeDuplicates\|list_store_append'
```

## 14. Related documents

- Maintainer note:
  - `project_planning/UI/macros/20260301_170054_macros_improvements_and_code_comments.md`
- Code map:
  - `project_planning/UI/macros/code_structure.md`
- Future drag/drop merge design:
  - `project_planning/UI/macros/20260308_203348_drag_drop_merge_design.md`

## 15. Conclusion

The landed macro-editor redesign is best understood as one coherent subsystem:

- `CMacroTable` owns durable macro state,
- `macro_utils` owns UI/model synchronization,
- `signal_handlers` owns dialog behavior,
- `macro_file_io` owns external format compatibility,
- `tools/` provides reproducible validation.

A developer recreating the shipped changes should preserve that split, preserve
the duplicate-resolution rules in the correct layer, and preserve the sentinel,
sorting, and bulk-load invariants documented above.
