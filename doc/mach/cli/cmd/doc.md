# mach.cli.cmd.doc

## val COMMAND

```mach
pub val COMMAND: args.CommandSpec = args.CommandSpec;
```

`mach doc`

## fun run

```mach
pub fun run(cx: *args.Call) res[i64, fail.Fail];
```

`mach doc`: build the project's module graph and write one Markdown page per module plus
an index. pages go to `<root>/doc/<module path>.md`, or under `-o <dir>` inside the project
root, with a README.md index. a page opens with the module docstring, and every `pub`
declaration and `fwd` is rendered, documented or not, a `fwd` with what it forwards. a summary
line prints unless `--quiet`

cx: the call
ret: exit.OK, or the failure: a user one for a project, manifest or output path error, an
     internal one for an allocator, session or registry failure, an environment one for a write

