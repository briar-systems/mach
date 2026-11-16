[mach](../README.md) / [compile](README.md) / **load**

# mach.compile.load

loading a project's modules: the load walk, reloads after an edit, and the emit order

the walk is the front end's loader (mach.front.load), which calls back here for module
rows, parses and comptime contexts. `target_context` is the comptime context a build's
target and options give every module.

<sub>source: [`src/compile/load.mach`](../../../src/compile/load.mach) · imported by 12 modules</sub>

<sub>`project` is [`mach.compile.project`](project.md) · `intern` is [`mach.base.intern`](../base/intern.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `A` is `std.allocator` · `resolved` is [`mach.target.resolved`](../target/resolved.md) · `target_facts` is [`mach.target.facts`](../target/facts.md) · `request` is [`mach.compile.request`](request.md) · `comptime_scope` is [`mach.front.comptime.scope`](../front/comptime/scope.md) · `module` is [`mach.session.module`](../session/module.md)</sub>

| Functions | |
| :--- | :--- |
| [`entry_module_fqn`](#entry_module_fqn) · [`join_path`](#join_path) | |
| [`target_context`](#target_context) | the comptime context of a build for `t` under `req`, whose back end states `facts`: every fact comptime reads from the target and the build options, and nothing of a project, so the build and the editor fold the same program the same way |
| [`module_load`](#module_load) | load the module `fqn` names and everything it reaches |
| [`module_load_one`](#module_load_one) | parse the module `fqn` names as a root, without walking it |
| [`module_unread`](#module_unread) | whether the load reached the module `fqn` names and never read its file |
| [`module_reload`](#module_reload) | reparse and re-walk one loaded module after its text changed, and say whether its load surface survived, so the project's other modules stand as they are; every product the project holds of the module is dropped, since each is keyed by the old syntax tree |
| [`decorators_record`](#decorators_record) | record the strings a module's decorator arguments evaluate to, from the syntax the project's operations last published |
| [`rebuild_topo`](#rebuild_topo) · [`register_module_asts`](#register_module_asts) · [`acquire_loaded_parses`](#acquire_loaded_parses) · [`collect_loaded_parses`](#collect_loaded_parses) | |

---

## Functions

### entry_module_fqn

```mach
pub fun entry_module_fqn(p: *project.Project, t: *project.TargetEntry) res[intern.StrId, fail.Fail]
```

### join_path

```mach
pub fun join_path(alloc: *A.Allocator, a: str, b: str) res[str, fail.Fail]
```

### target_context

```mach
pub fun target_context(alloc: *A.Allocator, t: *resolved.Target, facts: target_facts.Facts, req: *request.BuildRequest,
    compiler_name: intern.StrId, compiler_ver: intern.StrId) comptime_scope.ComptimeCtx
```

the comptime context of a build for `t` under `req`, whose back end states `facts`: every
fact comptime reads from the target and the build options, and nothing of a project, so
the build and the editor fold the same program the same way

### module_load

```mach
pub fun module_load(p: *project.Project, fqn: intern.StrId) res[module.ModuleId, fail.Fail]
```

load the module `fqn` names and everything it reaches

### module_load_one

```mach
pub fun module_load_one(p: *project.Project, fqn: intern.StrId) res[module.ModuleId, fail.Fail]
```

parse the module `fqn` names as a root, without walking it

### module_unread

```mach
pub fun module_unread(p: *project.Project, fqn: intern.StrId) bool
```

whether the load reached the module `fqn` names and never read its file

### module_reload

```mach
pub fun module_reload(p: *project.Project, mid: module.ModuleId) res[bool, fail.Fail]
```

reparse and re-walk one loaded module after its text changed, and say whether its load surface
survived, so the project's other modules stand as they are; every product the project holds of
the module is dropped, since each is keyed by the old syntax tree

### decorators_record

```mach
pub fun decorators_record(p: *project.Project, mid: module.ModuleId) err[fail.Fail]
```

record the strings a module's decorator arguments evaluate to, from the syntax the project's
operations last published

### rebuild_topo

```mach
pub fun rebuild_topo(p: *project.Project, emit_roots: *module.ModuleId, emit_root_count: u32) err[fail.Fail]
```

### register_module_asts

```mach
pub fun register_module_asts(p: *project.Project) err[fail.Fail]
```

### acquire_loaded_parses

```mach
pub fun acquire_loaded_parses(p: *project.Project) err[fail.Fail]
```

### collect_loaded_parses

```mach
pub fun collect_loaded_parses(p: *project.Project, primary: err[fail.Fail]) err[fail.Fail]
```

