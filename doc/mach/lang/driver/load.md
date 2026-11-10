# mach.lang.driver.load

## fun entry_module_fqn

```mach
pub fun entry_module_fqn(p: *project.Project, t: *project.TargetEntry) res[intern.StrId, fail.Fail];
```

## fun join_path

```mach
pub fun join_path(alloc: *A.Allocator, a: str, b: str) res[str, fail.Fail];
```

## fun target_context

```mach
pub fun target_context(alloc: *A.Allocator, t: *resolved.Target, facts: target_facts.Facts, req: *request.BuildRequest,
compiler_name: intern.StrId, compiler_ver: intern.StrId) comptime_scope.ComptimeCtx;
```

the comptime context of a build for `t` under `req`, whose back end states `facts`: every
fact comptime reads from the target and the build options, and nothing of a project, so
the build and the editor fold the same program the same way

## fun module_load

```mach
pub fun module_load(p: *project.Project, fqn: intern.StrId) res[module.ModuleId, fail.Fail];
```

load the module `fqn` names and everything it reaches

## fun module_open

```mach
pub fun module_open(p: *project.Project, fqn: intern.StrId) res[module.ModuleId, fail.Fail];
```

parse the module `fqn` names as a root, without walking it

## fun module_unread

```mach
pub fun module_unread(p: *project.Project, fqn: intern.StrId) bool;
```

whether the load reached the module `fqn` names and never read its file

## fun module_reload

```mach
pub fun module_reload(p: *project.Project, mid: module.ModuleId) res[bool, fail.Fail];
```

reparse and re-walk one loaded module after its text changed, and say whether its load surface
survived, so the project's other modules stand as they are; every product the project holds of
the module is dropped, since each is keyed by the old syntax tree

## fun decorators_record

```mach
pub fun decorators_record(p: *project.Project, mid: module.ModuleId) err[fail.Fail];
```

record the strings a module's decorator arguments evaluate to

## fun rebuild_topo

```mach
pub fun rebuild_topo(p: *project.Project, emit_roots: *module.ModuleId, emit_root_count: u32) err[fail.Fail];
```

## fun register_module_asts

```mach
pub fun register_module_asts(p: *project.Project) err[fail.Fail];
```

## fun acquire_loaded_parses

```mach
pub fun acquire_loaded_parses(p: *project.Project) err[fail.Fail];
```

## fun collect_loaded_parses

```mach
pub fun collect_loaded_parses(p: *project.Project, primary: err[fail.Fail]) err[fail.Fail];
```

