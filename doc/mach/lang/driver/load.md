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
pub fun remerge_tuple_pub_consts(p: *project.Project, mid: session.ModuleId, ti: u32) res[bool, fail.Fail];
```

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

