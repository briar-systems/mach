# mach.lang.driver.passes

## fun read_definition

```mach
pub fun read_definition(raw: ptr, mid: session.ModuleId, phase: scx.DefinitionPhase) res[scx.Definition, fail.Fail];
```

## fun prepare_resolve_pass

```mach
pub fun prepare_resolve_pass(p: *project.Project) err[fail.Fail];
```

## fun run_resolve_pass

```mach
pub fun run_resolve_pass(p: *project.Project) err[fail.Fail];
```

## fun q_resolve_compute

```mach
pub fun q_resolve_compute(p: *project.Project, key: u64, alloc: *A.Allocator, diags: *diagnostic.DiagnosticStore) res[query.QueryOutput, fail.Fail];
```

## fun q_resolve_finalize

```mach
pub fun q_resolve_finalize(value: *u8, value_len: u32, alloc: *A.Allocator);
```

## fun prepare_sema_pass

```mach
pub fun prepare_sema_pass(p: *project.Project) err[outcome.Fail];
```

## fun run_sema_pass

```mach
pub fun run_sema_pass(p: *project.Project) err[outcome.Fail];
```

## fun acquire_sema

```mach
pub fun acquire_sema(p: *project.Project, mid: session.ModuleId) res[*sema.SemaResult, fail.Fail];
```

## fun q_sema_compute

```mach
pub fun q_sema_compute(p: *project.Project, key: u64, alloc: *A.Allocator, diags: *diagnostic.DiagnosticStore) res[query.QueryOutput, fail.Fail];
```

## fun q_sema_finalize

```mach
pub fun q_sema_finalize(value: *u8, value_len: u32, alloc: *A.Allocator);
```

## fun read_typed_surface

```mach
pub fun read_typed_surface(p: *project.Project, mid: session.ModuleId, a: *A.Allocator) res[scx.ModuleSema, fail.Fail];
```

## fun decided_gate_count

```mach
pub fun decided_gate_count(p: *project.Project) u32;
```

## fun run_gate_pass

```mach
pub fun run_gate_pass(p: *project.Project) res[bool, fail.Fail];
```

## fun clear_gate_results

```mach
pub fun clear_gate_results(p: *project.Project);
```

## fun invalidate_resolve_pass

```mach
pub fun invalidate_resolve_pass(p: *project.Project) err[fail.Fail];
```

## fun q_typed_exports_compute

```mach
pub fun q_typed_exports_compute(p: *project.Project, key: u64, alloc: *A.Allocator, diags: *diagnostic.DiagnosticStore) res[query.QueryOutput, fail.Fail];
```

## fun back_half_key

```mach
pub fun back_half_key(stable: session.StableModuleId, test_mode: bool) u64;
```

## fun back_half_key_is_test

```mach
pub fun back_half_key_is_test(key: u64) bool;
```

## fun back_half_key_for

```mach
pub fun back_half_key_for(p: *project.Project, m: *project.ModuleEntry) u64;
```

## fun codegen_reusable

```mach
pub fun codegen_reusable(p: *project.Project, mid: session.ModuleId) res[bool, fail.Fail];
```

## fun test_build

```mach
pub fun test_build(p: *project.Project) bool;
```

## fun debug_info_of

```mach
pub fun debug_info_of(p: *project.Project) res[codegen.DebugInfo, fail.Fail];
```

## fun run_lower_pass

```mach
pub fun run_lower_pass(p: *project.Project) err[outcome.Fail];
```

## fun prepare_lower_pass

```mach
pub fun prepare_lower_pass(p: *project.Project) err[outcome.Fail];
```

## fun q_cell_snapshot_compute

```mach
pub fun q_cell_snapshot_compute(p: *project.Project, key: u64, alloc: *A.Allocator, diags: *diagnostic.DiagnosticStore) res[query.QueryOutput, fail.Fail];
```

the persistent object key's inputs as one query product: every typed
definition in the cell, the target configuration and the codegen flags. its
bytes are the cell digest, so an edit anywhere in the cell advances it and
every product restored under the old digest recomputes

## fun frontend_status

```mach
pub fun frontend_status(p: *project.Project) fail.PhaseStatus;
```

## fun q_lowered_surface_compute

```mach
pub fun q_lowered_surface_compute(p: *project.Project, key: u64, alloc: *A.Allocator, diags: *diagnostic.DiagnosticStore) res[query.QueryOutput, fail.Fail];
```

## fun q_inline_bodies_compute

```mach
pub fun q_inline_bodies_compute(p: *project.Project, key: u64, alloc: *A.Allocator, diags: *diagnostic.DiagnosticStore) res[query.QueryOutput, fail.Fail];
```

## fun q_lower_compute

```mach
pub fun q_lower_compute(p: *project.Project, key: u64, alloc: *A.Allocator, diags: *diagnostic.DiagnosticStore) res[query.QueryOutput, fail.Fail];
```

## fun q_lower_finalize

```mach
pub fun q_lower_finalize(value: *u8, value_len: u32, alloc: *A.Allocator);
```

## fun prepare_codegen_pass

```mach
pub fun prepare_codegen_pass(p: *project.Project) err[outcome.Fail];
```

## fun acquire_codegen_inputs

```mach
pub fun acquire_codegen_inputs(p: *project.Project) err[outcome.Fail];
```

the modules that still lower here are those without a product restored under
this operation's snapshot; a module restored in the lower operation keeps its
staged product when the snapshot is unchanged and lowers now if it changed

## fun prepare_persistent_cache

```mach
pub fun prepare_persistent_cache(p: *project.Project) err[outcome.Fail];
```

once per query operation, after the typed definitions are current: the
compiler identity (once per project), then the cell snapshot through its
query so an unchanged cell is not rehashed, then the store directory

## fun run_codegen_pass

```mach
pub fun run_codegen_pass(p: *project.Project) err[outcome.Fail];
```

## fun code_sources

```mach
pub fun code_sources(p: *project.Project, mid: session.ModuleId, a: *A.Allocator) res[CodeSources, fail.Fail];
```

## rec CodeSources

```mach
pub rec CodeSources;
```

## fun sources_empty

```mach
pub fun sources_empty() CodeSources;
```

## fun sources_dnit

```mach
pub fun sources_dnit(a: *A.Allocator, cs: *CodeSources);
```

## fun q_codegen_compute

```mach
pub fun q_codegen_compute(p: *project.Project, key: u64, alloc: *A.Allocator, diags: *diagnostic.DiagnosticStore) res[query.QueryOutput, fail.Fail];
```

## fun q_codegen_finalize

```mach
pub fun q_codegen_finalize(value: *u8, value_len: u32, alloc: *A.Allocator);
```

## fun prepare_link_pass

```mach
pub fun prepare_link_pass(p: *project.Project) err[outcome.Fail];
```

## fun run_link_pass

```mach
pub fun run_link_pass(p: *project.Project) res[bool, outcome.Fail];
```

## fun q_link_compute

```mach
pub fun q_link_compute(p: *project.Project, key: u64, alloc: *A.Allocator, diags: *diagnostic.DiagnosticStore) res[query.QueryOutput, fail.Fail];
```

## fun capture_build_config

```mach
pub fun capture_build_config(p: *project.Project, alloc: *A.Allocator) res[query.QueryOutput, fail.Fail];
```

## fun capture_build_identity

```mach
pub fun capture_build_identity(p: *project.Project, alloc: *A.Allocator) res[query.QueryOutput, fail.Fail];
```

the configuration without the planner environment: what a persistent product
may key on. steps that ran are keyed by their own fingerprint chain, and the
inherited environment reaches an object only through them.

## fun capture_configuration_identity

```mach
pub fun capture_configuration_identity(p: *project.Project, alloc: *A.Allocator) res[query.QueryOutput, fail.Fail];
```

the build identity without the request: the persistent key hashes the request
itself, with the project root in canonical form, so the spelled root stays out

## fun set_build_config_input

```mach
pub fun set_build_config_input(p: *project.Project) err[fail.Fail];
```

## fun module_in_current_project

```mach
pub fun module_in_current_project(p: *project.Project, fqn: intern.StrId) bool;
```

