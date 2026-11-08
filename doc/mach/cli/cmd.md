# mach.cli.cmd

## fun dispatch

```mach
pub fun dispatch(argc: usize, argv: **u8) i64;
```

the `mach` entry point: route the arguments to one command and return its exit code. `--help`
renders the page of the command it follows in place of running it, whatever else the line is
refused for but an unknown command, and `--version` before a command prints the version alone.
with no command the overview prints and the code is 1, and an unknown command is refused with
the overview after it; otherwise args.run reports a refusal or a refused combination, or runs
the command

argc: process argument count
argv: process arguments, argv[0] the program
ret: the exit code: exit.USER for a missing or unknown command, the code `exit.of` maps a
      failure to set up or read the line to, otherwise the command's own code

