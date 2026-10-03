# mach.cli.cmd

## val COMMANDS_N

```mach
pub val COMMANDS_N: usize = 11
```

length of COMMANDS

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

ret: COMMANDS as a set

## fun dispatch

```mach
pub fun dispatch(argc: usize, argv: **u8) i64;
```

the `mach` entry point: route argv to one command and return its exit code
help requests are rendered first; with no arguments the overview prints and the code is 1;
an unknown command prints an error and the overview; an invocation that breaks its command's
schema is refused; otherwise the command's handler runs

argc: process argument count
argv: process arguments, argv[0] the program
ret: the exit code: exit.USER for a missing or unknown command, the code `exit.of` maps a
      failure to set up or read the invocation to, otherwise the command's own code

