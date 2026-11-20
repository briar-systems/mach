[mach](README.md) / **target**

# mach.target

what a target builds: the host the compiler names, mink's output kinds as an artifact and a
linkage select them, the name an artifact's file takes and what the target's object format
holds

a target is resolved through mink's catalog and masc's check (mach.target.resolved),
and every fact read here is read from that resolution's rows

Source: [`src/target.mach`](../../src/target.mach)

Uses types from [`mach.base.diagnostic.kind`](base/diagnostic/kind.md), [`mach.base.fail`](base/fail.md), [`mach.target.resolved`](target/resolved.md).

| Type | | |
| :--- | :--- | :--- |
| [`ArtifactName`](#artifactname) | `rec` | the naming of an artifact's file, as mink's system names a file of its kind: prefix, then the artifact's name, then the extension |

| On `ArtifactName` | |
| :--- | :--- |
| [`artifact_naming`](#artifact_naming) | the name an artifact of kind `k` takes on the target, mink's system's name of the file of its output kind, an executable's under the linkage the target resolved to |

| Functions | |
| :--- | :--- |
| [`host_arch_name`](#host_arch_name) | the architecture the compiler itself was built for, as its manifest names it |
| [`host_sys_name`](#host_sys_name) | the system the compiler itself was built for, as its manifest names it |
| [`host_abi_name`](#host_abi_name) | the calling convention the compiler itself was built for, as its manifest names it |
| [`host`](#host) | the target a `native` selection resolves: the host's architecture, system and convention as the compiler's own manifest names them, every other part mink's default |
| [`linkage_of`](#linkage_of) | the linkage of an executable built for `t`, one of mink's executable output kinds: the one its narrowing names, else the host's default |
| [`is_library`](#is_library) | whether `k` is a library, which a consumer links against rather than runs: an archive, or an image a loader binds into another |
| [`per_module`](#per_module) | whether `k` is each module's object, written under the obj directory for a later link, so an artifact of it delivers no one file at an `out` path |
| [`is_executable`](#is_executable) | whether `k` is linked as an executable, of the output kind its linkage names |
| [`kind_name`](#kind_name) | the name `--kind` and a manifest's `kind` spell `k` by |
| [`is_row`](#is_row) | whether `k` is the one output kind `row` |
| [`output_of`](#output_of) | the output kind mink makes of `k`: an executable's from its linkage, every other its own row |
| [`executable_output`](#executable_output) | the output kind of the executable a build for `t` links, refused where the host runs no executable |
| [`finished_modules`](#finished_modules) | whether the target's object format holds each module as its own artifact, as a SPIR-V module: mink builds a module of the format and no executable of it |
| [`flat_image`](#flat_image) | whether the target's object format is a flat image, mink's flat format |
| [`states_subsystem`](#states_subsystem) | whether the target's images state a subsystem, as mink's host of it names a default one, so an artifact's `subsystem`, icon and manifest mean something |
| [`admits_subsystem`](#admits_subsystem) | whether the target's images may state the subsystem `s`, as mink's host of it admits |
| [`carries_record`](#carries_record) | whether a module's object is read again by a link or the cache, so it carries mach's record section: every object but a flat image's and a finished module's |

---

## ArtifactName

```mach
pub rec ArtifactName {
    prefix: str;
    suffix: str;
}
```

the naming of an artifact's file, as mink's system names a file of its kind:
prefix, then the artifact's name, then the extension

| Field | Type | |
| :--- | :--- | :--- |
| `prefix` | `str` | written before its name |
| `suffix` | `str` | written after its name, with its dot |

### artifact_naming

```mach
pub fun artifact_naming(a: *std.allocator.Allocator, t: *mach.target.resolved.Target, k: Spelled) res[ArtifactName, mach.base.fail.Fail]
```

the name an artifact of kind `k` takes on the target, mink's system's name of the file of
its output kind, an executable's under the linkage the target resolved to. refused, with
mink's reason, where the system builds none, and an executable on a target of finished
modules naming the kind it builds

## Functions

### host_arch_name

```mach
pub fun host_arch_name() str
```

the architecture the compiler itself was built for, as its manifest names it

### host_sys_name

```mach
pub fun host_sys_name() str
```

the system the compiler itself was built for, as its manifest names it

### host_abi_name

```mach
pub fun host_abi_name() str
```

the calling convention the compiler itself was built for, as its manifest names it

### host

```mach
pub fun host(a: *std.allocator.Allocator) res[mach.target.resolved.Target, mach.base.fail.Fail]
```

the target a `native` selection resolves: the host's architecture, system and convention
as the compiler's own manifest names them, every other part mink's default

### linkage_of

```mach
pub fun linkage_of(a: *std.allocator.Allocator, t: *mach.target.resolved.Target, kind: mach.base.diagnostic.kind.Kind) res[*mink.catalog.output.Kind, mach.base.fail.Fail]
```

the linkage of an executable built for `t`, one of mink's executable output kinds: the one
its narrowing names, else the host's default. refused in mink's words, as a user failure of
`kind`, where the host runs none

### is_library

```mach
pub fun is_library(k: Spelled) bool
```

whether `k` is a library, which a consumer links against rather than runs: an archive, or an
image a loader binds into another

### per_module

```mach
pub fun per_module(k: Spelled) bool
```

whether `k` is each module's object, written under the obj directory for a later link, so an
artifact of it delivers no one file at an `out` path

### is_executable

```mach
pub fun is_executable(k: Spelled) bool
```

whether `k` is linked as an executable, of the output kind its linkage names

### kind_name

```mach
pub fun kind_name(k: Spelled) str
```

the name `--kind` and a manifest's `kind` spell `k` by

### is_row

```mach
pub fun is_row(k: Spelled, row: *mink.catalog.output.Kind) bool
```

whether `k` is the one output kind `row`

### output_of

```mach
pub fun output_of(k: Spelled, linkage: *mink.catalog.output.Kind) *mink.catalog.output.Kind
```

the output kind mink makes of `k`: an executable's from its linkage, every other its own row

### executable_output

```mach
pub fun executable_output(a: *std.allocator.Allocator, t: *mach.target.resolved.Target) res[*mink.catalog.output.Kind, mach.base.fail.Fail]
```

the output kind of the executable a build for `t` links, refused where the host runs no
executable

### finished_modules

```mach
pub fun finished_modules(t: *mach.target.resolved.Target) bool
```

whether the target's object format holds each module as its own artifact, as a
SPIR-V module: mink builds a module of the format and no executable of it

### flat_image

```mach
pub fun flat_image(t: *mach.target.resolved.Target) bool
```

whether the target's object format is a flat image, mink's flat format

### states_subsystem

```mach
pub fun states_subsystem(t: *mach.target.resolved.Target) bool
```

whether the target's images state a subsystem, as mink's host of it names a
default one, so an artifact's `subsystem`, icon and manifest mean something

### admits_subsystem

```mach
pub fun admits_subsystem(t: *mach.target.resolved.Target, s: *mink.catalog.subsystem.Subsystem) bool
```

whether the target's images may state the subsystem `s`, as mink's host of it admits

### carries_record

```mach
pub fun carries_record(t: *mach.target.resolved.Target) bool
```

whether a module's object is read again by a link or the cache, so it carries
mach's record section: every object but a flat image's and a finished module's

