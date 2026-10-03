# mach.cli.util

## val PROJECT_CONFIG_NAME

```mach
pub val PROJECT_CONFIG_NAME: str = manifest.MANIFEST_FILE
```

the manifest file name, "mach.toml"

## fun flag_exists

```mach
pub fun flag_exists(argc: usize, argv: *str, flag: str) bool;
```

whether a flag occurs anywhere in argv

argc: number of entries
argv: the arguments
flag: the exact spelling to find
ret: true on an exact match

## fun flag_value

```mach
pub fun flag_value(argc: usize, argv: *str, flag: str) opt[str];
```

the value following the first occurrence of a flag

argc: number of entries
argv: the arguments
flag: the exact spelling to find
ret: the argument after the first match; none when the flag is absent or is the last argument

## fun arg_is_flag

```mach
pub fun arg_is_flag(s: str) bool;
```

whether an argument starts with '-'; nil is not a flag, and a lone '-' is the
conventional name of the standard stream rather than an option

s: the argument
ret: true for a leading '-' followed by anything

## fun arg_separator_index

```mach
pub fun arg_separator_index(argc: usize, argv: *str) usize;
```

the index of the first "--" at or after argv[1]

argc: number of entries
argv: the arguments
ret: the index, or argc when there is none

## rec ProjectLocation

```mach
pub rec ProjectLocation;
```

a resolved project operand; every string is owned and freed by dnit_project_location

root: the project directory
manifest: the path of its mach.toml

## fun dnit_project_location

```mach
pub fun dnit_project_location(a: *A.Allocator, loc: *ProjectLocation);
```

free the two strings of a location; nil strings are skipped

a: the allocator resolve_project_location was given
loc: the location

## fun resolve_project_location

```mach
pub fun resolve_project_location(a: *A.Allocator, arg: str) res[ProjectLocation, outcome.Fail];
```

turn a project operand into a root directory and a manifest path
a directory operand must contain mach.toml; a file operand must itself be named mach.toml,
and its parent becomes the root

a: allocator for the returned strings
arg: the operand; nil is an error asking for a path
ret: the location, freed with dnit_project_location, or a message naming what was wrong

## fun ensure_parents

```mach
pub fun ensure_parents(a: *A.Allocator, p: str) err[outcome.Fail];
```

create every missing ancestor of a path with mode 0755; the path itself is not created

a: allocator for the parent path
p: the path whose parents are wanted
ret: none on success, or the parent computation or creation error

## fun resolve_cmd

```mach
pub fun resolve_cmd(a: *A.Allocator, name: str) res[str, outcome.Fail];
```

locate an executable the way a shell would: a name with a path separator is used directly
and must be an existing executable file; any other name is searched on PATH

a: allocator for the returned path
name: the program name or path
ret: the resolved path, owned by the caller; "PATH unset", "not found on PATH", or the
      direct-path error

