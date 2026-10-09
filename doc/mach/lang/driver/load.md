# mach.lang.driver.load

## val MACH_VERSION

```mach
pub val MACH_VERSION: str = lang_version.MACH_VERSION
```

## fun entry_module_fqn

```mach
pub fun entry_module_fqn(p: *project.Project, t: *project.TargetEntry) res[intern.StrId, fail.Fail];
```

## fun compose_module_fqn

```mach
pub fun compose_module_fqn(alloc: *A.Allocator, itn: *intern.Interner, id_text: str, rel_text: str) res[intern.StrId, fail.Fail];
```

## fun diag_join_named

```mach
pub fun diag_join_named(s: *session.Session, prefix: str, name_id: intern.StrId, suffix: str) res[str, fail.Fail];
```

`prefix`, the text of `name_id`, then `suffix`, interned

## fun join_path

```mach
pub fun join_path(alloc: *A.Allocator, a: str, b: str) res[str, fail.Fail];
```

## fun target_context

```mach
pub fun target_context(alloc: *A.Allocator, t: *lang_target.Binding, req: *request.BuildRequest,
compiler_name: intern.StrId, compiler_ver: intern.StrId) comptime.ComptimeCtx;
```

the comptime context of a build for `t` under `req`: every fact comptime reads from
the target and the build options, and nothing of a project, so the build and the
editor fold the same program the same way

## fun module_load

```mach
pub fun module_load(p: *project.Project, fqn: intern.StrId) res[session.ModuleId, fail.Fail];
```

load the module `fqn` names and everything it reaches

## fun module_open

```mach
pub fun module_open(p: *project.Project, fqn: intern.StrId) res[session.ModuleId, fail.Fail];
```

parse the module `fqn` names as a root, without walking it

## fun module_reload

```mach
pub fun module_reload(p: *project.Project, mid: session.ModuleId) res[bool, fail.Fail];
```

reparse and re-walk one loaded module after its text changed, and say whether its load surface
survived, so the project's other modules stand as they are; every product the project holds of
the module is dropped, since each is keyed by the old syntax tree

## fun decorators_record

```mach
pub fun decorators_record(p: *project.Project, mid: session.ModuleId) err[fail.Fail];
```

record the strings a module's decorator arguments evaluate to

## fun parsed_definition

```mach
pub fun parsed_definition(ctx: ptr, mid: session.ModuleId) res[resolve.ParsedDefinition, fail.Fail];
```

## fun q_parse_compute

```mach
pub fun q_parse_compute(p: *project.Project, key: u64, alloc: *A.Allocator, diags: *diagnostic.DiagnosticStore) res[query.QueryOutput, fail.Fail];
```

## fun q_parse_finalize

```mach
pub fun q_parse_finalize(value: *u8, value_len: u32, alloc: *A.Allocator);
```

## fun q_exports_compute

```mach
pub fun q_exports_compute(p: *project.Project, key: u64, alloc: *A.Allocator, diags: *diagnostic.DiagnosticStore) res[query.QueryOutput, fail.Fail];
```

## fun rebuild_topo

```mach
pub fun rebuild_topo(p: *project.Project, emit_roots: *session.ModuleId, emit_root_count: u32) err[fail.Fail];
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

