# mach.cli.cmd.clean

## val COMMAND

```mach
pub val COMMAND: std_args.CommandSpec = std_args.CommandSpec;
```

`mach clean`

## fun run

```mach
pub fun run(cx: *args.Call) res[i64, fail.Fail];
```

`mach clean`: remove every path a build of every cell writes, as the build
plans it: each artifact's output and the object, IR, assembly, test, cache,
staging and dependency trees beneath each cell's work directory, dependency
requirement cells included; a dependency never realized was never built, so
it adds no cell. they are sorted, collapsed under their ancestors,
and removed inside the project root; a path that escapes the root or crosses a symlink is refused. a missing path is
not an error; "nothing to clean" prints when nothing was removed

cx: the call
ret: exit.OK, or the shared code of the failure: exit.USER for a manifest error or an escaping
     target, exit.INTERNAL for an allocator failure, exit.ENVIRONMENT for a read or removal
     failure; a removal that fails is reported and the rest are still attempted

