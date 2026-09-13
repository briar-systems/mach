# mach.cli.cmd.init

## fun run

```mach
pub fun run(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach init`: scaffold a project. the positional names the directory (default the working
directory) and, unless `--name` is given, the project id, which must be a portable
identifier. an absent directory is created whole; an existing one is filled in, and
`--force` allows overwriting mach.toml and the entry file. `--lib` writes src/lib.mach
and a static artifact instead of src/root.mach and binary artifacts. the std dependency
is then realized as a git submodule unless `--no-deps`; `--quiet` silences progress

argv: the full process arguments
inv: the parsed invocation for this command
ret: 0 success, 1 an invalid id, a refused overwrite, or a failed publish, 2 allocator or
      filesystem failure, 3 the std dependency could not be realized

