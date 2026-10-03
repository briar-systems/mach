# mach.lang.source

## rec FileId

```mach
pub rec FileId;
```

a file's slot in the source map

every other id's nil is all ones. a file id's nil is 0, beside the register
ids the second such exception: a zero-filled location is nowhere, so records
that carry one are blank when zeroed. slot 0 of the map is never issued.

## val FILE_NIL

```mach
pub val FILE_NIL: FileId = FileId;
```

no source: an unlocated diagnostic, or a node the build synthesized

## fun file_id

```mach
pub fun file_id(index: u32) FileId;
```

## fun file_index

```mach
pub fun file_index(f: FileId) u32;
```

## fun file_same

```mach
pub fun file_same(left: FileId, right: FileId) bool;
```

## fun file_is_nil

```mach
pub fun file_is_nil(f: FileId) bool;
```

## rec Span

```mach
pub rec Span;
```

the bytes [offset, offset + len) of one text

## rec Location

```mach
pub rec Location;
```

where in the compilation something is: a span of the file `file`, or
nowhere when `file` is FILE_NIL

## fun span

```mach
pub fun span(offset: usize, len: usize) Span;
```

## fun span_between

```mach
pub fun span_between(offset: usize, end: usize) Span;
```

the bytes [offset, end) as a span; an end before the offset is empty

## fun span_end

```mach
pub fun span_end(s: Span) usize;
```

the offset just past the span's last byte

## fun location

```mach
pub fun location(file: FileId, s: Span) Location;
```

## fun location_nil

```mach
pub fun location_nil() Location;
```

## fun location_is_valid

```mach
pub fun location_is_valid(l: Location) bool;
```

## fun location_equals

```mach
pub fun location_equals(x: Location, y: Location) bool;
```

## rec Position

```mach
pub rec Position;
```

a line and a column, each from 1, the column counted in UTF-8 bytes

## rec Lines

```mach
pub rec Lines;
```

the line index of one text: where each line starts, for mapping an offset
to its line and column in time logarithmic in the text's lines. a line ends
at a newline byte

starts: the offset of each line's first byte, the first line's 0
len: how many lines, at least 1 once built
n: the text's length in bytes

## tag Error

```mach
pub tag Error: u8 {
    alloc: A.Error;
    store: handle.Error;
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

## fun text

```mach
pub fun text(e: Error) str;
```

## rec SourceFile

```mach
pub rec SourceFile;
```

## rec SourceMap

```mach
pub rec SourceMap;
```

the files of one compilation, addressed by FileId

the store is a handle.StableChunks: a SourceFile is written once into a
fixed chunk and never moves or is reused, so a `*SourceFile` taken from
`get` stays valid until `dnit`. slot 0 is the FILE_NIL sentinel. growing
the map, updating a file's text or releasing its payload mutates the file
in place and bumps its `revision`; a slot is never handed to a second file
without a generation check on that revision

## fun init

```mach
pub fun init(a: *A.Allocator) SourceMap;
```

## fun dnit

```mach
pub fun dnit(m: *SourceMap);
```

## fun count

```mach
pub fun count(m: *SourceMap) usize;
```

the slot count including the FILE_NIL sentinel; the next FileId to be issued

## fun add

```mach
pub fun add(m: *SourceMap, path: str, text: str) res[FileId, Error];
```

## rec PreparedLoad

```mach
pub rec PreparedLoad;
```

a staged publication; `editor` holds the store's exclusive editor over the
reserved slot of a new entry until commit or discard

## fun prepare_load

```mach
pub fun prepare_load(m: *SourceMap, interner: *intern.Interner, path: str, text: str) res[PreparedLoad, Error];
```

the caller exclusively borrows the map until commit or discard; a new entry
reserves its slot here so commit_load cannot fail

## fun discard_load

```mach
pub fun discard_load(prepared: *PreparedLoad);
```

## fun commit_load

```mach
pub fun commit_load(prepared: *PreparedLoad) FileId;
```

an existing file takes the staged payload in place; a new one lands in the
slot prepare_load reserved. neither moves any other file. total: the staged
file is the map's own, so its loss stops the compiler

## fun prepare_release

```mach
pub fun prepare_release(m: *SourceMap, id: FileId) err[Error];
```

preflight completes before query and editor owners are retired

## fun release_payload

```mach
pub fun release_payload(m: *SourceMap, id: FileId);
```

free a file's text and line index in place and bump its revision. the slot
keeps its path and identity: it is never moved, compacted or reissued to
another file, so a `*SourceFile` held across the release stays valid and
reads `present == false`. a future reuse of released slots must check the
revision as a generation before trusting a stored FileId

## fun get

```mach
pub fun get(m: *SourceMap, id: FileId) opt[*SourceFile];
```

the file behind an id; none for FILE_NIL or an id the map never issued

the returned `*SourceFile` is stable until `dnit`: the store never moves a
file when the map grows, and a released slot keeps its address. the pointer
may be held across later loads; `present` and `revision` tell the holder
whether the payload behind it changed

## fun copy_file

```mach
pub fun copy_file(file: *SourceFile, a: *A.Allocator) res[SourceFile, Error];
```

## fun dnit_file

```mach
pub fun dnit_file(file: *SourceFile, a: *A.Allocator);
```

## fun snapshot_from

```mach
pub fun snapshot_from(src: *SourceMap, a: *A.Allocator) res[*SourceMap, Error];
```

## fun position

```mach
pub fun position(file: *SourceFile, offset: usize) Position;
```

the line and column of `offset` in the file; an offset past the end is
placed at the end

## fun line_start

```mach
pub fun line_start(file: *SourceFile, line: usize) opt[usize];
```

the offset line `line` (from 1) starts at; reached only by mach-lsp

## fun line_bounds

```mach
pub fun line_bounds(file: *SourceFile, line: usize) opt[Span];
```

the bytes of line `line` (from 1) without its line ending

## fun lines_init

```mach
pub fun lines_init(a: *A.Allocator, data: *u8, n: usize) res[Lines, A.Error];
```

the line index of the `n` bytes at `data`; released with `lines_dnit`

## fun lines_dnit

```mach
pub fun lines_dnit(a: *A.Allocator, l: *Lines);
```

## fun lines_position

```mach
pub fun lines_position(l: *Lines, offset: usize) Position;
```

the line and column of `offset`, clamped to the text's end

