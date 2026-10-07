# mach.lang.target.of.destination

destination: the files a writer produces, held in memory until its caller
publishes them through output. a writer asks here for the buffer of each file
it writes and fills it, so no writer opens a file, and a link that keeps its
image in memory takes the bytes instead of publishing them

## rec File

```mach
pub rec File;
```

one file a writer produces: the artifact itself, or a companion published
beside it, as an import library beside the library it binds

ext: empty for the artifact; a companion's extension, with its dot, which
      takes the place of the artifact's own
mode: the permissions the file is published with

## rec Destination

```mach
pub rec Destination;
```

## fun init

```mach
pub fun init(alloc: *A.Allocator) Destination;
```

## fun dnit

```mach
pub fun dnit(dst: *Destination);
```

## fun file_add

```mach
pub fun file_add(dst: *Destination, ext: str, len: usize, mode: i32) res[*u8, fail.Fail];
```

the zeroed buffer of `len` bytes the file under `ext` is written into, owned
by the destination; a writer adds each file once

## fun artifact

```mach
pub fun artifact(dst: *Destination) *File;
```

the artifact's file, nil before a writer adds it

## fun artifact_take

```mach
pub fun artifact_take(dst: *Destination, len: *usize) res[*u8, fail.Fail];
```

the artifact's bytes, which the caller then owns and releases with the
destination's allocator

## fun publish

```mach
pub fun publish(dst: *Destination, itn: *intern.Interner, path: str, op: str) err[fail.Fail];
```

publishes every file: the artifact at `path` and each companion beside it,
under the artifact's name with the companion's extension

op: the operation a failure names, as "cannot publish executable"

