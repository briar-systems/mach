# mach.cli.cmd.migrate

## val COMMAND

```mach
pub val COMMAND: args.CommandSpec = args.CommandSpec;
```

`mach migrate`

## fun run

```mach
pub fun run(cx: *args.Call) res[i64, fail.Fail];
```

`mach migrate [<path>] [--check]`

cx: the call
ret: exit.OK when the project is in this major's spelling (or was made so), exit.USER when a
     file would change under --check, otherwise the failure

