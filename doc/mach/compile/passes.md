[mach](../README.md) / [compile](README.md) / **passes**

# mach.compile.passes

the back half's query kinds, and the passes that drive a project's modules through the build

lowering, codegen, instance typing, each module's instances and the link are each a kind
declared here beside its computation. the resolve and sema passes fold a rejected
module and carry on over the rest, and an internal failure ends a pass.

Source: [`src/compile/passes.mach`](../../../src/compile/passes.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.compile.project`](project.md), [`mach.session.module`](../session/module.md), [`mach.session.query`](../session/query.md).

| Functions | |
| :--- | :--- |
| [`run_resolve_pass`](#run_resolve_pass) | a rejected module is one module's answer, so the pass folds it and carries on over the rest; an internal failure ends the pass where it met it |
| [`run_sema_pass`](#run_sema_pass) | a rejected module is one module's answer, so the pass folds it and carries on over the rest; an internal failure ends the pass where it met it |
| [`types_instances`](#types_instances) | the build types the instances module `m` asks for: its front end runs, and no object restored for it holds them |
| [`retain_modules`](#retain_modules) | every module the load reached, gated out or not, is held under the session's active retainer: the project borrows each one's parse |
| [`back_half_key`](#back_half_key) | a module's lowering and codegen products are keyed by its stable id, and by the test bit for its test object |
| [`back_half_key_is_test`](#back_half_key_is_test) | |
| [`unit_key`](#unit_key) | the key of unit `u`'s lowering and codegen: its module's normal key, or for an artifact taken whole that module's key with the whole bit |
| [`back_half_key_for`](#back_half_key_for) | the normal object's key, the same in every build that compiles the module |
| [`test_key_for`](#test_key_for) | |
| [`has_test_object`](#has_test_object) | the module gets a test object in this build: a test build, and a module whose source declares a test or a `#[testing]` declaration |
| [`codegen_reusable`](#codegen_reusable), [`test_build`](#test_build) | |
| [`prepare_codegen_pass`](#prepare_codegen_pass) | the import libraries the declarations name, which every lowering reads, registered before any is |
| [`run_test_pass`](#run_test_pass) | the test stage's computation: each test object's typed inputs, then each test object |
| [`run_codegen_pass`](#run_codegen_pass) | each module in topological order, its front-end products given back once its own unit is done |
| [`test_codegen_reusable`](#test_codegen_reusable) | the test object's codegen product is current or cached, so no worker generates it |
| [`compile_module`](#compile_module) | the mirl module `m`, the project's own, compiled into an object, which owns its storage in the session's backing allocator, the warnings mirl gave reported on `diags` |
| [`run_link_pass`](#run_link_pass) | the link stage's computation: the current objects, then the link the build's job names, which runs only when one of its inputs moved |
| [`prepare_link`](#prepare_link) | before the link stage's operation: when the artifact on disk is not what the link last wrote to `artifact`, or nothing was linked yet, LINK_ARTIFACT moves to a value it never held, the database's next revision, so the link runs again |
| [`prepare_object_cache`](#prepare_object_cache) | key the loaded modules and read `obj/` under their keys (compile/cache), reported as the cache phase carved out of resolve, which it precedes |
| [`module_in_current_project`](#module_in_current_project) | |

| Constants | |
| :--- | :--- |
| [`LOWER`](#constants) | a module's code lowered into a mirl module on a heap of its own, with the facts its lowering kept, keyed by its back-half key |
| [`DEFINED`](#constants) | the symbols a module's lowered code defines under a binding other objects see, keyed by its back-half key: each one's session id, a little-endian u32, and none for a module that declares no tests |
| [`CODEGEN`](#constants) | a module's object, mink's, and the facts its lowering kept, keyed by its back-half key: its lowered module compiled, prepared and completed by the pass (run_units), or the object the object cache staged |
| [`INSTANCE_SEMA`](#constants) | one instance typed on its own (sema_product.InstanceResult), keyed by the session's id of its symbol, which is its identity: it reads its declaring module's typed definition and the typed surfaces instance_deps names, never a module that asks for it, so it is typed once per build however many modules reach it |
| [`INSTANCES`](#constants) | every instance a module's code asks for, closed through what each instance's body asks for (compile/demand), each typed, keyed by the module's stable id |
| [`LINK`](#constants) | the build's artifact, written by the link job from the current objects |
| [`LINK_CONFIG`](#constants) | what the link is configured with beyond the build context, set by the build |
| [`LINK_ARTIFACT`](#constants) | moves whenever the artifact on disk is not the one the link last wrote, so a missing or overwritten artifact is linked again |

---

## Functions

### run_resolve_pass

```mach
pub fun run_resolve_pass(p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

a rejected module is one module's answer, so the pass folds it and carries
on over the rest; an internal failure ends the pass where it met it

### run_sema_pass

```mach
pub fun run_sema_pass(p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

a rejected module is one module's answer, so the pass folds it and carries
on over the rest; an internal failure ends the pass where it met it

### types_instances

```mach
pub fun types_instances(p: *mach.compile.project.Project, m: *mach.compile.project.ModuleEntry) bool
```

the build types the instances module `m` asks for: its front end runs, and no object restored
for it holds them

### retain_modules

```mach
pub fun retain_modules(p: *mach.compile.project.Project, rejected: bool) err[mach.base.fail.Fail]
```

every module the load reached, gated out or not, is held under the session's active retainer: the
project borrows each one's parse. a load closes its retainer's round unless a caller keeps the round
open across builds, and then whatever no retainer holds is retired

### back_half_key

```mach
pub fun back_half_key(stable: mach.session.module.StableModuleId, test_object: bool) u64
```

a module's lowering and codegen products are keyed by its stable id, and by the
test bit for its test object

### back_half_key_is_test

```mach
pub fun back_half_key_is_test(key: u64) bool
```

### unit_key

```mach
pub fun unit_key(p: *mach.compile.project.Project, u: u32) u64
```

the key of unit `u`'s lowering and codegen: its module's normal key, or for an artifact taken
whole that module's key with the whole bit

### back_half_key_for

```mach
pub fun back_half_key_for(p: *mach.compile.project.Project, m: *mach.compile.project.ModuleEntry) u64
```

the normal object's key, the same in every build that compiles the module

### test_key_for

```mach
pub fun test_key_for(m: *mach.compile.project.ModuleEntry) u64
```

### has_test_object

```mach
pub fun has_test_object(p: *mach.compile.project.Project, m: *mach.compile.project.ModuleEntry) bool
```

the module gets a test object in this build: a test build, and a module whose
source declares a test or a `#[testing]` declaration

### codegen_reusable

```mach
pub fun codegen_reusable(p: *mach.compile.project.Project, mid: mach.session.module.ModuleId) res[bool, mach.base.fail.Fail]
```

### test_build

```mach
pub fun test_build(p: *mach.compile.project.Project) bool
```

### prepare_codegen_pass

```mach
pub fun prepare_codegen_pass(p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

the import libraries the declarations name, which every lowering reads, registered before any is

### run_test_pass

```mach
pub fun run_test_pass(p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

the test stage's computation: each test object's typed inputs, then each test object

### run_codegen_pass

```mach
pub fun run_codegen_pass(p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

each module in topological order, its front-end products given back once its own unit is done.
a unit's prepare reads the syntax, resolution, per-node typing and lowering scope of its own
modules alone: what it reads of another module is that module's interface (its declarations,
constants, template copy), the typed instances, the surfaces instance typing reads and the source
identities instances are bound by, each a product of its own that outlives the release. those
made from the released products are made before it: the instances once sema is done, and the
identities before the first unit (identities_made). a read of a released product refuses
(query.get, frontend.acquire_sema). a test build lowers every module again and gives back none

### test_codegen_reusable

```mach
pub fun test_codegen_reusable(p: *mach.compile.project.Project, mid: mach.session.module.ModuleId) res[bool, mach.base.fail.Fail]
```

the test object's codegen product is current or cached, so no worker generates it

### compile_module

```mach
pub fun compile_module(p: *mach.compile.project.Project, diags: *mach.base.diagnostic.DiagnosticStore, m: *Module, name: str) res[Object, mach.base.fail.Fail]
```

the mirl module `m`, the project's own, compiled into an object, which owns its storage in the session's backing
allocator, the warnings mirl gave reported on `diags`. compiling spends the module

### run_link_pass

```mach
pub fun run_link_pass(p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

the link stage's computation: the current objects, then the link the build's job names,
which runs only when one of its inputs moved

### prepare_link

```mach
pub fun prepare_link(p: *mach.compile.project.Project, artifact: str) err[mach.base.fail.Fail]
```

before the link stage's operation: when the artifact on disk is not what the link last wrote
to `artifact`, or nothing was linked yet, LINK_ARTIFACT moves to a value it never held, the
database's next revision, so the link runs again

### prepare_object_cache

```mach
pub fun prepare_object_cache(p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

key the loaded modules and read `obj/` under their keys (compile/cache),
reported as the cache phase carved out of resolve, which it precedes

### module_in_current_project

```mach
pub fun module_in_current_project(p: *mach.compile.project.Project, fqn: mach.base.intern.StrId) bool
```

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `LOWER` | `mach.session.query.Kind` | `query.Kind{ name: "lower", role: query.DERIVED, owner: query.KEY_MODULE, origin: diagnostic.ORIGIN_LOWER, compute: lower_compute, finalize: lowered_finalize, replay: true }` | a module's code lowered into a mirl module on a heap of its own, with the facts its lowering kept, keyed by its back-half key |
| `DEFINED` | `mach.session.query.Kind` | `query.Kind{name: "defined", role: query.DERIVED, owner: query.KEY_MODULE, origin: diagnostic.ORIGIN_LOWER, compute: defined_compute}` | the symbols a module's lowered code defines under a binding other objects see, keyed by its back-half key: each one's session id, a little-endian u32, and none for a module that declares no tests |
| `CODEGEN` | `mach.session.query.Kind` | `query.Kind{ name: "codegen", role: query.DERIVED, owner: query.KEY_MODULE, origin: diagnostic.ORIGIN_CODEGEN, compute: codegen_compute, finalize: codegen_finalize, replay: true }` | a module's object, mink's, and the facts its lowering kept, keyed by its back-half key: its lowered module compiled, prepared and completed by the pass (run_units), or the object the object cache staged |
| `INSTANCE_SEMA` | `mach.session.query.Kind` | `query.Kind{ name: "instance sema", role: query.DERIVED, owner: query.KEY_UNOWNED, origin: diagnostic.ORIGIN_SEMA, compute: instance_sema_compute, finalize: instance_sema_finalize }` | one instance typed on its own (sema_product.InstanceResult), keyed by the session's id of its symbol, which is its identity: it reads its declaring module's typed definition and the typed surfaces instance_deps names, never a module that asks for it, so it is typed once per build however many modules reach it |
| `INSTANCES` | `mach.session.query.Kind` | `query.Kind{ name: "instances", role: query.DERIVED, owner: query.KEY_MODULE, origin: diagnostic.ORIGIN_SEMA, compute: instances_compute, replay: true }` | every instance a module's code asks for, closed through what each instance's body asks for (compile/demand), each typed, keyed by the module's stable id |
| `LINK` | `mach.session.query.Kind` | `query.Kind{name: "link", role: query.DERIVED, owner: query.KEY_UNOWNED, origin: diagnostic.ORIGIN_LINK, compute: link_compute}` | the build's artifact, written by the link job from the current objects |
| `LINK_CONFIG` | `mach.session.query.Kind` | `query.Kind{name: "link config", role: query.INPUT, owner: query.KEY_UNOWNED}` | what the link is configured with beyond the build context, set by the build |
| `LINK_ARTIFACT` | `mach.session.query.Kind` | `query.Kind{name: "link artifact", role: query.INPUT, owner: query.KEY_UNOWNED}` | moves whenever the artifact on disk is not the one the link last wrote, so a missing or overwritten artifact is linked again |

