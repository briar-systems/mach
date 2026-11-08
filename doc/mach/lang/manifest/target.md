# mach.lang.manifest.target

## fun host_arch_name

```mach
pub fun host_arch_name() str;
```

## fun host_sys_name

```mach
pub fun host_sys_name() str;
```

## fun host_tuple

```mach
pub fun host_tuple(alloc: *A.Allocator) res[str, fail.Fail];
```

the host as `mach info` names it, by `tuple_of`

alloc: owns the returned string
ret: the formatted tuple; err when the allocator refused it

## fun tuple_of

```mach
pub fun tuple_of(alloc: *A.Allocator, os_name: str, isa_name: str) res[str, fail.Fail];
```

the one spelling of a (sys, arch) pair, "<sys>-<arch>", e.g. "linux-x86_64"

alloc: owns the returned string
os_name: the system's name
isa_name: the architecture's name
ret: the formatted tuple; err when the allocator refused it

## fun parse_targets

```mach
pub fun parse_targets(alloc: *A.Allocator, itn: *intern.Interner, t: *toml.Table, m: *Manifest) err[fail.Fail];
```

## fun target_set

```mach
pub fun target_set(alloc: *A.Allocator, itn: *intern.Interner, sub: *toml.Table, lead: str, d: *TargetDef) err[fail.Fail];
```

every key `--set` writes over a target that the table `sub` holds, decoded over `d` with where
it was written; a key `sub` does not hold leaves `d` as it is

alloc: owns a refusal's text
itn: receives the strings
sub: the table
lead: the table's label in mach.toml, nil for a `--set` value
d: the target
ret: ok, or the refusal of a value its key does not take, or a refused allocation

## fun find_target_by_name

```mach
pub fun find_target_by_name(itn: *intern.Interner, m: *Manifest, name: str) *TargetDef;
```

the target `m` declares under `name`, nil when none is

## fun resolve_declared

```mach
pub fun resolve_declared(alloc: *A.Allocator, itn: *intern.Interner, d: *TargetDef) res[resolved.Target, fail.Fail];
```

resolve a declared target with every key it states: the one place a
`[target.<name>]` table becomes a target request

alloc: owns the `env` origin a refusal names
itn: resolves the target's strings and holds that origin
d: the target
ret: the target; err the owners' refusal of the request

## fun target_matches_host

```mach
pub fun target_matches_host(itn: *intern.Interner, d: *TargetDef) bool;
```

whether a target's `sys` and `arch` name the host the compiler runs on. the abi is not compared

itn: resolves the target's strings
d: the target
ret: true when both names are the host's

## fun target_is_hosted

```mach
pub fun target_is_hosted(itn: *intern.Interner, d: *TargetDef) bool;
```

whether `native` could ever name the target: its system has a host row whose
environment and architecture the target states, so a build could run on one. a
freestanding or vulkan target is only ever selected by name

## fun make_native_target

```mach
pub fun make_native_target(alloc: *A.Allocator, itn: *intern.Interner) res[*TargetDef, fail.Fail];
```

