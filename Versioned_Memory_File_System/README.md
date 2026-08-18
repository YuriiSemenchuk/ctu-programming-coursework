# Versioned Memory File System

A C++ in-memory binary file (`CFile`) with read/write/seek/truncate, snapshots (`addVersion`), and rollback (`undoVersion`). Unchanged data is shared between copies and versions via reference counting and copy-on-write chunks.

## Files

| File | Description |
| --- | --- |
| `main.cpp` | `Chunk`, `Content`, `Version`, `CFile`, and local tests |

## Public API (`CFile`)

| Method | Description |
| --- | --- |
| `seek(size_t pos)` | Set cursor; fails if `pos` > file size |
| `read(uint8_t dst[], size_t len)` | Read up to `len` bytes from cursor; advances cursor |
| `write(const uint8_t src[], size_t len)` | Write at cursor; extends file if needed |
| `truncate()` | Cut file at cursor position |
| `fileSize()` | Logical file length |
| `addVersion()` | Snapshot current state |
| `undoVersion()` | Restore previous snapshot; fails if only one version remains |

Copy constructor and assignment operator duplicate the version stack while sharing `Content` via reference counts.

## Design

- **Chunk** — byte buffer with `refCount`; allocated dynamically.
- **Content** — array of chunks, logical size, minimum chunk size; `cloneShallow()` for COW.
- **Version** — pointer to `Content` plus cursor position.
- **CFile** — stack of versions; `detachCurrentBody()` clones shared `Content` before a mutating operation.

Writes and truncates only duplicate chunks that are actually modified or removed.

## Build and run

```bash
g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o versioned-file
./versioned-file
```

At startup, the program prompts:

```text
write a for auto test or m for manual test or e for exit
```

| Key | Action |
| --- | --- |
| `a` | Run `auto_test()` — fixed `assert` scenarios (copy, versions, undo) |
| `m` | Run `manual_test()` — line-oriented REPL on stdin |
| `e` | Exit |

### Manual REPL commands

| Command | Action |
| --- | --- |
| `W` / `write` + text | Write bytes of remainder of line |
| `R` / `read` + n | Read `n` bytes to stdout |
| `S` / `seek` + pos | Seek to position |
| `A` / `add` | `addVersion()` |
| `U` / `undo` | `undoVersion()` |

Under `#ifndef __PROGTEST__`, `auto_test()` can also be invoked directly by choosing `a` at the menu.

## Concepts

Rule of three, reference counting, copy-on-write, chunked storage, version stacks, binary I/O.
