# mach.cli.cmd.init

## val COMMAND

```mach
pub val COMMAND: args.CommandSpec = args.CommandSpec;
```

`mach init`

## fun run

```mach
pub fun run(cx: *args.Call) res[i64, fail.Fail];
```

`mach init`: scaffold a project. the positional names the directory (default the working
directory) and, unless `--name` is given, the project id, which must be a portable
identifier. an absent directory is created whole; an existing one is filled in, and
`--force` allows overwriting mach.toml and the entry file. `--lib` writes src/lib.mach
and a static artifact instead of src/main.mach and binary artifacts. the std dependency
is then added at the caret range of the release resolution picks and checked out, unless
`--no-deps`, which still resolves the range (so needs the network) but skips the checkout.
`--quiet` silences progress

cx: the call
ret: exit.OK, or the failure: a user one for an invalid id or a refused overwrite, otherwise the
     std dependency's own class when it could not be added

