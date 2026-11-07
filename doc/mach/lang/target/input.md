# mach.lang.target.input

link inputs: how a link token names a file or a library

a token names a file by its path, located here, or a library by name, which
mink's search finds when the link runs: the search its host row states for the
target's system, environment and architecture. mach looks for no library of
its own

## def Form

```mach
pub def Form: u8
```

how an input is named: a file at a path, or a library or a framework by
name that mink's search finds

## val FILE

```mach
pub val FILE:      Form = 0
```

## val LIBRARY

```mach
pub val LIBRARY:   Form = 1
```

## val FRAMEWORK

```mach
pub val FRAMEWORK: Form = 2
```

## def Kind

```mach
pub def Kind: u8
```

what a library named by name must be, judged by mink's search from what the
file holds. nothing else is taken in its place, so a wrong kind or a miss is
a refusal

## val SHARED

```mach
pub val SHARED: Kind = 0
```

## val STATIC

```mach
pub val STATIC: Kind = 1
```

## rec Input

```mach
pub rec Input;
```

a link input: a file at the path `text`, located, or a library or framework
of the name `text`. `key` is the library a provider is registered under, ""
for none, `include_referenced` whether the image loads it only when a live
import references it, and `kind` what a library by name must be, SHARED for
any other input

## fun token_is_path

```mach
pub fun token_is_path(tok: *u8) bool;
```

whether a token names a file by path rather than a library by name: it holds a
separator, or it ends the way a file any system of mink's catalog searches for
is named

## fun names_file

```mach
pub fun names_file(tok: *u8) bool;
```

whether a library's name names a file, which states its own kind: a path, or
a name ending in the suffix a system's naming states for the file its loader
loads, as `kernel32.dll` does

## fun token_input

```mach
pub fun token_input(a: *A.Allocator, root: str, tok: *u8) res[Input, fail.Fail];
```

the input a link token names: a path, located as written or under `root`,
or a library by name, left to mink's search

## fun framework_input

```mach
pub fun framework_input(name: *u8) Input;
```

a framework by name, left to mink's search

