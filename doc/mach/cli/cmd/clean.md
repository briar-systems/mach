# mach.cli.cmd.clean

## fun run

```mach
pub fun run(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach clean`: remove every output path the manifest can produce for every declared target
and profile. the obj, ir, asm, and test roots, the compiler cache and step staging
directories, and the binary path of each artifact cell
are collected, sorted, collapsed under their ancestors, and removed inside the project
root; a path that escapes the root or crosses a symlink is refused. a missing path is
not an error; "nothing to clean" prints when nothing was removed

argv: the full process arguments
inv: the parsed invocation for this command
ret: exit.OK, or the shared code of the failure: exit.USER for a manifest error or an escaping
      target, exit.INTERNAL for an allocator failure, exit.ENVIRONMENT for a read or removal failure

