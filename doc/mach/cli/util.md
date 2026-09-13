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

## fun flag_mark

```mach
pub fun flag_mark(marks: *bool, argc: usize, argv: *str, flag: str) bool;
```

find every occurrence of a flag and mark its index

marks: one bool per argv entry, set true at each occurrence; nil to only test presence
argc: number of entries
argv: the arguments
flag: the exact spelling to find
ret: true when the flag occurred at least once

## fun flag_mark_value

```mach
pub fun flag_mark_value(marks: *bool, argc: usize, argv: *str, flag: str) opt[str];
```

find every occurrence of a value flag, mark each flag and its following argument, and
return the first value

marks: one bool per argv entry; the flag and the argument after it are marked; nil to skip marking
argc: number of entries
argv: the arguments
flag: the exact spelling to find
ret: the argument after the first occurrence that has one; none when the flag is absent or
       every occurrence is last

## fun arg_is_flag

```mach
pub fun arg_is_flag(s: str) bool;
```

whether an argument starts with '-'; nil is not a flag

s: the argument
ret: true for a leading '-'

## fun arg_separator_index

```mach
pub fun arg_separator_index(argc: usize, argv: *str) usize;
```

the index of the first "--" at or after argv[1]

argc: number of entries
argv: the arguments
ret: the index, or argc when there is none

## fun arg_index_positional

```mach
pub fun arg_index_positional(argc: usize, argv: *str, marks: *bool, start: usize) usize;
```

the first unmarked argument at or after start that is not a flag

argc: number of entries
argv: the arguments
marks: one bool per argv entry, true where an option or its value sits
start: first index to inspect
ret: the index, or argc when there is none

## fun flag_reject_unknown

```mach
pub fun flag_reject_unknown(argc: usize, argv: *str, marks: *bool, start: usize, cmd: str) bool;
```

print "error: unknown flag" for the first unmarked flag at or after start

argc: number of entries
argv: the arguments
marks: one bool per argv entry, true where an option or its value sits
start: first index to inspect
cmd: the command name printed in the message
ret: true when an unknown flag was found and reported

## def OperandClass

```mach
pub def OperandClass: u8
```

how the project operand was given

## val OPERAND_DIRECTORY

```mach
pub val OPERAND_DIRECTORY: OperandClass = 0
```

a directory containing mach.toml

## val OPERAND_MANIFEST

```mach
pub val OPERAND_MANIFEST: OperandClass = 1
```

the path of the mach.toml itself

## rec ProjectLocation

```mach
pub rec ProjectLocation;
```

a resolved project operand; every string is owned and freed by dnit_project_location

root: the project directory
manifest: the path of its mach.toml
display: the operand as the user typed it
class: OPERAND_DIRECTORY or OPERAND_MANIFEST

## fun dnit_project_location

```mach
pub fun dnit_project_location(a: *A.Allocator, loc: *ProjectLocation);
```

free the three strings of a location; nil strings are skipped

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

## fun ensure_dir

```mach
pub fun ensure_dir(p: str) bool;
```

create a directory with mode 0755 when it does not exist; not recursive

p: the directory path
ret: true when the directory exists afterwards

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

## fun resolve_cmd_in

```mach
pub fun resolve_cmd_in(a: *A.Allocator, name: str, search: str) res[str, outcome.Fail];
```

locate an executable through an explicit search list
on windows the list separator is ';' and a name without a '.' is tried with each PATHEXT
extension (default .COM;.EXE;.BAT;.CMD); elsewhere the separator is ':' and the name is
tried as given. a name with a path separator is used directly, as in resolve_cmd

a: allocator for the returned path
name: the program name or path
search: the separator-joined directories; empty segments are skipped
ret: the first existing executable candidate, owned by the caller; "empty program name",
        "not found on PATH", or the direct-path error

