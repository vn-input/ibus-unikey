# Full Project Migration Plan: Move ibus-unikey Toward Modern C++

**Date:** 2026-03-16  
**Scope:** Whole repository  
**Primary objective:** Replace internal C-style allocation, raw buffer bookkeeping, and ad-hoc record layouts with modern C++ containers, classes, RAII, and clearer module boundaries while preserving runtime behavior, file-format compatibility, and public integration points.

## 1. Executive summary

This migration is **not** a rewrite. It is a staged modernization of the existing codebase so the project becomes easier to maintain, safer to evolve, and less dependent on manual memory ownership.

The most important constraint is that the project still integrates with **GLib**, **GTK**, and **IBus**, all of which expose C APIs. That means the target state is **modern C++ internally with deliberate C-compatible boundaries**, not total removal of all C-style interop.

The migration should therefore follow three rules:

1. Keep external behavior stable.
2. Replace internals incrementally behind compatibility layers.
3. Modernize by subsystem, not by scattered one-off edits.

## 2. Current architecture snapshot

Based on the current repository structure, the project is split into these major areas:

### 2.1 Core engine: `src/ukengine/` and `include/ukengine/`

This is the most sensitive area and the highest-risk migration zone.

- `ukengine.cpp` contains the main Vietnamese input processing pipeline.
- `mactab.cpp` / `mactab.h` implement macro storage and lookup using manual allocation and offset tables.
- `usrkeymap.cpp` contains explicit dynamic allocation patterns such as `new char[bufSize]`.
- `byteio.cpp`, `charset.h`, `vnconv`, and related modules form the low-level data and encoding layer.
- `ukengine.h` exposes shared structures such as `UkSharedMem`, which currently embeds `CMacroTable`.

### 2.2 IBus engine adapter: `src/engine/` and `include/engine/`

This layer bridges the core engine to IBus runtime objects.

- The code already mixes C and C++ styles.
- There are ownership-sensitive objects allocated with `new`, including state such as `std::string` pointers.
- This area is a strong candidate for RAII cleanup and object-lifetime simplification.

### 2.3 GTK setup application: `src/setup/` and `include/setup/`

This subsystem is already partially modernized.

- Files such as `macro_file_io.cpp` already use `std::vector` and `std::string`.
- UI glue still uses a large amount of `gchar*`, `g_free`, fixed-size buffers, and direct GTK/GLib ownership rules.
- This layer should move toward C++ domain classes plus thin GTK boundary adapters.

### 2.4 Tests: `tests/`

The repository already has focused tests for UI and engine behavior.

- `tests/UI/macros/*` covers import/export and macro dialog behavior.
- `tests/ukengine/*` covers core engine logic.
- These tests are essential for a phased migration and should be expanded before high-risk refactors.

### 2.5 Tools and build system

- `CMakeLists.txt`, subdirectory CMake files, and helper tools under `tools/` will need updates as new classes/files are introduced.
- Build rules should eventually enforce a consistent C++ standard and stricter warnings for migrated modules.

## 3. Migration goals

### 3.1 Technical goals

- Replace manual memory growth logic with `std::vector`, `std::string`, `std::array`, and smart pointers where appropriate.
- Replace raw record structs used as storage mechanics with domain-specific classes and value types.
- Reduce raw owning pointers and `new` / `delete` in project code.
- Isolate GLib/GTK/IBus ownership to adapter functions instead of spreading it across business logic.
- Preserve existing file formats, macro semantics, and engine behavior.

### 3.2 Maintainability goals

- Make ownership explicit.
- Reduce hidden coupling between storage layout and business logic.
- Split large responsibilities into focused classes and helpers.
- Make unit testing easier by separating parsing, state, and UI binding.

### 3.3 Non-goals

- Do not replace GLib, GTK, or IBus with different frameworks.
- Do not rewrite the full Vietnamese processing algorithm in one step.
- Do not break compatibility with existing configuration, macro, or keymap files.
- Do not attempt a single massive branch that changes the whole repository at once.

## 4. Guiding principles

1. **Boundary-first modernization**  
  Keep C API interactions at the boundary and move the inner logic to C++ value types.

2. **Compatibility wrappers before removal**  
  Introduce new classes first. Remove old storage only after callers are migrated.

3. **Test before invasive refactors**  
  Expand regression coverage in any subsystem before replacing low-level internals.

4. **Prefer value semantics**  
  Use stack-owned objects and containers by default. Use heap allocation only where lifetime or polymorphism requires it.

5. **Keep module responsibilities small**  
  Parsing, storage, UI conversion, and runtime engine logic should not live in the same class when avoidable.

## 5. Repository-wide target design

### 5.1 Core design direction

The project should move toward this internal model:

- **Domain data** represented by C++ structs/classes.
- **Collections** represented by `std::vector`, `std::unordered_map`, `std::map`, or `std::array` depending on access needs.
- **Strings and text buffers** represented by `std::string` or typed containers for internal encoding data.
- **Ownership** represented by RAII types:
  - `std::unique_ptr`
  - stack values
  - dedicated wrapper types for GLib resources when needed
- **Adapters** responsible for conversion between C++ objects and C APIs.

### 5.2 Boundary policy

Not every C-style pointer can disappear.

The following should remain C-oriented only at the edge:

- `gchar*` returned from GLib/GTK functions
- `GObject*`, `GtkWidget*`, `GtkBuilder*`, `IBus*` object handles
- C callbacks required by GTK/IBus APIs
- engine-facing compatibility APIs that external code expects

Inside project-owned logic, those should be converted quickly into safer C++ representations.

## 6. Subsystem migration plan

### 6.1 Phase A — Baseline, inventory, and safety net

#### Deliverables

- Written inventory of manual memory ownership hotspots.
- Stronger automated tests for engine, setup, and macro behavior.
- Build baseline for comparing behavior before and after migration.

#### Tasks

1. Audit repository usage of:
  - `malloc`, `calloc`, `realloc`, `free`
  - `new`, `delete`, `new[]`, `delete[]`
  - fixed-size char buffers
  - ownership-sensitive `gchar*` / `g_free` flows
2. Classify each use site:
  - C API boundary
  - internal storage
  - parser scratch buffer
  - long-lived owning state
3. Expand test coverage before refactoring:
  - macro storage and file round-trip tests
  - keymap loading tests
  - engine behavior tests for common typing scenarios
  - setup dialog state and import/export tests
4. Add simple migration tracking notes to this plan as phases complete.

#### Exit criteria

- The team knows which allocations are boundary-required versus project-owned.
- High-risk areas have regression tests.

### 6.2 Phase B — Establish common C++ infrastructure

#### Deliverables

- Shared helper types for ownership and conversions.
- Consistent C++ standard selection in CMake.
- Common wrappers for GLib-owned strings and resources where useful.

#### Tasks

1. Confirm and standardize the C++ language level across the project.
2. Introduce small shared helpers, for example:
  - `glib_string.h` with safe wrappers for `gchar*`
  - conversion helpers returning `std::string`
  - path helpers for setup/file code
3. Define repository conventions:
  - when to use `std::string`
  - when to use `std::vector<StdVnChar>` or similar encoded buffers
  - when raw pointers are allowed
4. Enable warnings that help detect manual-memory mistakes in migrated files.

#### Exit criteria

- New code has a consistent modernization target.
- There is a shared pattern for GLib/C++ interop.

### 6.3 Phase C — Modernize macro storage first

This is the recommended first deep refactor because it is self-contained, heavily user-visible, and already partially surrounded by tests.

#### Current issues

- `CMacroTable` uses a manually resized blob buffer and offset metadata.
- Storage growth is tied to fixed-size maximum lengths.
- Sorting and duplicate handling depend on low-level memory layout.
- File loading and UI synchronization are forced to work around storage internals.

#### Target design

- Introduce a domain record such as:

  ```cpp
  struct MacroEntry {
    std::string trigger;
    std::string replacement;
  };
  ```

- Introduce a container-backed class such as:

  ```cpp
  class MacroStore {
  public:
    bool add_or_replace(const MacroEntry& entry);
    const MacroEntry* find(std::string_view key) const;
    void clear();
    std::span<const MacroEntry> entries() const;
  private:
    std::vector<MacroEntry> entries_;
  };
  ```

- Keep a compatibility adapter for `CMacroTable` until all dependent code is migrated.

#### Tasks

1. Add a new C++ storage class in `src/ukengine/`.
2. Recreate current semantics exactly:
  - case-insensitive lookup rules
  - duplicate resolution where later definitions win
  - stable file format behavior
3. Update macro import/export logic to consume the new store directly.
4. Update setup UI synchronization helpers to avoid fixed-size temporary arrays where possible.
5. Preserve existing public APIs until all call sites move over.

#### Exit criteria

- Macro storage no longer depends on manual `realloc` + offset arithmetic.
- Macro tests pass without behavior changes.

### 6.4 Phase D — Modernize keymap and file parsing utilities

#### Current issues

- `usrkeymap.cpp` uses explicit dynamic array allocation and line-buffer management.
- Low-level file parsing patterns rely on mutable char buffers and manual growth.

#### Target design

- Replace parser scratch buffers with `std::string` and `std::vector<std::string>`.
- Isolate file reading from parsing logic.
- Represent parsed keymap entries with simple value types.

#### Tasks

1. Refactor keymap loading into:
  - file read layer
  - parse layer
  - apply layer
2. Replace `new char[bufSize]` and similar patterns with managed containers.
3. Add focused tests for malformed keymap input and compatibility with existing files.

#### Exit criteria

- Keymap code no longer owns manual dynamic line buffers.
- Parsing logic is testable without file I/O.

### 6.5 Phase E — Modernize engine state and ownership in `src/engine/`

#### Current issues

- Engine adapter code mixes C object lifetimes with heap-allocated C++ state.
- Some state uses owning raw pointers where values or smart pointers would be clearer.

#### Target design

- Convert simple heap-owned members to direct value members where possible.
- Use `std::unique_ptr` only when lifetime needs indirection.
- Create small RAII helpers for IBus/GLib object ownership when project code owns references.

#### Tasks

1. Review each owning pointer in `src/engine/` and classify it:
  - can become value member
  - should become `std::unique_ptr`
  - must remain raw because ownership is external
2. Replace pointer-owned `std::string` state with direct members.
3. Reduce manual cleanup paths in constructors/destructors/finalizers.
4. Add regression tests for engine lifecycle and property updates where practical.

#### Exit criteria

- Engine adapter code has explicit ownership with minimal raw owning pointers.

### 6.6 Phase F — Modernize setup application domain logic

#### Current issues

- Setup code mixes domain logic with GTK object manipulation.
- There is still repeated use of fixed-size buffers, `gchar*`, and manual copy helpers.
- Some logic is already in modern C++, but boundaries are inconsistent.

#### Target design

- Introduce small setup-domain classes for:
  - macro document state
  - import/export settings
  - filename completion/validation
  - UI-to-domain translation helpers
- Keep GTK-specific code thin and event-focused.

#### Tasks

1. Move reusable setup logic into pure C++ helpers where possible.
2. Replace manual bounded-copy helpers with `std::string` plus explicit validation rules.
3. Introduce boundary conversion helpers for `gchar*` <-> `std::string`.
4. Reduce direct mutation of global UI state where practical.
5. Keep GTK signal handlers as orchestration functions, not storage implementations.

#### Exit criteria

- Setup logic is easier to test outside GTK.
- UI code mostly coordinates objects rather than owning parsing/storage details.

### 6.7 Phase G — Modernize core engine internals carefully

This phase is the highest risk and should happen only after the earlier phases succeed.

#### Current issues

- Core engine modules rely on C-era data patterns and tight coupling between algorithms and storage.
- Shared state structures have constraints such as comments warning against pointers in shared objects.

#### Constraints

- `UkSharedMem` and related structures may be constrained by layout assumptions.
- Any change to embedded object layout must preserve process/shared-memory expectations.
- Algorithmic behavior must remain identical for user-visible typing output.

#### Target design

- Keep externally constrained layout stable.
- Move mutable helper logic out of shared structs and into owning C++ engine classes.
- Use safer containers internally where layout does not cross a binary/shared-memory boundary.

#### Tasks

1. Audit all structs that cross process or binary boundaries.
2. Mark them as:
  - fixed-layout / do not modernize directly
  - wrapper candidate
  - internal-only / safe to modernize
3. Extract helper logic from low-level engine files into focused classes without changing behavior.
4. Migrate internal temporary buffers to `std::array` or `std::vector` where layout safety allows.
5. Add heavy regression tests before touching algorithm-critical paths.

#### Exit criteria

- Internal engine helpers are more modular.
- Shared-memory-sensitive types remain safe and documented.

### 6.8 Phase H — Headers, API cleanup, and repository consistency

#### Deliverables

- Clear public/private header boundaries.
- Smaller public API surface.
- Removal of obsolete compatibility layers that are no longer needed.

#### Tasks

1. Move internal-only helpers out of broad public headers.
2. Replace storage-mechanism comments with domain-oriented class documentation.
3. Deprecate old compatibility methods only after all in-repo call sites are migrated.
4. Update README and developer documentation to describe the new architecture.

#### Exit criteria

- Public headers expose stable APIs, not internal storage details.

## 7. File-by-file migration priorities

### Priority 1: Immediate targets

- `src/ukengine/mactab.cpp`
- `include/ukengine/mactab.h`
- `src/setup/macro_file_io.cpp`
- `src/setup/macro_utils.cpp`
- `src/setup/signal_handlers_macros.cpp`
- `tests/UI/macros/*`

### Priority 2: Near-term targets

- `src/ukengine/usrkeymap.cpp`
- related keymap headers and tests
- `src/engine/*` ownership cleanup
- setup helper modules with repeated `gchar*` handling

### Priority 3: Later, high-risk targets

- `src/ukengine/ukengine.cpp`
- `include/ukengine/ukengine.h`
- low-level charset / byte-stream modules
- any shared-memory-sensitive or algorithm-critical storage code

## 8. Testing strategy

### 8.1 Before each subsystem refactor

- Add tests that lock down current behavior.
- Save representative sample files for macros and keymaps.
- Capture expected outputs for common typing scenarios.

### 8.2 During migration

- Run targeted unit tests after each module change.
- Run full test suite at the end of each phase.
- Compare serialized outputs where compatibility matters.

### 8.3 After migration of each phase

- Manual smoke test the setup UI.
- Manual smoke test engine behavior under IBus.
- Confirm no regressions in import/export, macros, keymaps, and options.

## 9. Build and tooling plan

### Tasks

1. Keep CMake targets modular by subsystem.
2. Introduce new source files per component rather than expanding monoliths.
3. Gradually enable stricter warnings for migrated code.
4. Optionally add static analysis in CI for ownership and bounds issues.

### Suggested policy

- Newly migrated modules must avoid raw owning pointers.
- New parsing code should avoid mutable fixed-size C buffers unless required by external APIs.
- File and string transformations should live in testable helpers.

## 10. Risks and mitigations

### Risk: behavior drift in typing engine

**Mitigation:** do not modernize core algorithm paths until tests and lower-risk modules are complete.

### Risk: ABI/layout breakage in shared structures

**Mitigation:** treat cross-boundary structs as constrained interfaces; use wrappers instead of changing layout blindly.

### Risk: GLib/GTK ownership confusion during partial migration

**Mitigation:** introduce explicit adapter helpers and document who frees what.

### Risk: oversized migration branch

**Mitigation:** ship in small phases with independent merge points.

## 11. Recommended rollout sequence

1. Baseline audit and tests.
2. Shared C++ helper infrastructure.
3. Macro subsystem migration.
4. Keymap/file parser modernization.
5. Engine adapter ownership cleanup.
6. Setup application domain cleanup.
7. Core engine internal modernization where safe.
8. Final API/header cleanup and documentation pass.

## 12. Definition of done

The migration should be considered complete only when all of the following are true:

- Major project-owned storage uses C++ containers instead of manual allocation.
- Raw owning pointers are rare and justified.
- GLib/GTK/IBus ownership is isolated to boundary code.
- Core user workflows still behave the same.
- Existing file formats remain compatible.
- Tests cover the major migrated subsystems.
- Documentation reflects the new structure and conventions.

## 13. Immediate next implementation slice

The best first implementation slice is still the macro subsystem because it offers the strongest ratio of user impact to migration risk.

Suggested first concrete tasks:

1. Add macro storage tests that fully lock current behavior.
2. Introduce `MacroEntry` and a container-backed macro store.
3. Adapt `CMacroTable` behind a compatibility layer.
4. Update setup import/export helpers to use the new domain model.
5. Remove legacy macro blob allocation only after tests pass.

---

**Planning note:** This document intentionally covers the full repository, but it keeps the migration incremental. The project should modernize one subsystem at a time while preserving the C-compatible boundaries required by GTK, GLib, IBus, and any shared-memory-sensitive engine APIs.