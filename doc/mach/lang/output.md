# mach.lang.output

## val DIR_MODE

```mach
pub val DIR_MODE: i32 = 0o755
```

## fun intern_message

```mach
pub fun intern_message(itn: *intern.Interner, a: *A.Allocator, buf: *u8, total: usize, generic: str) str;
```

## fun io_message

```mach
pub fun io_message(itn: *intern.Interner, a: *A.Allocator, op: str, path: str, os_error: str, generic: str) str;
```

## fun directory

```mach
pub fun directory(a: *A.Allocator, root: str, dir: str) err[outcome.Fail];
```

the build owns the layout beneath its output root: `dir` and each missing
ancestor become directories, and beneath `root` an entry where one belongs
that does not resolve to a directory (a stale file, a dangling link) is
removed first, a link as the link and never its target. `root` itself and
what lies above it or outside it are created but never replaced, and a
symlink to a directory is used as it stands

## fun reserve

```mach
pub fun reserve(a: *A.Allocator, root: str, file: str) err[outcome.Fail];
```

ready `file` to be written: its parent is a directory as `directory` makes
it, and beneath `root` a stale directory at its own name is removed. a
symlink there is left for the write to follow

## fun bytes

```mach
pub fun bytes(itn: *intern.Interner, a: *A.Allocator, path: str, buf: *u8, len: usize, mode: i32,
op: str, generic: str) err[outcome.Fail];
```

write `buf` to `path`, whose parent exists

## fun writer

```mach
pub fun writer[W](itn: *intern.Interner, a: *A.Allocator, path: str, ctx: *W,
write_cb: fun(*W, *mwriter.Writer) err[mwriter.WriteError], mode: i32,
op: str, generic: str) err[outcome.Fail];
```

stream `path`, whose parent exists, through `write_cb`

## fun replace

```mach
pub fun replace(a: *A.Allocator, path: str, buf: *u8, len: usize, mode: i32) err[outcome.Fail];
```

replace a source-tree file through a sibling temporary and rename, so the
destination is either the old content or the complete new content

## fun mirror_fqn

```mach
pub fun mirror_fqn(a: *A.Allocator, base: *u8, fqn: str, suffix: str) res[str, outcome.Fail];
```

the file a module's output takes under `base`: its dotted name as a path,
with `suffix` as the extension

## fun through_temporary

```mach
pub fun through_temporary[C](a: *A.Allocator, destination: str, ctx: *C,
write: fun(*C, str) err[fail.Fail]) err[outcome.Fail];
```

`write` fills a sibling temporary of `destination`, which is then renamed
over it: a reader sees the previous file or the complete new one, never a
torn one, and a failed write leaves the previous file and no temporary

