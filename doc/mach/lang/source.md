# mach.lang.source

## def FileId

```mach
pub def FileId: u32
```

## val FILE_NIL

```mach
pub val FILE_NIL: FileId = 0
```

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

## fun init

```mach
pub fun init(a: *A.Allocator) SourceMap;
```

## fun dnit

```mach
pub fun dnit(m: *SourceMap);
```

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

## fun prepare_load

```mach
pub fun prepare_load(m: *SourceMap, interner: *intern.Interner, path: str, text: str) res[PreparedLoad, fail.Fail];
```

the caller exclusively borrows the map until commit or discard

## fun discard_load

```mach
pub fun discard_load(prepared: *PreparedLoad);
```

## fun commit_load

```mach
pub fun commit_load(prepared: *PreparedLoad) FileId;
```

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

## fun get

```mach
pub fun get(m: *SourceMap, id: FileId) opt[*SourceFile];
```

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

