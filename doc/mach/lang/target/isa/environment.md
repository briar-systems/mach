# mach.lang.target.isa.environment

the execution environments an instruction set defines and the profile and
extensions each one grants a target that names it

## rec Environment

```mach
pub rec Environment;
```

an execution environment an isa defines: its name, its profile, and the
extensions of the isa's vocabulary it guarantees (spirv's `zero_init_workgroup`
from vulkan1.3)

## rec Set

```mach
pub rec Set;
```

the environments an isa defines, and what a target naming none of them is
granted of the vocabulary

## val NONE

```mach
pub val NONE: u32 = 0xFFFFFFFF
```

## fun lookup

```mach
pub fun lookup(set: *Set, name: str) u32;
```

## fun profile

```mach
pub fun profile(set: *Set, env_id: u32) u32;
```

## fun extensions

```mach
pub fun extensions(set: *Set, env_id: u32) u64;
```

the extensions the target's environment guarantees, which its selection holds
beside the ones it names

## fun validate

```mach
pub fun validate(a: *A.Allocator, set: *Set, isa_name: str) err[fail.Fail];
```

the set `isa_name` declares, refused when it is malformed

