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
| [`entry_module_fqn`](#entry_module_fqn), [`join_path`](#join_path) | |
| [`target_context`](#target_context) | the comptime context of a build for `t` under `req`, whose back end states `facts`: every fact comptime reads from the target and the build options, and nothing of a project, so the build and the editor fold the same program the same way |
| [`module_load`](#module_load) | load the module `fqn` names and everything it reaches |
| [`module_load_one`](#module_load_one) | parse the module `fqn` names as a root, without walking it |
| [`module_unread`](#module_unread) | whether the load reached the module `fqn` names and never read its file |
| [`module_reload`](#module_reload) | reparse and re-walk one loaded module after its text changed, and say whether its load surface survived, so the project's other modules stand as they are; every product the project holds of the module is dropped, since each is keyed by the old syntax tree |
| [`decorators_record`](#decorators_record) | record the strings a module's decorator arguments evaluate to, from the syntax the project's operations last published |
| [`rebuild_topo`](#rebuild_topo), [`register_module_asts`](#register_module_asts), [`acquire_loaded_parses`](#acquire_loaded_parses), [`collect_loaded_parses`](#collect_loaded_parses) | |

---

## Functions

### entry_module_fqn

```mach
pub fun entry_module_fqn(p: *mach.compile.project.Project, t: *mach.compile.project.TargetEntry) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

### join_path

```mach
pub fun join_path(alloc: *std.allocator.Allocator, a: str, b: str) res[str, mach.base.fail.Fail]
```

### target_context

```mach
pub fun target_context(alloc: *std.allocator.Allocator, t: *mach.target.resolved.Target, facts: mach.target.facts.Facts, req: *mach.compile.request.BuildRequest,
    compiler_name: mach.base.intern.StrId, compiler_ver: mach.base.intern.StrId) mach.front.comptime.scope.ComptimeCtx
```

the comptime context of a build for `t` under `req`, whose back end states `facts`: every
fact comptime reads from the target and the build options, and nothing of a project, so
the build and the editor fold the same program the same way

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

### register_module_asts

```mach
pub fun register_module_asts(p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

### acquire_loaded_parses

```mach
pub fun acquire_loaded_parses(p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

### collect_loaded_parses

```mach
pub fun collect_loaded_parses(p: *mach.compile.project.Project, primary: err[mach.base.fail.Fail]) err[mach.base.fail.Fail]
```

