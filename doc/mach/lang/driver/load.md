# mach.lang.driver.load

## val MACH_VERSION

```mach
pub val MACH_VERSION: str = version.MACH_VERSION
```

## val INITIAL_MODULE_CAP

```mach
pub val INITIAL_MODULE_CAP: u32 = 16
```

## fun entry_module_fqn

```mach
pub fun entry_module_fqn(p: *project.Project, t: *project.TargetEntry) res[intern.StrId, fail.Fail];
```

## fun compose_module_fqn

```mach
pub fun compose_module_fqn(alloc: *A.Allocator, itn: *intern.Interner, id_text: str, rel_text: str) res[intern.StrId, fail.Fail];
```

## fun fqn_in_root_project

```mach
pub fun fqn_in_root_project(p: *project.Project, fqn: intern.StrId) bool;
```

the head segment of a module path names the project that owns the module, and
the loader is where that ownership is decided: it is what picks the source
root a module is read from

## fun diag_join3

```mach
pub fun diag_join3(s: *session.Session, a: str, b: str, c: str, fallback: str) str;
```

## fun diag_join_named

```mach
pub fun diag_join_named(s: *session.Session, prefix: str, name_id: intern.StrId, suffix: str, fallback: str) str;
```

## fun join_path

```mach
pub fun join_path(alloc: *A.Allocator, a: str, b: str) res[str, fail.Fail];
```

## fun parse_root

```mach
pub fun parse_root(p: *project.Project, fqn: intern.StrId) res[session.ModuleId, fail.Fail];
```

## fun dfs_load

```mach
pub fun dfs_load(p: *project.Project, fqn: intern.StrId) res[session.ModuleId, fail.Fail];
```

## fun reload_module

```mach
pub fun reload_module(p: *project.Project, mid: session.ModuleId) res[bool, fail.Fail];
```

reparse and re-walk one loaded module in place, after its text changed, and say
whether its load surface survived: the modules it reaches, the public constants it
declares to importers' gates, its own gate outcome and its target gating. a surface
that survived leaves the project's module set, topo and every other entry as they
are, so the caller can rerun the query phases over the kept project; one that did
not needs a full load. the entry's walk state is rebuilt from a fresh comptime
context, since gate states and load marks are keyed by the old syntax tree's ids

p: the loaded project
mid: the module whose text changed
ret: ok(true) when the surface is unchanged; ok(false) when the caller must reload;
     or the parse or walk failure

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

## fun resume_deferred_gates

```mach
pub fun resume_deferred_gates(p: *project.Project) res[bool, fail.Fail];
```

## fun deferred_gate_count

```mach
pub fun deferred_gate_count(p: *project.Project) u32;
```

## rec UseTarget

```mach
pub rec UseTarget;
```

## fun check_gated_const_imports

```mach
pub fun check_gated_const_imports(p: *project.Project) err[fail.Fail];
```

## fun remerge_tuple_pub_consts

```mach
pub fun remerge_tuple_pub_consts(p: *project.Project, mid: session.ModuleId, ti: u32) res[u32, fail.Fail];
```

the imported public constants one union tuple round binds, and how many it bound

## fun eval_for_load

```mach
pub fun eval_for_load(
p: *project.Project,
mid: session.ModuleId,
source: str,
e: id.ExprId,
fw: float.FloatWidth) res[comptime.CTValue, comptime.EvalFail];
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
pub fun collect_loaded_parses(p: *project.Project, primary: err[outcome.Fail]) err[outcome.Fail];
```

