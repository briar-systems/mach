# mach.cli.cmd.run

## val COMMAND

```mach
pub val COMMAND: std_args.Command = std_args.Command;
```

`mach run`

## val POLICY

```mach
pub val POLICY: args.Policy = args.Policy;
```

what mach holds `mach run` to beyond the parse

## fun run

```mach
pub fun run(cx: *args.Call) res[i64, fail.Fail];
```

`mach run`: execute the project's already-built binary, forwarding the arguments after
`--` to it. a target that declares a `runner` launches the expanded runner followed by
the arguments instead, and `--runner <cmd>` replaces it for this run with
`<cmd> <binary> <args>`; a target that is not the host's and has neither is refused.
`--timeout <duration>` runs the program in its own process group and terminates it
after the duration

cx: the call
ret: the program's exit code; 128 plus the signal number when it was killed by a signal;
     exit.TIMEOUT when the timeout expired; otherwise mach's own failure: a user failure for a
     usage error or an artifact that is missing or that the system refuses to execute (a
     malformed image fails the spawn on windows), an environment failure for a wait, an
     internal one out of memory

