**Hex Editor Feature Checklist**

- Data views
  - Hex view with 00–FF columns and fixed-width rows (8/16 bytes) plus offset column
  - ASCII/character view (printable chars, dots for non-printable)
  - Optional decoded views (integers, floats, UTF-8/UTF-16, etc.)

- Editing modes
  - Direct in-place byte edit in hex and ASCII panes
  - Insert vs. overwrite mode toggle (insert shifts data)

- Navigation & addressing
  - Jump to offset (hex/decimal), start/end of file
  - Show current cursor offset (hex + decimal)

- Selection & basic ops
  - Range selection
  - Copy/cut/paste raw bytes; paste hex text
  - Fill selection with pattern (00/FF/repeating)
  - Delete/insert blocks

- Search & replace
  - Search hex patterns (e.g., DE AD BE EF)
  - Search text (ASCII, UTF-8, UTF-16), optional wildcard/regex
  - Find next/previous and find-all with match list
  - Replace for both hex and text

- Data interpretation
  - View selected bytes as signed/unsigned ints (sizes, endianness)
  - Floats/doubles
  - Timestamps (UNIX epoch, FILETIME, etc.)
  - Quick “inspect selection as…” widget

- File safety & operations
  - Efficient large-file support (paging/mmap)
  - Read-only mode
  - Backup/undo safety: temp file, diff/preview, or transactional save
  - Multi-level undo/redo

- Structure/templates
  - Binary templates/structs with named fields, nested structures, arrays
  - Live decode + navigation between template fields and bytes

- Analysis tools
  - Checksums/hashes over selection (CRC32, MD5, SHA-1/2, etc.)
  - Byte frequency/entropy stats
  - XOR/arithmetic ops on ranges

- Bookmarks & annotations
  - Bookmarks with labels
  - Comments/annotations on ranges
  - Color highlighting for regions

- Multi-file & diff
  - Open multiple files
  - Binary diff view: compare by offset, side-by-side, navigate differences

- Encoding/endian helpers
  - Byte swap (16/32/64-bit)
  - Convert text encodings for ranges
  - View as code points / escaped strings for copy/paste

- Scripting & automation
  - Macro recording or scripting (Python/Lua/etc.)
  - Batch operations
  - Custom decoders/encoders

- UI/UX essentials
  - Configurable bytes per row, font, and color scheme
  - Keyboard-centric workflow (jump/edit/search without mouse)
  - Status bar: offset, value in multiple formats, selection size, insert/overwrite indicator
