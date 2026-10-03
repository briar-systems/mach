# mach.lang.target.registry

the target registry: one container holding every axis of the catalog by
pointer. a member is static data its own module declares, so adding one
checks it once and keeps the pointer: the registry copies nothing, owns
nothing and is never released. a registry is published once the catalog it
holds is checked whole, and publication is the proof every later read
relies on

## val AXIS_CAP

```mach
pub val AXIS_CAP: u32 = 16
```

## rec Axis

```mach
pub rec Axis[T];
```

the members of one axis, read by name or position; the accessors and the
validator are the axis's own, so the container never reads a member

## fun axis

```mach
pub fun axis[T](noun: str, name_of: fun(*T) str, id_of: fun(*T) u32, validate: fun(*A.Allocator, *T) err[fail.Fail]) Axis[T];
```

## fun add

```mach
pub fun add[T](ax: *Axis[T], a: *A.Allocator, m: *T) err[fail.Fail];
```

add a member: refused when it is malformed, when its name or its catalog id
is already held, or when the axis is full. an id of 0 is outside the catalog
and may repeat

a: formats the refusal
m: the member, static data that outlives the registry

## fun find

```mach
pub fun find[T](ax: *Axis[T], name: str) opt[*T];
```

## fun at

```mach
pub fun at[T](ax: *Axis[T], idx: u32) opt[*T];
```

## rec TargetRegistry

```mach
pub rec TargetRegistry;
```

## fun registry_init_with_allocator

```mach
pub fun registry_init_with_allocator(alloc: *A.Allocator) TargetRegistry;
```

an empty registry whose refusals are formatted from `alloc`

## fun registry_published

```mach
pub fun registry_published(reg: *TargetRegistry) bool;
```

## fun registry_empty

```mach
pub fun registry_empty(reg: *TargetRegistry) bool;
```

