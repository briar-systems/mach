# mach.cli.cmd

## fun dispatch

```mach
pub fun dispatch(argc: usize, argv: **u8) i64;
```

the `mach` entry point: route argv to one command and return its exit code
help requests are rendered first; with no arguments the overview prints and the code is 1;
an unknown command prints an error and the overview

argc: process argument count
argv: process arguments, argv[0] the program
ret: the exit code: 0 success, 1 usage or user error, 2 when the allocator or the
      invocation could not be set up, otherwise the command's own code

