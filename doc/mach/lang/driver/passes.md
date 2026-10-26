# mach.lang.driver.passes

## val LOWERED_SURFACE

```mach
pub val LOWERED_SURFACE: query.Kind = query.Kind;
```

a module's source with what lowering omits cut away, keyed by its stable id

## val CODEGEN

```mach
pub val CODEGEN: query.Kind = query.Kind;
```

a module's object, mink's, and the facts its lowering kept, keyed by its back-half key. one
step lowers and compiles the module, so no mirl module outlives its own codegen

## val LINK

```mach
pub val LINK: query.Kind = query.Kind;
```

the link's inputs as revisions, so a link reruns only when one moved

## val LINK_CONFIG

```mach
pub val LINK_CONFIG: query.Kind = query.Kind;
```

what the link is configured with beyond the build context, set by the build

## fun run_resolve_pass

```mach
pub fun run_resolve_pass(p: *project.Project) err[fail.Fail];
```

a rejected module is one module's answer, so the pass folds it and carries
on over the rest; an internal failure ends the pass where it met it

## fun run_sema_pass

```mach
pub fun run_sema_pass(p: *project.Project) err[fail.Fail];
```

a rejected module is one module's answer, so the pass folds it and carries
on over the rest; an internal failure ends the pass where it met it

## fun retain_modules

```mach
pub fun retain_modules(p: *project.Project, rejected: bool) err[fail.Fail];
```

every module the load reached, gated out or not, is held under the session's active retainer: the
project borrows each one's parse. a load closes its retainer's round unless a caller keeps the round
open across builds, and then whatever no retainer holds is retired

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

## fun run_lower_pass

```mach
pub fun run_lower_pass(p: *project.Project) err[fail.Fail];
```

the lowering's inputs, acquired before any module is lowered: each module's typed
definition and its lowered surface. the codegen pass lowers each module in its step

## fun prepare_lower_pass

```mach
pub fun prepare_lower_pass(p: *project.Project) err[fail.Fail];
```

## fun acquire_codegen_inputs

```mach
pub fun acquire_codegen_inputs(p: *project.Project) err[fail.Fail];
```

the codegen operation's inputs: the typed definitions every step reads. the facts a
module's step keeps are read from its product as the pass takes it

## fun acquire_test_inputs

```mach
pub fun acquire_test_inputs(p: *project.Project) err[fail.Fail];
```

the test operation's inputs: the typed definitions every test object's step reads

## fun run_test_object_pass

```mach
pub fun run_test_object_pass(p: *project.Project) err[fail.Fail];
```

## fun run_codegen_pass

```mach
pub fun run_codegen_pass(p: *project.Project) err[fail.Fail];
```

## fun test_codegen_reusable

```mach
pub fun test_codegen_reusable(p: *project.Project, mid: session.ModuleId) res[bool, fail.Fail];
```

the test object's codegen product is current or cached, so no worker generates it

## fun compile_module

```mach
pub fun compile_module(p: *project.Project, m: *Module, name: str) res[Object, fail.Fail];
```

the mirl module `m` compiled into an object, which owns its storage in the session's backing
allocator. compiling spends the module

## fun run_link_pass

```mach
pub fun run_link_pass(p: *project.Project) res[bool, fail.Fail];
```

## fun prepare_object_cache

```mach
pub fun prepare_object_cache(p: *project.Project) err[fail.Fail];
```

key the loaded modules and read `obj/` under their keys (driver/cache),
reported as the cache phase carved out of resolve, which it precedes

## fun module_in_current_project

```mach
pub fun module_in_current_project(p: *project.Project, fqn: intern.StrId) bool;
```

