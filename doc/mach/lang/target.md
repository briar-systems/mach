# mach.lang.target

target resolution: a target request read against the owners' catalogs

a request names an architecture and a system as mink spells them, the
environment its manifest states, a calling convention, a baseline as masc
spells one and the extensions it adds. each part is looked up in its owner's
catalog: mink's architecture and system lists, the host mink states for them,
masc's extension catalog of the architecture, and the target mirl assembles
from the five. every fact the build reads of a target comes from those rows,
and mach states none of them

## fwd resolved.Target

```mach
fwd resolved.Target
```

forwards [`mach.lang.target.resolved.Target`](target/resolved.md#rec-target)

## rec TargetRequest

```mach
pub rec TargetRequest;
```

## fun target_request

```mach
pub fun target_request(arch_name: str, sys_name: str, abi_name: str) TargetRequest;
```

## fun with_isa

```mach
pub fun with_isa(req: *TargetRequest, isa_name: str);
```

## fun with_extensions

```mach
pub fun with_extensions(req: *TargetRequest, names: *str, count: u32);
```

## fun with_of

```mach
pub fun with_of(req: *TargetRequest, of_name: str);
```

## fun with_env

```mach
pub fun with_env(req: *TargetRequest, env_name: str, origin: str);
```

## fun with_image

```mach
pub fun with_image(req: *TargetRequest, base: u64, stack_reserve: u64, stack_commit: u64);
```

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

## fun host_request

```mach
pub fun host_request() TargetRequest;
```

the request a `native` selection resolves: the host's architecture, system and
convention as the compiler's own manifest names them. no environment is filled
in, since env comes from a manifest alone

## fun set_of

```mach
pub fun set_of(arch: *mink_arch.Arch) opt[*masc_set.Set];
```

masc's instruction set for `arch`, none where masc serves it with none

## fun resolve

```mach
pub fun resolve(a: *A.Allocator, req: *TargetRequest) res[resolved.Target, fail.Fail];
```

the target a request names, every fact of it read from its owners

## rec ArtifactName

```mach
pub rec ArtifactName;
```

the naming of an artifact's file, as mink's system names a file of its kind:
prefix, then the artifact's name, then the extension

prefix: written before its name
suffix: written after its name, with its dot

## fun output_of

```mach
pub fun output_of(t: *resolved.Target, kind: catalog_artifact.Kind) opt[*mink_output.Kind];
```

the output mink builds for an artifact of `kind` on the target: a finished module
for an executable of a format that finishes its modules, else a fixed-address
executable, a shared library, and for an object directory its relocatable objects.
none for a static library, which mink archives rather than links

## fun artifact_naming

```mach
pub fun artifact_naming(a: *A.Allocator, t: *resolved.Target, kind: catalog_artifact.Kind) res[ArtifactName, fail.Fail];
```

the name an artifact of `kind` takes on the target, mink's system's name of the
file: an archive for a static library, and for every other kind the kind of file
its output reads back as. refused, with mink's reason, where the system builds none

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

## rec Cell

```mach
pub rec Cell;
```

one target the new path builds: a host mink states, the convention and the floor
it runs under, as mirl assembles them

## fun cells

```mach
pub fun cells(a: *A.Allocator, out: *Vector[Cell]) err[fail.Fail];
```

every target the new path builds over its host's floor, in mink's system order: each
host row of each system, under the convention it calls by default, else each its
architecture's family names, that mirl assembles a target from. a host that states no
floor is a target only with the `isa` its chip implements, so it is not listed

