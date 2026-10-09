# mach.cli.cmd.dep

## val COMMAND

```mach
pub val COMMAND: args.CommandSpec = args.CommandSpec;
```

`mach dep`

## fun run

```mach
pub fun run(cx: *args.Call) res[i64, fail.Fail];
```

`mach dep`: route to one action: list, add, remove, update, pull, verify or outdated, each a
command over the dependency manager (mach.lang.package) that renders what it reports

cx: the call, its action read and its operands counted
ret: exit.OK, the code `exit.of` maps the action's rendered failure to, or the failure that
     kept the action from starting

