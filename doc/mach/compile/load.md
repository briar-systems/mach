[mach](../README.md) / [compile](README.md) / **load**

# mach.compile.load

loading a project's modules: the load walk, reloads after an edit, and the emit order

the walk is the front end's loader (mach.front.load), which calls back here for module
rows, parses and comptime contexts. `target_context` is the comptime context a build's
target and options give every module.

Source: [`src/compile/load.mach`](../../../src/compile/load.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.compile.project`](project.md), [`mach.compile.request`](request.md), [`mach.front.comptime.scope`](../front/comptime/scope.md), [`mach.session.module`](../session/module.md), [`mach.target.facts`](../target/facts.md), [`mach.target.resolved`](../target/resolved.md).

| Functions | |
| :--- | :--- |
| [`entry_module_fqn`](#entry_module_fqn) | the fully qualified name of a target's entry module, under the project id |
| [`join_path`](#join_path) | two paths joined |
| [`target_context`](#target_context) | the comptime context of a build for `t` under `req`, whose back end states `facts`: every fact comptime reads from the target and the build options, and nothing of a project, so the build and the editor fold the same program the same way |
| [`artifact_kind`](#artifact_kind) | the kind of the artifact named `artifact` as the manifest spells it, what `$bin.kind` reads: the cell's own or one the union walk loads, STR_NIL for a build of no artifact |
| [`module_load`](#module_load) | load the module `fqn` names and everything it reaches |
| [`module_load_one`](#module_load_one) | parse the module `fqn` names as a root, without walking it |
| [`module_unread`](#module_unread) | whether the load reached the module `fqn` names and never read its file |
| [`module_reload`](#module_reload) | reparse and re-walk one loaded module after its text changed, and say whether its load surface survived, so the project's other modules stand as they are; every product the project holds of the module is dropped, since each is keyed by the old syntax tree |
| [`reach`](#reach) | mark in `reached`, by module index, every module the build target's own frame reaches from the modules marked there already, the loader bound for the walk |
| [`decorators_record`](#decorators_record) | record the strings a module's decorator arguments evaluate to, from the syntax the project's operations last published |
| [`rebuild_topo`](#rebuild_topo) | reorder the modules of the current topo order for emission: those the emit roots reach first, each after what it imports, then every other module of the order the same way |
| [`untested`](#untested) | every module `mid` reaches marked emitted: through every import in a build of the tests, and otherwise through the imports not marked `#[testing]` the modules a build that runs no tests emits narrowed to those its entries `entries[0 .. count]` reach through imports not marked `#[testing]`, each after what it imports: a module reached only through `#[testing] use` is checked but not emitted |
| [`register_module_asts`](#register_module_asts) | number every module with a file, parse each in one operation and register its identity with the session |
| [`acquire_loaded_parses`](#acquire_loaded_parses) | acquire the parse of every module with a file in the open operation |
| [`collect_loaded_parses`](#collect_loaded_parses) | acquire the parse of every module with a file in an operation of its own and present them |

---

## Functions

### entry_module_fqn

```mach
pub fun entry_module_fqn(p: *mach.compile.project.Project, t: *mach.compile.project.TargetEntry) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

the fully qualified name of a target's entry module, under the project id

| Parameter | |
| :--- | :--- |
| `p` | project whose id prefixes the name |
| `t` | target entry whose entrypoint names the module |
| **returns** | the interned name, or a failure when the entrypoint or id is not interned or the name cannot be composed |

### join_path

```mach
pub fun join_path(alloc: *std.allocator.Allocator, a: str, b: str) res[str, mach.base.fail.Fail]
```

two paths joined

| Parameter | |
| :--- | :--- |
| `alloc` | allocator for the result |
| `a` | the leading path |
| `b` | the path joined onto it |
| **returns** | the joined path, owned by the caller in `alloc`, or the allocation failure |

### target_context

```mach
pub fun target_context(alloc: *std.allocator.Allocator, t: *mach.target.resolved.Target, facts: mach.target.facts.Facts, req: *mach.compile.request.BuildRequest,
    compiler_name: mach.base.intern.StrId, compiler_ver: mach.base.intern.StrId) mach.front.comptime.scope.ComptimeCtx
```

the comptime context of a build for `t` under `req`, whose back end states `facts`: every
fact comptime reads from the target and the build options, and nothing of a project, so
the build and the editor fold the same program the same way

### artifact_kind

```mach
pub fun artifact_kind(p: *mach.compile.project.Project, artifact: mach.base.intern.StrId) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

the kind of the artifact named `artifact` as the manifest spells it, what `$bin.kind` reads:
the cell's own or one the union walk loads, STR_NIL for a build of no artifact

### module_load

```mach
pub fun module_load(p: *mach.compile.project.Project, fqn: mach.base.intern.StrId) res[mach.session.module.ModuleId, mach.base.fail.Fail]
```

load the module `fqn` names and everything it reaches

### module_load_one

```mach
pub fun module_load_one(p: *mach.compile.project.Project, fqn: mach.base.intern.StrId) res[mach.session.module.ModuleId, mach.base.fail.Fail]
```

parse the module `fqn` names as a root, without walking it

### module_unread

```mach
pub fun module_unread(p: *mach.compile.project.Project, fqn: mach.base.intern.StrId) bool
```

whether the load reached the module `fqn` names and never read its file

### module_reload

```mach
pub fun module_reload(p: *mach.compile.project.Project, mid: mach.session.module.ModuleId) res[bool, mach.base.fail.Fail]
```

reparse and re-walk one loaded module after its text changed, and say whether its load surface
survived, so the project's other modules stand as they are; every product the project holds of
the module is dropped, since each is keyed by the old syntax tree

### reach

```mach
pub fun reach(p: *mach.compile.project.Project, reached: *bool) err[mach.base.fail.Fail]
```

mark in `reached`, by module index, every module the build target's own frame reaches from the
modules marked there already, the loader bound for the walk

### decorators_record

```mach
pub fun decorators_record(p: *mach.compile.project.Project, mid: mach.session.module.ModuleId) err[mach.base.fail.Fail]
```

record the strings a module's decorator arguments evaluate to, from the syntax the project's
operations last published

### rebuild_topo

```mach
pub fun rebuild_topo(p: *mach.compile.project.Project, emit_roots: *mach.session.module.ModuleId, emit_root_count: u32) err[mach.base.fail.Fail]
```

reorder the modules of the current topo order for emission: those the emit roots reach first,
each after what it imports, then every other module of the order the same way

| Parameter | |
| :--- | :--- |
| `p` | project whose topo order and emit length are replaced |
| `emit_roots` | the modules the build emits from |
| `emit_root_count` | length of `emit_roots` |
| **returns** | ok, or the failure of an allocation or of a cycle among the loaded modules |

### untested

```mach
pub fun untested(p: *mach.compile.project.Project, entries: *mach.session.module.ModuleId, count: u32) err[mach.base.fail.Fail]
```

every module `mid` reaches marked emitted: through every import in a build of the tests, and
otherwise through the imports not marked `#[testing]`
the modules a build that runs no tests emits narrowed to those its entries `entries[0 .. count]` reach through
imports not marked `#[testing]`, each after what it imports: a module reached only through `#[testing] use` is
checked but not emitted

### register_module_asts

```mach
pub fun register_module_asts(p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

number every module with a file, parse each in one operation and register its identity with the session

| Parameter | |
| :--- | :--- |
| `p` | project whose loaded modules are registered |
| **returns** | ok, or the first failure of numbering, parsing, registering or presenting, after which every identity registered here is unregistered again |

### acquire_loaded_parses

```mach
pub fun acquire_loaded_parses(p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

acquire the parse of every module with a file in the open operation

| Parameter | |
| :--- | :--- |
| `p` | project whose modules are parsed |
| **returns** | ok, or the first parse failure |

### collect_loaded_parses

```mach
pub fun collect_loaded_parses(p: *mach.compile.project.Project, primary: err[mach.base.fail.Fail]) err[mach.base.fail.Fail]
```

acquire the parse of every module with a file in an operation of its own and present them

| Parameter | |
| :--- | :--- |
| `p` | project whose modules are parsed |
| `primary` | the caller's own result, which leads any failure found here |
| **returns** | what finish_query_phase gives back for `primary` or the first parse failure, or `primary` or the failure itself when the roots cannot be collected or the operation cannot be opened |

