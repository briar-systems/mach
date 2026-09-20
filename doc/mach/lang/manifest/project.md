# mach.lang.manifest.project

## fun host_isa_name

```mach
pub fun host_isa_name() str;
```

## fun host_os_name

```mach
pub fun host_os_name() str;
```

## fun host_tuple

```mach
pub fun host_tuple(alloc: *A.Allocator) str;
```

the host as "<isa>-<os>", e.g. "x86_64-linux"

alloc: owns the returned string
ret: the formatted tuple; on allocation failure the literal "out of memory",
       which must not be freed

## fun parse_project

```mach
pub fun parse_project(alloc: *A.Allocator, itn: *intern.Interner, t: *toml.Table, m: *Manifest) err[outcome.Fail];
```

## fun parse_targets

```mach
pub fun parse_targets(alloc: *A.Allocator, itn: *intern.Interner, t: *toml.Table, m: *Manifest, as_root: bool) err[outcome.Fail];
```

## val MISSING_PROFILE_MESSAGE

```mach
pub val MISSING_PROFILE_MESSAGE: str = "mach.toml: no [profile.<name>] table is declared
```

## fun parse_profiles

```mach
pub fun parse_profiles(alloc: *A.Allocator, itn: *intern.Interner, t: *toml.Table, m: *Manifest, as_root: bool) err[outcome.Fail];
```

the `[profile.*]` tables. a root manifest declares at least one; a dependency
manifest that declares none gets the two synthesized ones, since its profiles
are never read to build the consumer

## fun builtin_profile

```mach
pub fun builtin_profile(itn: *intern.Interner, name: str, found: MOpt, debug: bool,
vectorize: bool, is_default: bool) ProfileDef;
```

## fun find_target_by_name

```mach
pub fun find_target_by_name(itn: *intern.Interner, m: *Manifest, name: str) *TargetDef;
```

## fun target_matches_host

```mach
pub fun target_matches_host(itn: *intern.Interner, d: *TargetDef, host_os: u32, host_arch: u32) bool;
```

whether a target's `os` and `isa` name the given host ids. the abi is not compared

itn: resolves the target's strings
d: the target
host_os: an os id from `mach.lang.target.os`
host_arch: an arch id from `mach.lang.target.isa`
ret: true when both ids match; false when either string is unknown to `itn`

## fun make_native_target

```mach
pub fun make_native_target(alloc: *A.Allocator, itn: *intern.Interner) res[*TargetDef, outcome.Fail];
```

