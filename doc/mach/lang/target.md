# mach.lang.target

what a target builds: the host the compiler names, mink's products and linkages, the
name an artifact's file takes and what the target's object format holds

a target is resolved through mink's catalog and masc's check (mach.lang.target.resolved),
and every fact read here is read from that resolution's rows

## fwd resolved.Target

```mach
fwd resolved.Target
```

forwards [`mach.lang.target.resolved.Target`](target/resolved.md#rec-target)

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
pub fun linkage_of(a: *A.Allocator, t: *resolved.Target, kind: diagnostic_kind.Kind) res[*Linkage, fail.Fail];
```

the linkage of an executable built for `t`: the one its narrowing names, else the host's
default. refused in mink's words, as a user failure of `kind`, where the host runs none

## fun is_library

```mach
pub fun is_library(p: *Product) bool;
```

whether `p` is a library, which a consumer links against rather than runs: an archive, or an
image of a kind of file a link binds against

## fun is_executable

```mach
pub fun is_executable(p: *Product) bool;
```

whether `p` is linked as an executable, of the image kind its linkage names

## fun output_of

```mach
pub fun output_of(p: *Product, linkage: *Linkage) opt[*mink_output.Kind];
```

the image kind mink builds for a product: an executable's from its linkage, an image product's
its own. none for a static library, which mink archives rather than links

## fun executable_output

```mach
pub fun executable_output(a: *A.Allocator, t: *resolved.Target) res[*mink_output.Kind, fail.Fail];
```

the image kind of the executable a build for `t` links, refused where the host runs no
executable

## fun artifact_naming

```mach
pub fun artifact_naming(a: *A.Allocator, t: *resolved.Target, p: *Product) res[ArtifactName, fail.Fail];
```

the name an artifact of product `p` takes on the target, mink's system's name of the file:
an archive for a static library, and for every other product the kind of file its image
reads back as, an executable's under the linkage the target resolved to. refused, with
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

