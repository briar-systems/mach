# mach.lang.module

## rec ModuleId

```mach
pub rec ModuleId;
```

a module's index in the session's module table

## val MODULE_NIL

```mach
pub val MODULE_NIL: ModuleId = ModuleId;
```

## rec StableModuleId

```mach
pub rec StableModuleId;
```

a module's identity across sessions, keyed by its fully qualified name

## val STABLE_MODULE_NIL

```mach
pub val STABLE_MODULE_NIL: StableModuleId = StableModuleId;
```

## val BUFFER_BASE

```mach
pub val BUFFER_BASE: u32 = 0x80000000
```

a project numbers its modules from 0 below BUFFER_BASE, and a standalone
editor buffer's module is BUFFER_BASE past its file's index, so no project
module takes a buffer's id and a table keyed by module serves both

## fun buffer_id

```mach
pub fun buffer_id(file: u32) opt[ModuleId];
```

the module of the standalone buffer over the file with index `file`, none
past the last id the buffer range holds

## fun buffer_file

```mach
pub fun buffer_file(m: ModuleId) opt[u32];
```

the index of the file whose standalone buffer is the module `m`, none when
`m` is a project's module or nil

## fun id

```mach
pub fun id(index: u32) ModuleId;
```

## fun index

```mach
pub fun index(m: ModuleId) u32;
```

## fun same

```mach
pub fun same(left: ModuleId, right: ModuleId) bool;
```

## fun is_nil

```mach
pub fun is_nil(m: ModuleId) bool;
```

## fun stable_id

```mach
pub fun stable_id(index: u32) StableModuleId;
```

## fun stable_index

```mach
pub fun stable_index(m: StableModuleId) u32;
```

## fun stable_same

```mach
pub fun stable_same(left: StableModuleId, right: StableModuleId) bool;
```

## fun stable_is_nil

```mach
pub fun stable_is_nil(m: StableModuleId) bool;
```

## fun compose_fqn

```mach
pub fun compose_fqn(alloc: *A.Allocator, itn: *intern.Interner, id_text: str, rel_text: str) res[intern.StrId, fail.Fail];
```

the module name of a source path under a package: the id, then the path without its suffix, its separators dotted

## rec Registry

```mach
pub rec Registry;
```

the identities of the modules a session loaded: each module's name, whether it
belongs to the root project, and the stable identity of each name, which
outlives the loads that reuse it

count: one past the highest project module id held; below BUFFER_BASE, so it
               never wraps
fqns: each module's fully qualified name
root_project: which modules belong to the root project rather than a dependency,
               declared by the loader as it gives the module its id; a shared
               library's export surface is drawn from these alone
stable_ids: the stable identity of each name
stable_next: the next stable identity to issue
stable_to_mid: the module each stable identity is loaded as

## fun registry_init

```mach
pub fun registry_init(a: *A.Allocator) Registry;
```

## fun registry_dnit

```mach
pub fun registry_dnit(r: *Registry);
```

## fun reset_module_registry

```mach
pub fun reset_module_registry(r: *Registry);
```

forget every project module identity, keeping the stable identities, which outlive a load

## fun module_count

```mach
pub fun module_count(r: *Registry) u32;
```

one past the highest project module id the registry holds an identity for

## fun register_module_root_project

```mach
pub fun register_module_root_project(r: *Registry, mid: ModuleId, root: bool) err[fail.Fail];
```

## fun module_is_root_project

```mach
pub fun module_is_root_project(r: *Registry, mid: ModuleId) bool;
```

a module the loader never declared is not the root project's: a dependency's
module, or one loaded outside a project, exports nothing of its own

## fun module_fqn

```mach
pub fun module_fqn(r: *Registry, mid: ModuleId) intern.StrId;
```

## fun stable_module_id

```mach
pub fun stable_module_id(r: *Registry, fqn: intern.StrId) res[StableModuleId, fail.Fail];
```

## fun stable_module_id_lookup

```mach
pub fun stable_module_id_lookup(r: *Registry, fqn: intern.StrId) StableModuleId;
```

## fun register_stable_mid

```mach
pub fun register_stable_mid(r: *Registry, stable: StableModuleId, mid: ModuleId) err[fail.Fail];
```

## fun module_id_for_stable

```mach
pub fun module_id_for_stable(r: *Registry, stable: StableModuleId) ModuleId;
```

## fun register_module_identity

```mach
pub fun register_module_identity(r: *Registry, mid: ModuleId, fqn: intern.StrId, stable: StableModuleId) err[fail.Fail];
```

give the module `mid` its name and stable identity; a project's module also
counts toward module_count, a standalone buffer's does not

## fun unregister_module_identity

```mach
pub fun unregister_module_identity(r: *Registry, mid: ModuleId, stable: StableModuleId);
```

