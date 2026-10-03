# mach.cli.cmd.testing

## val COMMAND

```mach
pub val COMMAND: args.CommandSpec = args.CommandSpec;
```

`mach test`

## fun run

```mach
pub fun run(cx: *args.Call) res[i64, fail.Fail];
```

`mach test`: build the project's test dispatcher and run the collected tests. `--emit` is
refused; `--list` prints the collected tests and stops; `--diagnostics json` adds the run's
records to the report; `--filter`, `--runner`, `--timeout`, and `--jobs` shape the run; `-o`
names the dispatcher binary the way `mach build -o` names a built one, inside the project
root, so runs in one tree with different `-o` do not collide

cx: the call
ret: exit.OK when every test passed, exit.USER for at least one failed test, exit.INTERNAL for
     an infrastructure failure, otherwise the code `exit.of` maps the reported failure to

