# mach.lang.source

## def FileId

```mach
pub def FileId: u32
```

## val FILE_NIL

```mach
pub val FILE_NIL: FileId = 0
```

no source: an unlocated diagnostic, or a node the build synthesized

## rec SrcLoc

```mach
pub rec SrcLoc;
```

## fun loc

```mach
pub fun loc(file: FileId, offset: usize) SrcLoc;
```

## fun loc_nil

```mach
pub fun loc_nil() SrcLoc;
```

## fun loc_is_valid

```mach
pub fun loc_is_valid(l: SrcLoc) bool;
```

## rec Position

```mach
pub rec Position;
```

## rec LineSpan

```mach
pub rec LineSpan;
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
pub fun add(m: *SourceMap, path: str, text: str) res[FileId, fail.Fail];
```

## fun update

```mach
pub fun update(m: *SourceMap, id: FileId, text: str) res[bool, fail.Fail];
```

## rec PreparedLoad

```mach
pub rec PreparedLoad;
```

a staged publication; `editor` holds the store's exclusive editor over the
reserved slot of a new entry until commit or discard

## fun prepare_load

```mach
pub fun prepare_load(m: *SourceMap, interner: *intern.Interner, path: str, text: str) res[PreparedLoad, fail.Fail];
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
slot prepare_load reserved. neither moves any other file

## fun load

```mach
pub fun load(m: *SourceMap, interner: *intern.Interner, path: str, text: str) res[FileId, fail.Fail];
```

## fun prepare_release

```mach
pub fun prepare_release(m: *SourceMap, id: FileId) err[fail.Fail];
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
pub fun copy_file(file: *SourceFile, a: *A.Allocator) res[SourceFile, fail.Fail];
```

## fun dnit_file

```mach
pub fun dnit_file(file: *SourceFile, a: *A.Allocator);
```

## fun snapshot_from

```mach
pub fun snapshot_from(src: *SourceMap, a: *A.Allocator) res[*SourceMap, fail.Fail];
```

## fun position

```mach
pub fun position(file: *SourceFile, offset: usize) Position;
```

## fun line_start

```mach
pub fun line_start(file: *SourceFile, line: usize) opt[usize];
```

## fun line_bounds

```mach
pub fun line_bounds(file: *SourceFile, line: usize) opt[LineSpan];
```

