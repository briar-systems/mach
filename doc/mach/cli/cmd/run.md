# mach.cli.cmd.run

## val COMMAND

```mach
pub val COMMAND: args.CommandSpec = args.CommandSpec;
```

`mach run`

## fun run

```mach
pub fun run(cx: *args.Call) res[i64, fail.Fail];
```

`mach run`: execute the project's already-built binary, forwarding the arguments after
`--` to it; `--runner <cmd>` executes
`<cmd> <binary> <args>` instead, which is required when the target is not runnable on
this host; `--timeout <duration>` runs the program in its own process group and
terminates it after the duration

cx: the call
ret: the program's exit code; 128 plus the signal number when it was killed by a signal;
     exit.TIMEOUT when the timeout expired; otherwise mach's own failure: a user failure for a
     usage error or an artifact that is missing or that the system refuses to execute (a
     malformed image fails the spawn on windows), an environment failure for a wait, an
     internal one out of memory

