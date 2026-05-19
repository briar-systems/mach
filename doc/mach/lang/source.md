# mach.lang.source

Source-file registry. Maps [`FileId`](#fileid) handles to loaded path +
text and resolves byte offsets to 1-based `(line, column)` positions.
Used by [diagnostics](diagnostic.md) for span rendering and by every
phase that reports source locations.

## Types

### `FileId`

```mach
pub def FileId: u32;
```

Stable handle into a [`SourceMap`](#sourcemap)'s file array.

### `Position`

```mach
pub rec Position {
    line: usize;
    col:  usize;
}
```

1-based line and column within a source file.

| Field | Type    | Description                |
|-------|---------|----------------------------|
| line  | `usize` | Line number, 1-based.      |
| col   | `usize` | Column number, 1-based.    |

### `SourceFile`

```mach
pub rec SourceFile {
    path:        str;
    text:        str;
    line_starts: *usize;
    line_len:    usize;
}
```

A loaded source file with a precomputed newline index.

| Field       | Type      | Description                                                  |
|-------------|-----------|--------------------------------------------------------------|
| path        | `str`     | Owned file path as known to the loader.                      |
| text        | `str`     | Owned source text; `text.data[text.len]` is 0 for FFI use.   |
| line_starts | `*usize`  | Byte offset of the start of each line.                       |
| line_len    | `usize`   | Number of entries in `line_starts`.                          |

### `SourceMap`

```mach
pub rec SourceMap {
    alloc: *Allocator;
    files: *SourceFile;
    len:   usize;
    cap:   usize;
}
```

Append-only registry of loaded source files indexed by
[`FileId`](#fileid).

| Field | Type                                | Description                                          |
|-------|-------------------------------------|------------------------------------------------------|
| alloc | `*Allocator`                        | Allocator backing every owned array in the map.      |
| files | [`*SourceFile`](#sourcefile)        | Contiguous array of [`SourceFile`](#sourcefile) records. |
| len   | `usize`                             | Number of files currently stored.                    |
| cap   | `usize`                             | Allocated slots in `files`.                          |

## Constants

```mach
val INITIAL_MAP_CAP: usize = 8;
```

Starting capacity for [`SourceMap.files`](#sourcemap). The array
doubles on overflow.

## Functions

### `init`

```mach
pub fun init(alloc: *Allocator) SourceMap
```

Constructs an empty source map backed by `alloc`. Infallible.

| Param | Type         | Description                                        |
|-------|--------------|----------------------------------------------------|
| alloc | `*Allocator` | Allocator used for path/text/line index storage.   |

Returns the populated [`SourceMap`](#sourcemap).

### `dnit`

```mach
pub fun dnit(m: *SourceMap)
```

Releases every loaded file (path, text, line index) and the files array
itself. `nil` is a no-op. After `dnit` every [`FileId`](#fileid)
previously issued is invalid.

| Param | Type                          | Description                            |
|-------|-------------------------------|----------------------------------------|
| m     | [`*SourceMap`](#sourcemap)    | Source map to tear down. `nil` is a no-op.|

### `add`

```mach
pub fun add(m: *SourceMap, path: str, text: str) Result[FileId, str]
```

Copies `path` and `text` into the map's allocator, precomputes a newline
index, and returns a fresh [`FileId`](#fileid). Errors on allocation
failure; on failure no partial state is added to the map.

| Param | Type                          | Description                                |
|-------|-------------------------------|--------------------------------------------|
| m     | [`*SourceMap`](#sourcemap)    | The source map.                            |
| path  | `str`                         | File path to associate with this file.     |
| text  | `str`                         | Source text to index and store.            |

Returns the [`FileId`](#fileid) of the newly added file, or an
allocation error.

### `get`

```mach
pub fun get(m: *SourceMap, id: FileId) Option[*SourceFile]
```

Returns a pointer into the map's `files` array, or `none` when `id` is
out of range. The pointer is invalidated by a later [`add`](#add) that
grows the array.

| Param | Type                          | Description                |
|-------|-------------------------------|----------------------------|
| m     | [`*SourceMap`](#sourcemap)    | The source map.            |
| id    | [`FileId`](#fileid)           | Identifier to look up.     |

Returns `some(*SourceFile)` into `m.files` when `id` is in range,
`none` otherwise.

### `position`

```mach
pub fun position(file: *SourceFile, offset: usize) Position
```

Converts a byte offset in `file.text` to a 1-based `(line, col)`
[`Position`](#position). Uses binary search over `line_starts`.

| Param  | Type                            | Description                      |
|--------|---------------------------------|----------------------------------|
| file   | [`*SourceFile`](#sourcefile)    | The source file being queried.   |
| offset | `usize`                         | Byte offset into `file.text`.    |

Returns the 1-based `(line, col)` [`Position`](#position) for `offset`.

### `line_start`

```mach
pub fun line_start(file: *SourceFile, line: usize) Option[usize]
```

Returns the byte offset where 1-based `line` begins, or `none` when
`line` is outside `1..=line_len`.

| Param | Type                            | Description                       |
|-------|---------------------------------|-----------------------------------|
| file  | [`*SourceFile`](#sourcefile)    | The source file being queried.    |
| line  | `usize`                         | 1-based line number.              |

Returns `some(offset)` for a valid line, `none` otherwise.

## Dependencies

`std.types.bool`, `std.types.option`, `std.types.size`,
`std.types.string`, `std.types.result`, `std.allocator`.
