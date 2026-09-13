# mach.cli.cmd.doc

## fun run

```mach
pub fun run(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach doc`: build the project's module graph and write one Markdown page per module plus
an index. pages go to `<root>/doc/api/<module path>.md`, or under `--out <dir>` relative
to the root, with a README.md index; every `pub` declaration is rendered, documented or
not. a summary line prints unless `--quiet`

argv: the full process arguments
inv: the parsed invocation for this command
ret: 0 success, 1 project or manifest error, 2 allocator, session, registry, or write failure

