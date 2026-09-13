# mach.lang.target.registry

## rec TargetRegistry

```mach
pub rec TargetRegistry;
```

## val REGISTRY_FRESH

```mach
pub val REGISTRY_FRESH:    u8  = 0
```

## val REGISTRY_BUILDING

```mach
pub val REGISTRY_BUILDING: u8  = 1
```

## val REGISTRY_READY

```mach
pub val REGISTRY_READY:    u8  = 2
```

## val REGISTRY_RELEASED

```mach
pub val REGISTRY_RELEASED: u8  = 3
```

## val REGISTRY_VERSION

```mach
pub val REGISTRY_VERSION:  u32 = 1
```

## fun registry_init

```mach
pub fun registry_init() TargetRegistry;
```

an in-place registry over the sub-registries' own page allocators

## fun registry_new

```mach
pub fun registry_new(alloc: *A.Allocator) res[*TargetRegistry, fail.Fail];
```

a heap registry whose block and every entry come from `alloc`

## fun registry_rollback

```mach
pub fun registry_rollback(reg: *TargetRegistry);
```

a publication that failed part way releases what it registered and hands
back a fresh registry; nothing borrowed from it yet, so a retry is sound

## fun registry_dnit

```mach
pub fun registry_dnit(reg: *TargetRegistry);
```

release the entries and, for a heap registry, return the block to its
owner; release is terminal, a second call on an in-place registry is a
no-op, and a second call on a heap registry is a call on a dead pointer

## fun registry_published

```mach
pub fun registry_published(reg: *TargetRegistry) bool;
```

## fun registry_empty

```mach
pub fun registry_empty(reg: *TargetRegistry) bool;
```

