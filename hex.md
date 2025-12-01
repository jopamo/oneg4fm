**Hex Editor Feature Checklist (progress)**

- Data views
  - [x] Hex view with fixed-width rows and offset column
  - [x] ASCII/character view
  - [x] Decoded previews (ints, floats, UTF-8) via inspector

- Editing modes
  - [x] Direct byte edit in hex and ASCII panes
  - [x] Insert vs. overwrite toggle

- Navigation & addressing
  - [x] Jump to offset (hex/decimal), start/end
  - [x] Show current cursor offset (hex + decimal)

- Selection & basic ops
  - [x] Range selection
  - [x] Copy/cut/paste raw bytes; paste hex text
  - [ ] Fill selection with pattern (00/FF/repeating)
  - [x] Delete/insert blocks

- Search & replace
  - [x] Search hex patterns
  - [x] Search text (UTF-8; UTF-16 pending)
  - [x] Find next/previous and find-all summary
  - [x] Replace (single/all) for hex/text
  - [ ] Wildcard/regex search

- Data interpretation
  - [x] View bytes as signed/unsigned ints (LE/BE)
  - [x] Floats/doubles
  - [ ] Timestamps (UNIX, FILETIME, etc.)
  - [x] Inspector widget for selection/cursor

- File safety & operations
  - [~] Efficient large-file support (paged core; further bounding TBD)
  - [ ] Read-only mode
  - [ ] Backups/diff preview on save
  - [x] Multi-level undo/redo

- Structure/templates
  - [ ] Binary templates/structs with fields/arrays
  - [ ] Live decode + navigation between template fields and bytes

- Analysis tools
  - [x] Checksums/hashes (CRC32, SHA-256) over selection/whole file
  - [x] Byte frequency + entropy stats
  - [ ] XOR/arithmetic ops on ranges

- Bookmarks & annotations
  - [x] Bookmarks with labels and navigation
  - [ ] Comments/annotations and color regions

- Multi-file & diff
  - [x] Binary diff against another file; navigate differences
  - [ ] Side-by-side diff view

- Encoding/endian helpers
  - [ ] Byte swap (16/32/64-bit)
  - [ ] Text encoding conversions for ranges
  - [ ] View as code points/escaped strings

- Scripting & automation
  - [ ] Macro/scripting (Python/Lua/etc.)
  - [ ] Batch operations
  - [ ] Custom decoders/encoders

- UI/UX essentials
  - [ ] Configurable bytes per row, font, color scheme
  - [x] Keyboard-centric workflow for jump/edit/search
  - [x] Status bar with offset/value/selection/mode indicators
