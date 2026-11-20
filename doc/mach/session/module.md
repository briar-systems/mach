[mach](../README.md) / [session](README.md) / **module**

# mach.session.module

module ids: a module's index in the session and the stable identity of its name

the registry holds each loaded module's fully qualified name, whether it belongs to the root
project, and the stable identity of its name, which outlives the loads that reuse it. a
standalone editor buffer's module id lies past BUFFER_BASE, so a table keyed by module serves
both.

Source: [`src/session/module.mach`](../../../src/session/module.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md).

| Type | | |
| :--- | :--- | :--- |
| [`ModuleId`](#moduleid) | `def` | a module's index in the session's module table |
| [`StableModuleId`](#stablemoduleid) | `def` | a module's identity across the loads of a session, keyed by its fully qualified name |
| [`Registry`](#registry) | `rec` | the identities of the modules a session loaded: each module's name, whether it belongs to the root project, and the stable identity of each name, which outlives the loads that reuse it |

| On `ModuleId` | |
| :--- | :--- |
| [`MODULE_NIL`](#moduleid) | |
| [`buffer_id`](#buffer_id) | the module of the standalone buffer over the file with index `file`, none past the last id the buffer range holds |
| [`buffer_file`](#buffer_file) | the index of the file whose standalone buffer is the module `m`, none when `m` is a project's module or nil |
| [`id`](#id) | the module id with the given index |
| [`index`](#index) | the index a module id names |
| [`same`](#same) | whether two module ids name the same module |
| [`is_nil`](#is_nil) | whether a module id is MODULE_NIL |

| On `StableModuleId` | |
| :--- | :--- |
| [`STABLE_MODULE_NIL`](#stablemoduleid) | |
| [`stable_id`](#stable_id) | the stable module identity with the given index |
| [`stable_index`](#stable_index) | the number a stable module identity holds |
| [`stable_same`](#stable_same) | whether two stable identities are the same |
| [`stable_is_nil`](#stable_is_nil) | whether a stable identity is STABLE_MODULE_NIL |

| On `Registry` | |
| :--- | :--- |
| [`registry_init`](#registry_init) | an empty registry |
| [`registry_dnit`](#registry_dnit) | release the registry's maps |
| [`reset_module_registry`](#reset_module_registry) | forget every project module identity, keeping the stable identities, which outlive a load |
| [`module_count`](#module_count) | one past the highest project module id the registry holds an identity for |
| [`register_module_warned`](#register_module_warned) | the build shows the warnings of the module `mid`, as its package's `warned` declares |
| [`module_warned`](#module_warned) | whether the build shows the warnings of the module `mid` |
| [`module_fqn`](#module_fqn) | the fully qualified name of a module |
| [`stable_module_id`](#stable_module_id) | the stable identity of a name, issuing the next one when the name has none yet |
| [`stable_module_id_lookup`](#stable_module_id_lookup) | the stable identity already issued for a name, without issuing one |
| [`register_stable_mid`](#register_stable_mid) | record the module a stable identity is loaded as |
| [`module_id_for_stable`](#module_id_for_stable) | the module a stable identity is loaded as |
| [`register_module_identity`](#register_module_identity) | give the module `mid` its name and stable identity; a project's module also counts toward module_count, a standalone buffer's does not |
| [`unregister_module_identity`](#unregister_module_identity) | forget a module's name, warning flag and the load of its stable identity the stable identity itself stays issued, and module_count is left as it was |

| Functions | |
| :--- | :--- |
| [`compose_fqn`](#compose_fqn) | the module name of a source path under a package: the id, then the path without its suffix, its separators dotted |

| Constants | |
| :--- | :--- |
| [`BUFFER_BASE`](#constants) | a project numbers its modules from 0 below BUFFER_BASE, and a standalone editor buffer's module is BUFFER_BASE past its file's index, so no project module takes a buffer's id and a table keyed by module serves both |

---

## ModuleId

```mach
pub def ModuleId: u32
```

a module's index in the session's module table

| Name | Value | |
| :--- | :--- | :--- |
| `MODULE_NIL` | `0xFFFFFFFF` |  |

### buffer_id

```mach
pub fun buffer_id(file: u32) opt[ModuleId]
```

the module of the standalone buffer over the file with index `file`, none
past the last id the buffer range holds

### buffer_file

```mach
pub fun buffer_file(m: ModuleId) opt[u32]
```

the index of the file whose standalone buffer is the module `m`, none when
`m` is a project's module or nil

### id

```mach
pub fun id(index: u32) ModuleId
```

the module id with the given index

| Parameter | |
| :--- | :--- |
| `index` | the module's index in the session's module table |
| **returns** | the module id |

### index

```mach
pub fun index(m: ModuleId) u32
```

the index a module id names

| Parameter | |
| :--- | :--- |
| `m` | the module id |
| **returns** | its index in the session's module table |

### same

```mach
pub fun same(left: ModuleId, right: ModuleId) bool
```

whether two module ids name the same module

### is_nil

```mach
pub fun is_nil(m: ModuleId) bool
```

whether a module id is MODULE_NIL

## StableModuleId

```mach
pub def StableModuleId: u32
```

a module's identity across the loads of a session, keyed by its fully qualified name

| Name | Value | |
| :--- | :--- | :--- |
| `STABLE_MODULE_NIL` | `0xFFFFFFFF` |  |

### stable_id

```mach
pub fun stable_id(index: u32) StableModuleId
```

the stable module identity with the given index

| Parameter | |
| :--- | :--- |
| `index` | the identity's number |
| **returns** | the stable identity |

### stable_index

```mach
pub fun stable_index(m: StableModuleId) u32
```

the number a stable module identity holds

### stable_same

```mach
pub fun stable_same(left: StableModuleId, right: StableModuleId) bool
```

whether two stable identities are the same

### stable_is_nil

```mach
pub fun stable_is_nil(m: StableModuleId) bool
```

whether a stable identity is STABLE_MODULE_NIL

## Registry

```mach
pub rec Registry {
    count:         u32;
    fqns:          std.collections.map.Map[ModuleId, mach.base.intern.StrId];
    warned:        std.collections.map.Map[ModuleId, bool];
    stable_ids:    std.collections.map.Map[mach.base.intern.StrId, StableModuleId];
    stable_next:   StableModuleId;
    stable_to_mid: std.collections.map.Map[StableModuleId, ModuleId];
}
```

the identities of the modules a session loaded: each module's name, whether it
belongs to the root project, and the stable identity of each name, which
outlives the loads that reuse it

| Field | Type | |
| :--- | :--- | :--- |
| `count` | `u32` | one past the highest project module id held; below BUFFER_BASE, so it never wraps |
| `fqns` | `std.collections.map.Map[ModuleId, mach.base.intern.StrId]` | each module's fully qualified name |
| `warned` | `std.collections.map.Map[ModuleId, bool]` | the modules whose warnings the build shows, declared by the loader as it gives the module its id, from its package (`mach.compile.request.shows_warnings`). the store and every pass whose only product is a warning read it here |
| `stable_ids` | `std.collections.map.Map[mach.base.intern.StrId, StableModuleId]` | the stable identity of each name |
| `stable_next` | `StableModuleId` | the next stable identity to issue |
| `stable_to_mid` | `std.collections.map.Map[StableModuleId, ModuleId]` | the module each stable identity is loaded as |

### registry_init

```mach
pub fun registry_init(a: *std.allocator.Allocator) Registry
```

an empty registry

| Parameter | |
| :--- | :--- |
| `a` | the allocator its maps draw from |
| **returns** | the registry, released with `registry_dnit` |

### registry_dnit

```mach
pub fun registry_dnit(r: *Registry)
```

release the registry's maps

### reset_module_registry

```mach
pub fun reset_module_registry(r: *Registry)
```

forget every project module identity, keeping the stable identities, which outlive a load

### module_count

```mach
pub fun module_count(r: *Registry) u32
```

one past the highest project module id the registry holds an identity for

### register_module_warned

```mach
pub fun register_module_warned(r: *Registry, mid: ModuleId, warned: bool) err[mach.base.fail.Fail]
```

the build shows the warnings of the module `mid`, as its package's `warned` declares

### module_warned

```mach
pub fun module_warned(r: *Registry, mid: ModuleId) bool
```

whether the build shows the warnings of the module `mid`. a module the loader never declared,
as a standalone editor buffer's, belongs to no project, and its warning-only passes do not run

### module_fqn

```mach
pub fun module_fqn(r: *Registry, mid: ModuleId) mach.base.intern.StrId
```

the fully qualified name of a module

| Parameter | |
| :--- | :--- |
| `r` | the registry |
| `mid` | the module |
| **returns** | its name, or STR_NIL when `mid` is nil or has no identity registered |

### stable_module_id

```mach
pub fun stable_module_id(r: *Registry, fqn: mach.base.intern.StrId) res[StableModuleId, mach.base.fail.Fail]
```

the stable identity of a name, issuing the next one when the name has none yet

| Parameter | |
| :--- | :--- |
| `r` | the registry |
| `fqn` | the module's fully qualified name |
| **returns** | the identity, or a failure when identities are exhausted or the map refuses the insert |

### stable_module_id_lookup

```mach
pub fun stable_module_id_lookup(r: *Registry, fqn: mach.base.intern.StrId) StableModuleId
```

the stable identity already issued for a name, without issuing one

| Parameter | |
| :--- | :--- |
| `r` | the registry |
| `fqn` | the module's fully qualified name |
| **returns** | the identity, or STABLE_MODULE_NIL when the name has none |

### register_stable_mid

```mach
pub fun register_stable_mid(r: *Registry, stable: StableModuleId, mid: ModuleId) err[mach.base.fail.Fail]
```

record the module a stable identity is loaded as

| Parameter | |
| :--- | :--- |
| `r` | the registry |
| `stable` | the stable identity |
| `mid` | the module it is loaded as, replacing any earlier one |
| **returns** | ok, or a failure when the map refuses the insert |

### module_id_for_stable

```mach
pub fun module_id_for_stable(r: *Registry, stable: StableModuleId) ModuleId
```

the module a stable identity is loaded as

| Parameter | |
| :--- | :--- |
| `r` | the registry |
| `stable` | the stable identity |
| **returns** | the module, or MODULE_NIL when `stable` is nil or not loaded |

### register_module_identity

```mach
pub fun register_module_identity(r: *Registry, mid: ModuleId, fqn: mach.base.intern.StrId, stable: StableModuleId) err[mach.base.fail.Fail]
```

give the module `mid` its name and stable identity; a project's module also
counts toward module_count, a standalone buffer's does not

### unregister_module_identity

```mach
pub fun unregister_module_identity(r: *Registry, mid: ModuleId, stable: StableModuleId)
```

forget a module's name, warning flag and the load of its stable identity
the stable identity itself stays issued, and module_count is left as it was

| Parameter | |
| :--- | :--- |
| `r` | the registry |
| `mid` | the module to forget |
| `stable` | the stable identity it was loaded as |

## Functions

### compose_fqn

```mach
pub fun compose_fqn(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, id_text: str, rel_text: str) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

the module name of a source path under a package: the id, then the path without its suffix, its separators dotted

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `BUFFER_BASE` | `u32` | `0x80000000` | a project numbers its modules from 0 below BUFFER_BASE, and a standalone editor buffer's module is BUFFER_BASE past its file's index, so no project module takes a buffer's id and a table keyed by module serves both |

