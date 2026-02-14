# HACKING

## Mental Model
This tree is a mixed architecture, not a clean rewrite.

- `pcmanfm/` is orchestration and product policy.
- `libfm-qt/src/` is a vendored widget/model stack with long-lived behavior quirks.
- `src/core`, `src/backends/qt`, and `src/ui` are newer Qt/POSIX components (file ops core, archive helpers, hex/disassembly tools).

`Panel::` in `src/panel/panel.h` is mostly an alias layer over `Fm::` types, not an isolation boundary. Treat it as compatibility glue.

The practical consequence: many features cross old and new subsystems. Refactors that assume one coherent layer usually break behavior in edge modes (split view, compact mode, recreated views, async jobs).

Essential complexity here:

- Qt item-view semantics differ by mode and by underlying widget (`QListView` vs `QTreeView`).
- Filesystem safety constraints (no-follow symlink behavior) are intentional and security-relevant.
- Progress and cancellation semantics are part of public behavior, not implementation detail.

Accidental complexity here:

- Transitional duplication and adapters (for example `Panel::PathBar` in `libfm-qt` vs `PCManFM::PathBar` in `src/ui`).
- Migration-era seams where ownership and lifetime rules are inconsistent across modules.

## Core Invariants
Do not break these.

- **No symlink-follow for dangerous operations.**
  - Enforced in multiple places: `src/core/fs_ops.cpp`, `src/core/windowed_file_reader.cpp`, `src/ui/hexdocument.cpp`, `src/core/archive_writer.cpp`, `src/core/archive_extract.cpp`.
  - If you replace low-level calls, preserve `O_NOFOLLOW` and `AT_SYMLINK_NOFOLLOW` behavior and equivalent checks.

- **Archive path safety is strict.**
  - Extraction sanitizes and rejects unsafe entries (`..`, absolute paths) in `src/core/archive_extract.cpp`.
  - Destination root is expected not to pre-exist.
  - Cancellation is expected to abort cleanly and leave no extracted tree on failure paths (see `tests/archive_extract_test.cpp`).

- **FolderView mode switches can recreate the child view.**
  - `libfm-qt/src/folderview.cpp` may `delete view` in `setViewMode()` when crossing detailed-list boundaries.
  - Any child-view hooks must be reattached after mode changes. See `pcmanfm/view.cpp` (`setupThumbnailHooks()` in `View::setViewMode()`) and `pcmanfm/tabpage.cpp` re-installing event filters.

- **Viewport, not parent view, is the event surface for many interactions.**
  - Event filters for wheel/hover/tooltip logic are intentionally installed on `childView()->viewport()` in multiple places.
  - Moving these filters to parent widgets silently drops behavior.

- **Compact mode is horizontally scrolled.**
  - In compact mode, flow is `TopToBottom` and horizontal scrollbar is primary (`libfm-qt/src/folderview.cpp`).
  - History save/restore and wheel behavior must use the active axis (`pcmanfm/tabpage.cpp`).

- **Wheel input must remain per-instance and device-appropriate.**
  - No cross-view shared accumulators for wheel deltas (`libfm-qt/src/folderview.cpp`).
  - Use `pixelDelta()` when present; treat tiny `angleDelta()` as high-resolution input, not immediate consume-and-drop.

- **Hex editor model is segment-based and concurrency-protected.**
  - `src/ui/hexdocument.cpp` tracks edits via segments over original data plus appended buffer, guarded by `std::shared_mutex`.
  - Undo/redo, modified markers, and save semantics depend on this representation.
  - Do not collapse into a single giant mutable buffer for convenience.

- **Hex view scroll math must stay within Qt scrollbar limits.**
  - Qt scrollbars are `int`-based. Large files require clamping in row/range math (`src/ui/hexeditorview.cpp`).

- **`FolderView::setModel()` ownership is non-obvious.**
  - `libfm-qt/src/folderview.cpp` deletes its previous model internally.
  - `CachedFolderModel` has manual `ref()/unref()` lifecycle (`libfm-qt/src/cachedfoldermodel.h`, `pcmanfm/tabpage.cpp`).
  - Treat model ownership transfer and unref ordering as fragile.

- **QtFileOps cancellation is cooperative and semantic.**
  - Worker thread cancellation is atomic flag based (`src/backends/qt/qt_fileops.cpp`).
  - Error code `ECANCELED` behavior is expected by tests and UI flows.

## Sharp Edges
These are common false refactors.

- **"Simplify" `FolderView` by removing workaround code.**
  - Many odd branches in `libfm-qt/src/folderview.cpp` are compensating for Qt behavior regressions (selection ordering, hit-testing, list-mode scrollbar jumps, drag/drop oddities).
  - If code looks strange, read nearby comments before touching it.

- **Assuming one scroll axis everywhere.**
  - Vertical-only assumptions break compact mode history, wheel handling, and restore logic.

- **Assuming child widgets are stable.**
  - Hooking once in constructors is wrong when view mode can rebuild internals.

- **Replacing low-level POSIX operations with convenience Qt APIs.**
  - Convenience APIs often blur symlink semantics and error specificity.
  - In `src/core/*` and security-sensitive UI models, keep explicit POSIX behavior unless you can prove equivalence.

- **Removing pre-scan progress aggregation in file ops.**
  - `QtFileOps` scans inputs first to provide stable totals across sources and recursive delete (`tests/qt_fileops_test.cpp`).
  - This extra IO is deliberate; removing it regresses user-visible progress semantics.

- **Treating `BrowseHistory` as robust without guards.**
  - `BrowseHistory` has known TODO/FIXME corners (`libfm-qt/src/browsehistory.cpp`).
  - Callers must avoid empty-history assumptions and maintain explicit save/restore discipline.

- **Mixing up the two path bar implementations.**
  - `Panel::PathBar` (`libfm-qt/src/pathbar.*`) is what main window navigation uses.
  - `src/ui/pathbar.*` is a Qt-native widget used by newer code paths.
  - Changes in one do not automatically affect the other.

## Modification Patterns
Use these patterns to avoid subtle regressions.

- **When changing view mode behavior:**
  - Assume child view replacement can happen.
  - Rebind event filters and any scrollbar/model signal hooks after `setViewMode()`.
  - Verify detailed, icon, thumbnail, and compact modes all still work.

- **When changing scrolling behavior:**
  - Resolve active scrollbar from mode first.
  - Prefer `pixelDelta()` for precision devices.
  - Keep per-instance wheel state.
  - Validate in split view and in two independent windows (state must not leak).

- **When adding settings:**
  - Update defaults in `Settings` constructor.
  - Wire load/save in `pcmanfm/settings.cpp`.
  - Add/adjust tests in `tests/test_settings_functionality.cpp`.

- **When extending file operations:**
  - Implement core semantics in `src/core/fs_ops.*` first.
  - Adapt through `src/backends/qt/qt_fileops.*` without changing contract shape.
  - Keep cancellation/error mapping stable (`ECANCELED`, meaningful messages).

- **When extending archive code:**
  - Preserve path sanitization, recursion guards, and no-follow behavior.
  - Keep progress monotonic and cancellation deterministic.
  - Add test vectors in `tests/archive_extract_test.cpp` or archive writer tests.

- **When touching hex editor internals:**
  - Keep segment operations coherent with undo/redo.
  - Maintain external-change detection logic.
  - Avoid any operation that reads whole-file into memory by default.

- **When changing thumbnail prefetch logic:**
  - Keep hook setup tied to view lifecycle (`pcmanfm/view.cpp`).
  - Verify mode switches and model resets still trigger prefetch scheduling.

## Before You Send a Patch
- [ ] Did you preserve no-follow and path-safety semantics for all filesystem and archive paths?
- [ ] If you touched view mode or scrolling, did you test icon/thumbnail/compact/detailed and split view?
- [ ] If you touched child view internals, did you reattach viewport/filter/scrollbar hooks after recreation?
- [ ] If you changed progress/cancel behavior, do `FsOps` and `QtFileOps` still emit monotonic progress and `ECANCELED` correctly?
- [ ] If you changed settings, are constructor defaults, load/save, and tests updated together?
- [ ] If you touched hex editor behavior, did you verify large-file scrolling, undo/redo integrity, and external-change checks?
