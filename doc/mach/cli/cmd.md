# mach.cli.cmd

## fun commands

```mach
pub fun commands() args.CommandSet;
```

the command set argv is read against

ret: COMMANDS as a set, which help answers and GLOBALS extends

## fun dispatch

```mach
pub fun dispatch(argc: usize, argv: **u8) i64;
```

the `mach` entry point: route argv to one command and return its exit code. with no arguments
the overview prints and the code is 1; an unknown command is refused and the overview prints;
otherwise args.run reports a refusal, renders the page `--help` asks for, or runs the command

argc: process argument count
argv: process arguments, argv[0] the program
ret: the exit code: exit.USER for a missing or unknown command, the code `exit.of` maps a
      failure to set up or read the invocation to, otherwise the command's own code

