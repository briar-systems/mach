# mach.lang.target

what a target builds: the host the compiler names, mink's output kinds as an artifact and a
linkage select them, the name an artifact's file takes and what the target's object format
holds

a target is resolved through mink's catalog and masc's check (mach.lang.target.resolved),
and every fact read here is read from that resolution's rows

## fun host_arch_name

```mach
pub fun host_arch_name() str;
```

the architecture the compiler itself was built for, as its manifest names it

## fun host_sys_name

```mach
pub fun host_sys_name() str;
```

the system the compiler itself was built for, as its manifest names it

## fun host_abi_name

```mach
pub fun host_abi_name() str;
```

the calling convention the compiler itself was built for, as its manifest names it

## fun host

```mach
pub fun host(a: *A.Allocator) res[resolved.Target, fail.Fail];
```

the target a `native` selection resolves: the host's architecture, system and convention
as the compiler's own manifest names them, every other part mink's default

## rec ArtifactName

```mach
pub rec ArtifactName;
```

the naming of an artifact's file, as mink's system names a file of its kind:
prefix, then the artifact's name, then the extension

prefix: written before its name
suffix: written after its name, with its dot

## fun linkage_of

```mach
pub fun linkage_of(a: *A.Allocator, t: *resolved.Target, kind: diagnostic_kind.Kind) res[*mink_output.Kind, fail.Fail];
```

the linkage of an executable built for `t`, one of mink's executable output kinds: the one
its narrowing names, else the host's default. refused in mink's words, as a user failure of
`kind`, where the host runs none

## fun is_library

```mach
pub fun is_library(k: Spelled) bool;
```

whether `k` is a library, which a consumer links against rather than runs: an archive, or an
image a loader binds into another

## fun per_module

```mach
pub fun per_module(k: Spelled) bool;
```

whether `k` is each module's object, written under the obj directory for a later link, so an
artifact of it delivers no one file at an `out` path

## fun is_executable

```mach
pub fun is_executable(k: Spelled) bool;
```

whether `k` is linked as an executable, of the output kind its linkage names

## fun kind_name

```mach
pub fun kind_name(k: Spelled) str;
```

the name `--kind` and a manifest's `kind` spell `k` by

## fun is_row

```mach
pub fun is_row(k: Spelled, row: *mink_output.Kind) bool;
```

whether `k` is the one output kind `row`

## fun output_of

```mach
pub fun output_of(k: Spelled, linkage: *mink_output.Kind) *mink_output.Kind;
```

the output kind mink makes of `k`: an executable's from its linkage, every other its own row

## fun executable_output

```mach
pub fun executable_output(a: *A.Allocator, t: *resolved.Target) res[*mink_output.Kind, fail.Fail];
```

the output kind of the executable a build for `t` links, refused where the host runs no
executable

## fun artifact_naming

```mach
pub fun artifact_naming(a: *A.Allocator, t: *resolved.Target, k: Spelled) res[ArtifactName, fail.Fail];
```

the name an artifact of kind `k` takes on the target, mink's system's name of the file of
its output kind, an executable's under the linkage the target resolved to. refused, with
mink's reason, where the system builds none

## fun finished_modules

```mach
pub fun finished_modules(t: *resolved.Target) bool;
```

whether the target's object format holds each module as its own artifact, as a
SPIR-V module: mink builds a module of the format and no executable of it

## fun flat_image

```mach
pub fun flat_image(t: *resolved.Target) bool;
```

whether the target's object format is a flat image, mink's flat format

## fun states_subsystem

```mach
pub fun states_subsystem(t: *resolved.Target) bool;
```

whether the target's images state a subsystem, as mink's host of it names a
default one, so an artifact's `subsystem`, icon and manifest mean something

## fun carries_record

```mach
pub fun carries_record(t: *resolved.Target) bool;
```

whether a module's object is read again by a link or the cache, so it carries
mach's record section: every object but a flat image's and a finished module's

