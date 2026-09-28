# mach.cli.cmd.doc

## fun run

```mach
pub fun run(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach doc`: build the project's module graph and write one Markdown page per module plus
an index. pages go to `<root>/doc/<module path>.md`, or under `--out <dir>` relative
to the root, with a README.md index. a page opens with the module docstring, and every
`pub` declaration and `fwd` is rendered, documented or not, a `fwd` with what it
forwards. a summary line prints unless `--quiet`

argv: the full process arguments
inv: the parsed invocation for this command
ret: exit.OK, or the shared code of the failure: exit.USER for a project or manifest error,
      exit.INTERNAL for an allocator, session or registry failure, exit.ENVIRONMENT for a write failure

