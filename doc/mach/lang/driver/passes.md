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

a rejected module is one module's answer, so the pass folds it and carries
on over the rest; an internal failure ends the pass where it met it

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

a rejected module is one module's answer, so the pass folds it and carries
on over the rest; an internal failure ends the pass where it met it

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

## fun retain_modules

```mach
pub fun retain_modules(p: *project.Project, rejected: bool) err[fail.Fail];
```

every module the load reached, gated out or not, is held under the session's active retainer: the
project borrows each one's parse. a load closes its retainer's round unless a caller keeps the round
open across builds, and then whatever no retainer holds is retired

## fun q_typed_exports_compute

```mach
pub fun q_typed_exports_compute(p: *project.Project, key: u64, alloc: *A.Allocator, diags: *diagnostic.DiagnosticStore) res[query.QueryOutput, fail.Fail];
```

## fun back_half_key

```mach
pub fun back_half_key(stable: session.StableModuleId, test_object: bool) u64;
```

a module's lowering and codegen products are keyed by its stable id, and by the
test bit for its test object

## fun back_half_key_is_test

```mach
pub fun back_half_key_is_test(key: u64) bool;
```

## fun back_half_key_for

```mach
pub fun back_half_key_for(p: *project.Project, m: *project.ModuleEntry) u64;
```

the normal object's key, the same in every build that compiles the module

## fun test_key_for

```mach
pub fun test_key_for(m: *project.ModuleEntry) u64;
```

## fun has_test_object

```mach
pub fun has_test_object(p: *project.Project, m: *project.ModuleEntry) bool;
```

the module gets a test object in this build: a test build, and a module whose
source declares a test or a `#[testing]` declaration

## fun codegen_reusable

```mach
pub fun codegen_reusable(p: *project.Project, mid: session.ModuleId) res[bool, fail.Fail];
```

## fun test_build

```mach
pub fun test_build(p: *project.Project) bool;
```

## fun shared_artifact_build

```mach
pub fun shared_artifact_build(p: *project.Project) bool;
```

the artifact this build links is a shared library. a test build compiles the
library's own objects, so it lowers them the way the library's build does

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

## fun load_status

```mach
pub fun load_status(p: *project.Project) fail.PhaseStatus;
```

the standing of the load walk alone, over every module it reached

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

## fun acquire_test_inputs

```mach
pub fun acquire_test_inputs(p: *project.Project) err[outcome.Fail];
```

the test operation's inputs: the typed definitions and each test object's
lowered ir, unless the load restored its object from `obj/`

## fun run_test_object_pass

```mach
pub fun run_test_object_pass(p: *project.Project) err[outcome.Fail];
```

## fun run_codegen_pass

```mach
pub fun run_codegen_pass(p: *project.Project) err[outcome.Fail];
```

## fun code_sources

```mach
pub fun code_sources(p: *project.Project, mid: session.ModuleId, seed: *ir.Module, a: *A.Allocator) res[CodeSources, fail.Fail];
```

the other modules' lowered ir a whole-module backend reads to generate `seed`, module mid's ir

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

## fun run_test_object_one

```mach
pub fun run_test_object_one(p: *project.Project, mid: session.ModuleId) err[fail.Fail];
```

the module's test object, lowered against its normal object and generated

## fun run_test_lower_one

```mach
pub fun run_test_lower_one(p: *project.Project, mid: session.ModuleId) err[fail.Fail];
```

the module's test ir, which test codegen workers read before the query publishes the object

## fun test_codegen_reusable

```mach
pub fun test_codegen_reusable(p: *project.Project, mid: session.ModuleId) res[bool, fail.Fail];
```

the test object's codegen product is current or cached, so no worker generates it

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

## fun capture_configuration_identity

```mach
pub fun capture_configuration_identity(p: *project.Project, alloc: *A.Allocator) res[query.QueryOutput, fail.Fail];
```

the build identity without the request: the persistent key hashes the request
itself, with the project root in canonical form, so the spelled root stays out

## fun prepare_object_cache

```mach
pub fun prepare_object_cache(p: *project.Project) err[fail.Fail];
```

key the loaded modules and read `obj/` under their keys (driver/cache),
reported as the cache phase carved out of resolve, which it precedes

## fun set_build_config_input

```mach
pub fun set_build_config_input(p: *project.Project) err[fail.Fail];
```

## fun module_in_current_project

```mach
pub fun module_in_current_project(p: *project.Project, fqn: intern.StrId) bool;
```

