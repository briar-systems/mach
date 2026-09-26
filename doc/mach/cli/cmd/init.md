# mach.cli.cmd.init

## fun run

```mach
pub fun run(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach init`: scaffold a project. the positional names the directory (default the working
directory) and, unless `--name` is given, the project id, which must be a portable
identifier. an absent directory is created whole; an existing one is filled in, and
`--force` allows overwriting mach.toml and the entry file. `--lib` writes src/lib.mach
and a static artifact instead of src/main.mach and binary artifacts. the std dependency
is then added at the caret range of the release resolution picks and checked out, unless
`--no-deps`, which still resolves the range (so needs the network) but skips the checkout.
`--quiet` silences progress

argv: the full process arguments
inv: the parsed invocation for this command
ret: exit.OK, exit.USER for an invalid id or a refused overwrite, otherwise the code
      `exit.of` maps the printed failure to, the std dependency's own when it could not be added

