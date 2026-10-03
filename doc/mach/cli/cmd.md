# mach.cli.cmd

## val COMMANDS_N

```mach
pub val COMMANDS_N: usize = 11
```

length of COMMANDS; the compiler refuses a COMMANDS literal of any other length

## val COMMANDS

```mach
pub val COMMANDS: [COMMANDS_N]*args.CommandSpec = [COMMANDS_N]*args.CommandSpec;
```

every command, in the order help lists them. a command is its own file's args.CommandSpec and
its row here; nothing else registers it

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

