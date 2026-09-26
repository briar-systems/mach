# mach.cli.exit

## val OK

```mach
pub val OK: i64 = 0
```

the command did what it was asked; help renders every shared code from SHARED and
render_fail maps a failure to one through `of`, so the two cannot disagree

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

the compiler failed, whatever the input: a bug or an exhausted allocator

## val ENVIRONMENT

```mach
pub val ENVIRONMENT: i64 = 3
```

the machine refused: a file, directory, process or network operation failed

## val TIMEOUT

```mach
pub val TIMEOUT: i64 = 124
```

`mach run` stopped the program at its --timeout; 124 as GNU timeout reports it,
apart from ENVIRONMENT so a script can tell the two apart

## val PASSTHROUGH

```mach
pub val PASSTHROUGH: i64 = -1
```

the note code of a command that forwards another process's exit code as its own

## rec Note

```mach
pub rec Note;
```

one exit code as help documents it

code: the process exit code, or PASSTHROUGH for the codes a command forwards; a
         command note whose code is shared refines that code's meaning, never replaces it
meaning: what the code reports

## val SHARED_N

```mach
pub val SHARED_N: usize = 4
```

length of SHARED

## val SHARED

```mach
pub val SHARED: [SHARED_N]Note = [SHARED_N]Note;
```

the codes every command can exit with, in ascending order

## fun shared

```mach
pub fun shared(code: i64) bool;
```

whether `code` is one of SHARED

## fun of

```mach
pub fun of(f: outcome.Fail) i64;
```

the exit code a failure maps to

f: the failure
ret: USER for reported and user failures, ENVIRONMENT for environment, INTERNAL for internal

## fun notes_valid

```mach
pub fun notes_valid(notes: *Note, n: usize) bool;
```

whether a command's own notes keep to the shared contract: each has a meaning, a
code that is PASSTHROUGH, shared, or a free code in 4..255, and no two share a code

notes: the command's notes; nil when n is 0
n: length of notes

