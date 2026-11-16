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
| [`StableModuleId`](#stablemoduleid) | `def` | a module's identity across sessions, keyed by its fully qualified name |
| [`Registry`](#registry) | `rec` | the identities of the modules a session loaded: each module's name, whether it belongs to the root project, and the stable identity of each name, which outlives the loads that reuse it |

| On `ModuleId` | |
| :--- | :--- |
| [`MODULE_NIL`](#moduleid) | |
| [`buffer_id`](#buffer_id) | the module of the standalone buffer over the file with index `file`, none past the last id the buffer range holds |
| [`buffer_file`](#buffer_file) | the index of the file whose standalone buffer is the module `m`, none when `m` is a project's module or nil |
| [`id`](#id), [`index`](#index), [`same`](#same), [`is_nil`](#is_nil) | |

| On `StableModuleId` | |
| :--- | :--- |
| [`STABLE_MODULE_NIL`](#stablemoduleid), [`stable_id`](#stable_id), [`stable_index`](#stable_index), [`stable_same`](#stable_same), [`stable_is_nil`](#stable_is_nil) | |

| On `Registry` | |
| :--- | :--- |
| [`registry_init`](#registry_init), [`registry_dnit`](#registry_dnit) | |
| [`reset_module_registry`](#reset_module_registry) | forget every project module identity, keeping the stable identities, which outlive a load |
| [`module_count`](#module_count) | one past the highest project module id the registry holds an identity for |
| [`register_module_root_project`](#register_module_root_project) | |
| [`module_is_root_project`](#module_is_root_project) | a module the loader never declared is not the root project's: a dependency's module, or one loaded outside a project, exports nothing of its own |
| [`module_fqn`](#module_fqn), [`stable_module_id`](#stable_module_id), [`stable_module_id_lookup`](#stable_module_id_lookup), [`register_stable_mid`](#register_stable_mid), [`module_id_for_stable`](#module_id_for_stable) | |
| [`register_module_identity`](#register_module_identity) | give the module `mid` its name and stable identity; a project's module also counts toward module_count, a standalone buffer's does not |
| [`unregister_module_identity`](#unregister_module_identity) | |

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

### index

```mach
pub fun index(m: ModuleId) u32
```

### same

```mach
pub fun same(left: ModuleId, right: ModuleId) bool
```

### is_nil

```mach
pub fun is_nil(m: ModuleId) bool
```

## StableModuleId

```mach
pub def StableModuleId: u32
```

a module's identity across sessions, keyed by its fully qualified name

| Name | Value | |
| :--- | :--- | :--- |
| `STABLE_MODULE_NIL` | `0xFFFFFFFF` |  |

### stable_id

```mach
pub fun stable_id(index: u32) StableModuleId
```

### stable_index

```mach
pub fun stable_index(m: StableModuleId) u32
```

### stable_same

```mach
pub fun stable_same(left: StableModuleId, right: StableModuleId) bool
```

### stable_is_nil

```mach
pub fun stable_is_nil(m: StableModuleId) bool
```

## Registry

```mach
pub rec Registry {
    count:         u32;
    fqns:          std.collections.map.Map[ModuleId, mach.base.intern.StrId];
    root_project:  std.collections.map.Map[ModuleId, bool];
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
| `root_project` | `std.collections.map.Map[ModuleId, bool]` | which modules belong to the root project rather than a dependency, declared by the loader as it gives the module its id; a shared library's export surface is drawn from these alone |
| `stable_ids` | `std.collections.map.Map[mach.base.intern.StrId, StableModuleId]` | the stable identity of each name |
| `stable_next` | `StableModuleId` | the next stable identity to issue |
| `stable_to_mid` | `std.collections.map.Map[StableModuleId, ModuleId]` | the module each stable identity is loaded as |

### registry_init

```mach
pub fun registry_init(a: *std.allocator.Allocator) Registry
```

### registry_dnit

```mach
pub fun registry_dnit(r: *Registry)
```

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

### register_module_root_project

```mach
pub fun register_module_root_project(r: *Registry, mid: ModuleId, root: bool) err[mach.base.fail.Fail]
```

### module_is_root_project

```mach
pub fun module_is_root_project(r: *Registry, mid: ModuleId) bool
```

a module the loader never declared is not the root project's: a dependency's
module, or one loaded outside a project, exports nothing of its own

### module_fqn

```mach
pub fun module_fqn(r: *Registry, mid: ModuleId) mach.base.intern.StrId
```

### stable_module_id

```mach
pub fun stable_module_id(r: *Registry, fqn: mach.base.intern.StrId) res[StableModuleId, mach.base.fail.Fail]
```

### stable_module_id_lookup

```mach
pub fun stable_module_id_lookup(r: *Registry, fqn: mach.base.intern.StrId) StableModuleId
```

### register_stable_mid

```mach
pub fun register_stable_mid(r: *Registry, stable: StableModuleId, mid: ModuleId) err[mach.base.fail.Fail]
```

### module_id_for_stable

```mach
pub fun module_id_for_stable(r: *Registry, stable: StableModuleId) ModuleId
```

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

