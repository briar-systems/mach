# mach.cli.cmd.run

## fun run

```mach
pub fun run(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach run`: execute the project's already-built binary, forwarding the arguments after
`--` to it. `--emit` and build-only options are refused; `--runner <cmd>` executes
`<cmd> <binary> <args>` instead, which is required when the target is not runnable on
this host; `--timeout <duration>` runs the program in its own process group and
terminates it after the duration

argv: the full process arguments
inv: the parsed invocation for this command
ret: the program's exit code; 128 plus the signal number when it was killed by a signal;
      exit.TIMEOUT when the timeout expired; otherwise the shared code of mach's own failure:
      exit.USER for a usage error or a missing or non-executable artifact, exit.ENVIRONMENT
      for a spawn or wait failure, exit.INTERNAL out of memory

