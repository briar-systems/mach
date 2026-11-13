# mach.cli.exit

## val OK

```mach
pub val OK: i64 = 0
```

the command did what it was asked

## val USER

```mach
pub val USER: i64 = 1
```

the invocation or the input the user controls is wrong: a bad flag, a rejected
manifest or source, a failure already reported through diagnostics

## val INTERNAL

```mach
pub val INTERNAL: i64 = 2
```

the compiler failed, whatever the input: a bug

## val ENVIRONMENT

```mach
pub val ENVIRONMENT: i64 = 3
```

the machine refused: storage, or a file, directory, process or network operation failed

## val TIMEOUT

```mach
pub val TIMEOUT: i64 = 124
```

`mach run` stopped the program at its --timeout; 124 as GNU timeout reports it,
apart from ENVIRONMENT so a script can tell the two apart

## fun of

```mach
pub fun of(f: fail.Fail) i64;
```

the exit code a failure maps to

f: the failure
ret: USER for reported and user failures, ENVIRONMENT for environment, INTERNAL for internal

## fun worst

```mach
pub fun worst(a: i64, b: i64) i64;
```

the worse of two shared codes, as a run over several cells reports them:
INTERNAL over ENVIRONMENT over USER over OK

