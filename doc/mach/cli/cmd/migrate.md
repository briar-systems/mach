# mach.cli.cmd.migrate

## val COMMAND

```mach
pub val COMMAND: std_args.Command = std_args.Command;
```

`mach migrate`

## val POLICY

```mach
pub val POLICY: args.Policy = args.Policy;
```

what mach holds `mach migrate` to beyond the parse

## fun run

```mach
pub fun run(cx: *args.Call) res[i64, fail.Fail];
```

`mach migrate [<path>] [--check]`

cx: the call
ret: exit.OK when the project is in this major's spelling (or was made so), exit.USER when a
     file would change under --check, or in either mode when the manifest still writes a
     removed key no rewrite migrates or a source could not be read as one; a script's or a
     step's removed option is reported and fails nothing. otherwise the failure

