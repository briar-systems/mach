# mach.cli.util

## val PROJECT_CONFIG_NAME

```mach
pub val PROJECT_CONFIG_NAME: str = manifest.MANIFEST_FILE
```

the manifest file name, "mach.toml"

## rec ProjectLocation

```mach
pub rec ProjectLocation;
```

a resolved project operand; every string is owned and freed by dnit_project_location

root: the project directory
manifest: the path of its mach.toml

## fun resolve_project_location

```mach
pub fun resolve_project_location(a: *A.Allocator, arg: str) res[ProjectLocation, fail.Fail];
```

turn a project operand into a root directory and a manifest path
a directory operand must contain mach.toml; a file operand must itself be named mach.toml,
and its parent becomes the root

a: allocator for the returned strings
arg: the operand; nil is an error asking for a path
ret: the location, freed with dnit_project_location, or a message naming what was wrong

## fun ensure_parents

```mach
pub fun ensure_parents(a: *A.Allocator, p: str) err[fail.Fail];
```

create every missing ancestor of a path with mode 0755; the path itself is not created

a: allocator for the parent path
p: the path whose parents are wanted
ret: none on success, or the parent computation or creation error

