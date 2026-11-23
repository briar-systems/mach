[mach](../README.md) / [base](README.md) / **source**

# mach.base.source

the source map: the files of a compilation and the spans, locations and lines in them

a file is written once into a slot that never moves, so a `*SourceFile` stays valid until the
map is released, and reloading or releasing its text bumps its revision in place. a location is
a span of one file, or nowhere when its file is FILE_NIL.

Source: [`src/base/source.mach`](../../../src/base/source.mach)

Uses types from [`mach.base.handle`](handle.md), [`mach.base.intern`](intern.md).

| Type | | |
| :--- | :--- | :--- |
| [`FileId`](#fileid) | `def` | a file's slot in the source map |
| [`Span`](#span) | `rec` | the bytes [offset, offset + len) of one text |
| [`Location`](#location-1) | `rec` | where in the compilation something is: a span of the file `file`, or nowhere when `file` is FILE_NIL |
| [`Position`](#position) | `rec` | a line and a column, each from 1, the column counted in UTF-8 bytes |
| [`Lines`](#lines) | `rec` | the line index of one text: where each line starts, for mapping an offset to its line and column in time logarithmic in the text's lines |
| [`Error`](#error) | `tag` | a refusal of the source map: the allocator's or the file store's, every file identity issued, a file revised as often as its revision counts, an identity the map never issued, a path indexed without its file, or a snapshot asked of no map or allocator |
| [`SourceFile`](#sourcefile) | `rec` | one file of the source map |
| [`SourceMap`](#sourcemap) | `rec` | the files of one compilation, addressed by FileId |
| [`PreparedLoad`](#preparedload) | `rec` | a staged publication; `editor` holds the store's exclusive editor over the reserved slot of a new entry until commit or discard |

| On `FileId` | |
| :--- | :--- |
| [`FILE_NIL`](#fileid) | no source: an unlocated diagnostic, or a node the build synthesized |
| [`FILE_PENDING`](#fileid) | a span whose file is not known yet: a reader that meets its spans before it knows the file they are in places them once it does |
| [`file_id`](#file_id) | the file id at slot `index` |
| [`file_index`](#file_index) | the slot a file id names |
| [`file_same`](#file_same) | whether two ids name the same file |
| [`file_is_nil`](#file_is_nil) | whether a file id is FILE_NIL |
| [`location`](#location) | the span `s` of `file` |

| On `Span` | |
| :--- | :--- |
| [`span`](#span-1) | the `len` bytes from `offset` |
| [`span_between`](#span_between) | the bytes [offset, end) as a span; an end before the offset is empty |
| [`span_end`](#span_end) | the offset just past the span's last byte |
| [`span_view`](#span_view) | the bytes of `text` the span covers |

| On `Location` | |
| :--- | :--- |
| [`location_nil`](#location_nil) | the location that is nowhere |
| [`location_is_nil`](#location_is_nil) | whether a location is nowhere; one whose file is pending is somewhere |
| [`location_is_valid`](#location_is_valid) | whether a location names a span of a file the map can hold |
| [`location_equals`](#location_equals) | whether two locations name the same file and the same span |

| On `Lines` | |
| :--- | :--- |
| [`lines_init`](#lines_init) | the line index of the `n` bytes at `data`; released with `lines_dnit` |
| [`lines_dnit`](#lines_dnit) | release a line index through `a` and leave it blank |
| [`lines_position`](#lines_position) | the line and column of `offset`, clamped to the text's end |

| On `Error` | |
| :--- | :--- |
| [`text`](#text) | the text of a source map's refusal |

| On `SourceFile` | |
| :--- | :--- |
| [`copy_file`](#copy_file) | a copy of `file` owning its path and, when present, its text and line index through `a` |
| [`dnit_file`](#dnit_file) | release a file's path, text and line index through `a` and leave it blank |
| [`position`](#position-1) | the line and column of `offset` in the file; an offset past the end is placed at the end |
| [`line_start`](#line_start) | the offset line `line` (from 1) starts at |
| [`offset_of`](#offset_of) | the offset of `at` in the file, none for a line it does not hold; a column past the line stays where it falls |
| [`line_bounds`](#line_bounds) | the bytes of line `line` (from 1) without its line ending |

| On `SourceMap` | |
| :--- | :--- |
| [`init`](#init) | an empty source map, released with dnit |
| [`dnit`](#dnit) | release a source map and its files; a nil map holds nothing |
| [`count`](#count) | the slot count including the FILE_NIL sentinel; the next FileId to be issued |
| [`add`](#add) | add a file holding a copy of `text` under a copy of `path` |
| [`prepare_load`](#prepare_load) | the caller exclusively borrows the map until commit or discard; a new entry reserves its slot here so commit_load cannot fail |
| [`load`](#load) | the file at `path` holding `text`: a path the map holds keeps its id and takes the text, a new one is added |
| [`prepare_release`](#prepare_release) | preflight completes before query and editor owners are retired |
| [`payload_dnit`](#payload_dnit) | free a file's text and line index in place and bump its revision |
| [`find`](#find) | the file loaded from `path`, FILE_NIL when none was |
| [`get`](#get) | the file behind an id; none for FILE_NIL or an id the map never issued |

| On `PreparedLoad` | |
| :--- | :--- |
| [`discard_load`](#discard_load) | release a load that was prepared and not committed; one already committed or discarded owns nothing |
| [`commit_load`](#commit_load) | an existing file takes the staged payload in place; a new one lands in the slot prepare_load reserved |

| Functions | |
| :--- | :--- |
| [`line_break_len`](#line_break_len) | the bytes of a line break that starts with byte `c`, `next` being the byte after it: a line feed, a carriage return and line feed, or a carriage return alone; 0 for any other byte |

---

## FileId

```mach
pub def FileId: u32
```

a file's slot in the source map

every other id's nil is all ones. a file id's nil is 0, beside the register
ids the second such exception: a zero-filled location is nowhere, so records
that carry one are blank when zeroed. slot 0 of the map is never issued.

| Name | Value | |
| :--- | :--- | :--- |
| `FILE_NIL` | `0` | no source: an unlocated diagnostic, or a node the build synthesized |
| `FILE_PENDING` | `0xFFFFFFFF` | a span whose file is not known yet: a reader that meets its spans before it knows the file they are in places them once it does |

### file_id

```mach
pub fun file_id(index: u32) FileId
```

the file id at slot `index`

### file_index

```mach
pub fun file_index(f: FileId) u32
```

the slot a file id names

### file_same

```mach
pub fun file_same(left: FileId, right: FileId) bool
```

whether two ids name the same file

### file_is_nil

```mach
pub fun file_is_nil(f: FileId) bool
```

whether a file id is FILE_NIL

### location

```mach
pub fun location(file: FileId, s: Span) Location
```

the span `s` of `file`

## Span

```mach
pub rec Span {
    offset: usize;
    len:    usize;
}
```

the bytes [offset, offset + len) of one text

### span

```mach
pub fun span(offset: usize, len: usize) Span
```

the `len` bytes from `offset`

### span_between

```mach
pub fun span_between(offset: usize, end: usize) Span
```

the bytes [offset, end) as a span; an end before the offset is empty

### span_end

```mach
pub fun span_end(s: Span) usize
```

the offset just past the span's last byte

### span_view

```mach
pub fun span_view(text: str, s: Span) View
```

the bytes of `text` the span covers

## Location

```mach
pub rec Location {
    file: FileId;
    span: Span;
}
```

where in the compilation something is: a span of the file `file`, or
nowhere when `file` is FILE_NIL

### location_nil

```mach
pub fun location_nil() Location
```

the location that is nowhere

### location_is_nil

```mach
pub fun location_is_nil(l: Location) bool
```

whether a location is nowhere; one whose file is pending is somewhere

### location_is_valid

```mach
pub fun location_is_valid(l: Location) bool
```

whether a location names a span of a file the map can hold

### location_equals

```mach
pub fun location_equals(x: Location, y: Location) bool
```

whether two locations name the same file and the same span

## Position

```mach
pub rec Position {
    line: usize;
    col:  usize;
}
```

a line and a column, each from 1, the column counted in UTF-8 bytes

## Lines

```mach
pub rec Lines {
    starts: *usize;
    len:    usize;
    n:      usize;
}
```

the line index of one text: where each line starts, for mapping an offset
to its line and column in time logarithmic in the text's lines. a line ends
at a line break as line_break_len reads it

| Field | Type | |
| :--- | :--- | :--- |
| `starts` | `*usize` | the offset of each line's first byte, the first line's 0 |
| `len` | `usize` | how many lines, at least 1 once built |
| `n` | `usize` | the text's length in bytes |

### lines_init

```mach
pub fun lines_init(a: *std.allocator.Allocator, data: *u8, n: usize) res[Lines, std.allocator.Error]
```

the line index of the `n` bytes at `data`; released with `lines_dnit`. a break is `\n`, `\r\n` or a
lone `\r`, as line_break_len reads it

### lines_dnit

```mach
pub fun lines_dnit(a: *std.allocator.Allocator, l: *Lines)
```

release a line index through `a` and leave it blank

### lines_position

```mach
pub fun lines_position(l: *Lines, offset: usize) Position
```

the line and column of `offset`, clamped to the text's end

## Error

```mach
pub tag Error: u8 {
    alloc: std.allocator.Error;
    store: mach.base.handle.Error;
    exhausted;
    revision;
    absent;
    unindexed;
    unsupplied;
}
```

a refusal of the source map: the allocator's or the file store's, every
file identity issued, a file revised as often as its revision counts, an
identity the map never issued, a path indexed without its file, or a
snapshot asked of no map or allocator

### text

```mach
pub fun text(e: Error) str
```

the text of a source map's refusal

## SourceFile

```mach
pub rec SourceFile {
    present:  bool;
    revision: u64;
    path:     str;
    text:     str;
    lines:    Lines;
}
```

one file of the source map

| Field | Type | |
| :--- | :--- | :--- |
| `present` | `bool` | the file holds its text and line index, false once its payload is released |
| `revision` | `u64` | bumped each time its text is replaced or released, 1 when first loaded |
| `path` | `str` | the path it was loaded from |
| `text` | `str` | its text, nil once released |
| `lines` | `Lines` | its line index |

### copy_file

```mach
pub fun copy_file(file: *SourceFile, a: *std.allocator.Allocator) res[SourceFile, Error]
```

a copy of `file` owning its path and, when present, its text and line index through `a`

| Parameter | |
| :--- | :--- |
| `file` | the file |
| `a` | the allocator the copy is owned through |
| **returns** | the copy, released with dnit_file, or the allocator's refusal |

### dnit_file

```mach
pub fun dnit_file(file: *SourceFile, a: *std.allocator.Allocator)
```

release a file's path, text and line index through `a` and leave it blank

### position

```mach
pub fun position(file: *SourceFile, offset: usize) Position
```

the line and column of `offset` in the file; an offset past the end is
placed at the end

### line_start

```mach
pub fun line_start(file: *SourceFile, line: usize) opt[usize]
```

the offset line `line` (from 1) starts at

### offset_of

```mach
pub fun offset_of(file: *SourceFile, at: Position) opt[usize]
```

the offset of `at` in the file, none for a line it does not hold; a column past the line
stays where it falls

### line_bounds

```mach
pub fun line_bounds(file: *SourceFile, line: usize) opt[Span]
```

the bytes of line `line` (from 1) without its line ending

## SourceMap

```mach
pub rec SourceMap {
    a:       *std.allocator.Allocator;
    files:   mach.base.handle.StableChunks[SourceFile];
    paths:   mach.base.intern.Interner;
    by_path: std.collections.map.Map[mach.base.intern.StrId, FileId];
}
```

the files of one compilation, addressed by FileId

the store is a handle.StableChunks: a SourceFile is written once into a
fixed chunk and never moves or is reused, so a `*SourceFile` taken from
`get` stays valid until `dnit`. slot 0 is the FILE_NIL sentinel. growing
the map moves no file, and updating a file's text or releasing its payload
mutates the file in place and bumps its `revision`. a slot is never handed to a second file
without a generation check on that revision. the map names its paths in its own
interner, so every session that shares it finds a file by the same path

### init

```mach
pub fun init(a: *std.allocator.Allocator) SourceMap
```

an empty source map, released with dnit

### dnit

```mach
pub fun dnit(m: *SourceMap)
```

release a source map and its files; a nil map holds nothing

### count

```mach
pub fun count(m: *SourceMap) usize
```

the slot count including the FILE_NIL sentinel; the next FileId to be issued

### add

```mach
pub fun add(m: *SourceMap, path: str, text: str) res[FileId, Error]
```

add a file holding a copy of `text` under a copy of `path`. the file is not indexed by its
path, so `find` and `load` do not see it

| Parameter | |
| :--- | :--- |
| `m` | the map |
| `path` | the path the file is named by |
| `text` | its text |
| **returns** | the new file's id, or exhausted when no id is left, or the store's or allocator's refusal |

### prepare_load

```mach
pub fun prepare_load(m: *SourceMap, path: str, text: str) res[PreparedLoad, Error]
```

the caller exclusively borrows the map until commit or discard; a new entry
reserves its slot here so commit_load cannot fail

### load

```mach
pub fun load(m: *SourceMap, path: str, text: str) res[FileId, Error]
```

the file at `path` holding `text`: a path the map holds keeps its id and takes the text, a new
one is added

### prepare_release

```mach
pub fun prepare_release(m: *SourceMap, id: FileId) err[Error]
```

preflight completes before query and editor owners are retired

### payload_dnit

```mach
pub fun payload_dnit(m: *SourceMap, id: FileId)
```

free a file's text and line index in place and bump its revision. the slot
keeps its path and identity: it is never moved, compacted or reissued to
another file, so a `*SourceFile` held across the release stays valid and
reads `present == false`. a future reuse of released slots must check the
revision as a generation before trusting a stored FileId

### find

```mach
pub fun find(m: *SourceMap, path: str) FileId
```

the file loaded from `path`, FILE_NIL when none was

### get

```mach
pub fun get(m: *SourceMap, id: FileId) opt[*SourceFile]
```

the file behind an id; none for FILE_NIL or an id the map never issued

the returned `*SourceFile` is stable until `dnit`: the store never moves a
file when the map grows, and a released slot keeps its address. the pointer
may be held across later loads; `present` and `revision` tell the holder
whether the payload behind it changed

## PreparedLoad

```mach
pub rec PreparedLoad {
    owner:     *SourceMap;
    id:        FileId;
    key:       mach.base.intern.StrId;
    file:      SourceFile;
    changed:   bool;
    new_entry: bool;
    editor:    mach.base.handle.ChunkEditor[SourceFile];
}
```

a staged publication; `editor` holds the store's exclusive editor over the
reserved slot of a new entry until commit or discard

### discard_load

```mach
pub fun discard_load(prepared: *PreparedLoad)
```

release a load that was prepared and not committed; one already committed or discarded
owns nothing

### commit_load

```mach
pub fun commit_load(prepared: *PreparedLoad) FileId
```

an existing file takes the staged payload in place; a new one lands in the
slot prepare_load reserved. neither moves any other file. total: the staged
file is the map's own, so its loss stops the compiler

## Functions

### line_break_len

```mach
pub fun line_break_len(c: u8, next: u8) usize
```

the bytes of a line break that starts with byte `c`, `next` being the byte after it: a
line feed, a carriage return and line feed, or a carriage return alone; 0 for any other
byte. the one rule every line count follows

