# TODO2: Migration Checklist for Core Library

This checklist outlines the steps to migrate the essential core logic of `libfm-qt` into a new, separate library (tentatively named `libfm-qt-core`). The goal is to separate non-UI business logic from UI components.

## 1. Project Structure & Build System Setup
- [ ] Create a new directory for the core library (e.g., `libfm-qt-core` or within `src/core`).
- [ ] Define a new CMake target for this library.
- [ ] Ensure the new target links against:
    - [ ] `Qt6::Core`
    - [ ] `GLib` / `GIO` (via pkg-config)
    - [ ] `libfm-qt`'s internal C/VFS dependencies (see below).

## 2. Low-Level VFS & Legacy C Code Migration
Move the foundational C code that interfaces with GIO.
- [ ] Migrate `libfm-qt/src/core/vfs/` contents.
- [ ] Migrate `libfm-qt/src/core/legacy/` contents.
- [ ] Ensure these compile as part of the new library without UI dependencies.

## 3. Core Data Models & Utilities Migration
Move the fundamental C++ classes representing filesystem entities.
- [ ] `FileInfo` (`fileinfo.h`, `fileinfo.cpp`, `fileinfo_p.h`)
- [ ] `FilePath` (`filepath.h`, `filepath.cpp`)
- [ ] `Folder` (`folder.h`, `folder.cpp`)
- [ ] `FolderConfig` (`folderconfig.h`, `folderconfig.cpp`)
- [ ] `MimeType` (`mimetype.h`, `mimetype.cpp`)
- [ ] `IconInfo` (`iconinfo.h`, `iconinfo.cpp`)
- [ ] Helper pointers: `gobjectptr.h`, `gioptrs.h`, `cstrptr.h`

## 4. Job System Migration
Move the asynchronous job processing classes.
- [ ] Base `Job` class (`job.h`, `job.cpp`, `job_p.h`)
- [ ] `DirListJob`
- [ ] `FileTransferJob`
- [ ] `FileOperationJob`
- [ ] `DeleteJob`
- [ ] `TrashJob` / `UntrashJob`
- [ ] `ThumbnailJob`
- [ ] `FileSystemInfoJob`
- [ ] `TotalSizeJob`
- [ ] `FileChangeAttrJob`
- [ ] `FileLinkJob`

## 5. System Services & Managers Migration
Move the singletons and managers that handle system integration.
- [ ] `VolumeManager` (`volumemanager.h`, `volumemanager.cpp`)
- [ ] `Bookmarks` (`bookmarks.h`, `bookmarks.cpp`)
- [ ] `Archiver` (`archiver.h`, `archiver.cpp`)
- [ ] `Terminal` (`terminal.h`, `terminal.cpp`)
- [ ] `Thumbnailer` (`thumbnailer.h`, `thumbnailer.cpp`)
- [ ] `UserInfoCache` (`userinfocache.h`, `userinfocache.cpp`)
- [ ] `Templates` (`templates.h`, `templates.cpp`)
- [ ] `FileMonitor` (`filemonitor.h`, `filemonitor.cpp`)

## 6. Basic File Launching
- [ ] `BasicFileLauncher` (`basicfilelauncher.h`, `basicfilelauncher.cpp`)

## 7. Refactoring libfm-qt (UI Library)
- [ ] Update `libfm-qt`'s CMakeLists.txt to link against the new core library.
- [ ] Remove compiled core sources from `libfm-qt`'s build (to avoid symbols duplication).
- [ ] Fix include paths in `libfm-qt` source files to point to the new core headers.

## 8. Application Update
- [ ] Update `pcmanfm-qt` build system to link against the new core library (if not transitively linked via `libfm-qt`).
- [ ] Verify `pcmanfm-qt` compiles and runs.
