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

## val INSTANCE_SEMA

```mach
pub val INSTANCE_SEMA: query.Kind = query.Kind;
```

one instance typed on its own (sema_product.InstanceResult), keyed by the session's id of its
symbol, which is its identity: it reads its declaring module's typed definition and the typed
surfaces instance_deps names, never a module that asks for it

## val INSTANCE_DEMAND

```mach
pub val INSTANCE_DEMAND: query.Kind = query.Kind;
```

every instance the build's modules ask for, closed through what each instance's body asks for,
and the instance unit each emitted one falls in (driver/demand): an instance is emitted when the
code of a module the build compiles reaches it, and in a test build when its tests do. the
reports of every instance it typed are placed and reported here. a demand equal to the last
keeps its revision, so a module that starts or stops using an instance another module also
uses changes no unit. it reads the cell's records as well (cache.DEMAND_RECORDS)

## val UNIT_DEMAND

```mach
pub val UNIT_DEMAND: query.Kind = query.Kind;
```

one instance unit's members, keyed by the unit's key: each member's symbol, in the order the
unit lowers them. equal bytes keep the revision, so the demand moving elsewhere leaves the
unit's object alone

## val UNIT_CODEGEN

```mach
pub val UNIT_CODEGEN: query.Kind = query.Kind;
```

an instance unit's object and the facts its lowering kept, keyed by the unit's key: one step
lowers its members from their own typing and compiles them, as a module's codegen does

## val LINK

```mach
pub val LINK: query.Kind = query.Kind;
```

the build's artifact, written by the link job from the current objects. its product is the
artifact's path and the digest of the bytes written, and it reruns only when an input moved

## val LINK_CONFIG

```mach
pub val LINK_CONFIG: query.Kind = query.Kind;
```

what the link is configured with beyond the build context, set by the build

## val LINK_ARTIFACT

```mach
pub val LINK_ARTIFACT: query.Kind = query.Kind;
```

moves whenever the artifact on disk is not the one the link last wrote, so a missing or
overwritten artifact is linked again

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
pub fun back_half_key(stable: module.StableModuleId, test_object: bool) u64;
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
pub fun codegen_reusable(p: *project.Project, mid: module.ModuleId) res[bool, fail.Fail];
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

## fun run_test_pass

```mach
pub fun run_test_pass(p: *project.Project) err[fail.Fail];
```

the test stage's computation: each test object's typed inputs, then each test object

## fun run_codegen_pass

```mach
pub fun run_codegen_pass(p: *project.Project) err[fail.Fail];
```

each module in topological order, its front-end products given back once nothing after reads
them: an importer's lowering reads the definitions of every module it reaches, so a module is
held until the last module reaching it is lowered, and a test build lowers every module again

## fun unit_output

```mach
pub fun unit_output(p: *project.Project, base: str, origin: module.ModuleId, bucket: u32, ext: str) res[str, fail.Fail];
```

where a unit's file of kind `ext` goes under `base`: beside its origin's, as
`<module path>.instances.<bucket>.<ext>`, its directory made. the project owns the path

## fun test_codegen_reusable

```mach
pub fun test_codegen_reusable(p: *project.Project, mid: module.ModuleId) res[bool, fail.Fail];
```

the test object's codegen product is current or cached, so no worker generates it

## fun compile_module

```mach
pub fun compile_module(p: *project.Project, diags: *diagnostic.DiagnosticStore, m: *Module, name: str) res[Object, fail.Fail];
```

the mirl module `m` compiled into an object, which owns its storage in the session's backing
allocator, the warnings mirl gave reported on `diags`. compiling spends the module

## fun run_link_pass

```mach
pub fun run_link_pass(p: *project.Project) err[fail.Fail];
```

the link stage's computation: the current objects, then the link the build's job names,
which runs only when one of its inputs moved

## fun prepare_link

```mach
pub fun prepare_link(p: *project.Project, artifact: str) err[fail.Fail];
```

before the link stage's operation: when the artifact on disk is not what the link last wrote
to `artifact`, or nothing was linked yet, LINK_ARTIFACT moves to a value it never held, the
database's next revision, so the link runs again

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

