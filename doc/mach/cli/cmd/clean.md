# mach.cli.cmd.clean

## def EntryKind

```mach
pub def EntryKind: u8
```

what a clean entry names

## val ENTRY_DIR

```mach
pub val ENTRY_DIR: EntryKind = 0
```

a directory tree

## val ENTRY_FILE

```mach
pub val ENTRY_FILE: EntryKind = 1
```

a single file

## rec CleanEntry

```mach
pub rec CleanEntry;
```

one path `mach clean` removes

rel: the path relative to the project root, as the manifest templates expand it
kind: ENTRY_DIR or ENTRY_FILE

## fun run

```mach
pub fun run(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach clean`: remove every output path the manifest can produce for every declared target
and profile. the obj, ir, asm, and test roots and the binary path of each artifact cell
are collected, sorted, collapsed under their ancestors, and removed inside the project
root; a path that escapes the root or crosses a symlink is refused. a missing path is
not an error; "nothing to clean" prints when nothing was removed

argv: the full process arguments
inv: the parsed invocation for this command
ret: 0 success, 1 manifest error, 2 allocator, read, or removal failure

