[mach](../README.md) / [manifest](README.md) / **target**

# mach.manifest.target

the `[target.<name>]` tables: their decoding, `--set`, migration and resolution

`resolve_declared` is the one place a declared target becomes mink's spelling and
narrowing. the host's target, made for a manifest that declares none, and the
`<sys>-<arch>` spelling of a target are made here too.

<sub>source: [`src/manifest/target.mach`](../../../src/manifest/target.mach) · imported by 9 modules</sub>

<sub>`A` is `std.allocator` · `fail` is [`mach.base.fail`](../base/fail.md) · `intern` is [`mach.base.intern`](../base/intern.md) · `toml_value` is `toml.value` · `resolved` is [`mach.target.resolved`](../target/resolved.md)</sub>

| Functions | |
| :--- | :--- |
| [`host_tuple`](#host_tuple) | the host as `mach info` names it, by `tuple_of` |
| [`tuple_of`](#tuple_of) | the one spelling of a (sys, arch) pair, "&lt;sys>-&lt;arch>", e.g. "linux-x86_64" |
| [`parse_targets`](#parse_targets) | |
| [`rewrites`](#rewrites) | every rewrite of a target spelling the previous major wrote, as fixes of the manifest's text, which `mach migrate` applies: each retired spelling and each `arch` written as an instruction set, as the refusals above fix them, and the previous major's `isa` written as an instruction set where the table writes no `arch`, which names the architecture that enters it beside it |
| [`target_set`](#target_set) | every key `--set` writes over a target that the table `sub` holds, decoded over `d` with where it was written; a key `sub` does not hold leaves `d` as it is |
| [`find_target_by_name`](#find_target_by_name) | the target `m` declares under `name`, nil when none is |
| [`resolve_declared`](#resolve_declared) | resolve a declared target with every key it states: the one place a `[target.<name>]` table becomes mink's spelling and narrowing |
| [`target_matches_host`](#target_matches_host) | whether a target's `sys` and `arch` name the host the compiler runs on |
| [`target_is_hosted`](#target_is_hosted) | whether `native` could ever name the target: its system has a host row whose environment and architecture the target states, so a build could run on one |
| [`native_target_init`](#native_target_init) | |

---

## Functions

### host_tuple

```mach
pub fun host_tuple(alloc: *A.Allocator) res[str, fail.Fail]
```

the host as `mach info` names it, by `tuple_of`

| Parameter | |
| :--- | :--- |
| `alloc` | owns the returned string |
| **returns** | the formatted tuple; err when the allocator refused it |

### tuple_of

```mach
pub fun tuple_of(alloc: *A.Allocator, os_name: str, isa_name: str) res[str, fail.Fail]
```

the one spelling of a (sys, arch) pair, "&lt;sys>-&lt;arch>", e.g. "linux-x86_64"

| Parameter | |
| :--- | :--- |
| `alloc` | owns the returned string |
| `os_name` | the system's name |
| `isa_name` | the architecture's name |
| **returns** | the formatted tuple; err when the allocator refused it |

### parse_targets

```mach
pub fun parse_targets(alloc: *A.Allocator, itn: *intern.Interner, t: *toml_value.Table, m: *Manifest) err[fail.Fail]
```

### rewrites

```mach
pub fun rewrites(alloc: *A.Allocator, doc: *toml_value.Table, out: *Vector[Fix]) err[fail.Fail]
```

every rewrite of a target spelling the previous major wrote, as fixes of the manifest's text,
which `mach migrate` applies: each retired spelling and each `arch` written as an instruction
set, as the refusals above fix them, and the previous major's `isa` written as an instruction
set where the table writes no `arch`, which names the architecture that enters it beside it

| Parameter | |
| :--- | :--- |
| `alloc` | owns the fixes' text and storage |
| `doc` | the document root, a manifest of either major |
| `out` | receives the fixes, in no order |
| **returns** | ok, or the refusal of an allocation |

### target_set

```mach
pub fun target_set(alloc: *A.Allocator, itn: *intern.Interner, sub: *toml_value.Table, lead: str, d: *TargetDef) err[fail.Fail]
```

every key `--set` writes over a target that the table `sub` holds, decoded over `d` with where
it was written; a key `sub` does not hold leaves `d` as it is

| Parameter | |
| :--- | :--- |
| `alloc` | owns a refusal's text |
| `itn` | receives the strings |
| `sub` | the table |
| `lead` | the table's label in mach.toml, nil for a `--set` value |
| `d` | the target |
| **returns** | ok, or the refusal of a value its key does not take, or a refused allocation |

### find_target_by_name

```mach
pub fun find_target_by_name(itn: *intern.Interner, m: *Manifest, name: str) *TargetDef
```

the target `m` declares under `name`, nil when none is

### resolve_declared

```mach
pub fun resolve_declared(alloc: *A.Allocator, itn: *intern.Interner, d: *TargetDef, linkage: intern.StrId, linkage_from: *Origin) res[resolved.Target, fail.Fail]
```

resolve a declared target with every key it states: the one place a `[target.<name>]`
table becomes mink's spelling and narrowing

| Parameter | |
| :--- | :--- |
| `alloc` | owns a refusal's text |
| `itn` | resolves the target's strings |
| `d` | the target |
| `linkage` | the name of the linkage of the executable under build, nil for the host's default, which mink resolves against the host |
| `linkage_from` | where `linkage` was written, which a refusal of it names and points at; nil where it was written nowhere |
| **returns** | the target; err the owners' refusal, naming the target |

### target_matches_host

```mach
pub fun target_matches_host(itn: *intern.Interner, d: *TargetDef) bool
```

whether a target's `sys` and `arch` name the host the compiler runs on. the abi is not compared

| Parameter | |
| :--- | :--- |
| `itn` | resolves the target's strings |
| `d` | the target |
| **returns** | true when both names are the host's |

### target_is_hosted

```mach
pub fun target_is_hosted(itn: *intern.Interner, d: *TargetDef) bool
```

whether `native` could ever name the target: its system has a host row whose
environment and architecture the target states, so a build could run on one. a
freestanding or vulkan target is only ever selected by name

### native_target_init

```mach
pub fun native_target_init(alloc: *A.Allocator, itn: *intern.Interner) res[*TargetDef, fail.Fail]
```

