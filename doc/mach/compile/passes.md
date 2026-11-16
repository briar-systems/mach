[mach](../README.md) / [compile](README.md) / **passes**

# mach.compile.passes

the back half's query kinds, and the passes that drive a project's modules through the build

lowering, codegen, instance typing and demand, instance units and the link are each a
kind declared here beside its computation. the resolve and sema passes fold a rejected
module and carry on over the rest, and an internal failure ends a pass.

<sub>source: [`src/compile/passes.mach`](../../../src/compile/passes.mach) · imported by 6 modules</sub>

<sub>`project` is [`mach.compile.project`](project.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `module` is [`mach.session.module`](../session/module.md) · `diagnostic` is [`mach.base.diagnostic`](../base/diagnostic.md) · `intern` is [`mach.base.intern`](../base/intern.md) · `query` is [`mach.session.query`](../session/query.md)</sub>

| Functions | |
| :--- | :--- |
| [`run_resolve_pass`](#run_resolve_pass) | a rejected module is one module's answer, so the pass folds it and carries on over the rest; an internal failure ends the pass where it met it |
| [`run_sema_pass`](#run_sema_pass) | a rejected module is one module's answer, so the pass folds it and carries on over the rest; an internal failure ends the pass where it met it |
| [`retain_modules`](#retain_modules) | every module the load reached, gated out or not, is held under the session's active retainer: the project borrows each one's parse |
| [`back_half_key`](#back_half_key) | a module's lowering and codegen products are keyed by its stable id, and by the test bit for its test object |
| [`back_half_key_is_test`](#back_half_key_is_test) | |
| [`back_half_key_for`](#back_half_key_for) | the normal object's key, the same in every build that compiles the module |
| [`test_key_for`](#test_key_for) | |
| [`has_test_object`](#has_test_object) | the module gets a test object in this build: a test build, and a module whose source declares a test or a `#[testing]` declaration |
| [`codegen_reusable`](#codegen_reusable) · [`test_build`](#test_build) | |
| [`prepare_codegen_pass`](#prepare_codegen_pass) | the import libraries the declarations name, which every lowering reads, registered before any is |
| [`run_test_pass`](#run_test_pass) | the test stage's computation: each test object's typed inputs, then each test object |
| [`run_codegen_pass`](#run_codegen_pass) | each module in topological order, its front-end products given back once nothing after reads them: an importer's lowering reads the definitions of every module it reaches, so a module is held until the last module reaching it is lowered, and a test build lowers every module again |
| [`unit_output`](#unit_output) | where a unit's file of kind `ext` goes under `base`: beside its origin's, as `<module path>.instances.<bucket>.<ext>`, its directory made |
| [`test_codegen_reusable`](#test_codegen_reusable) | the test object's codegen product is current or cached, so no worker generates it |
| [`compile_module`](#compile_module) | the mirl module `m` compiled into an object, which owns its storage in the session's backing allocator, the warnings mirl gave reported on `diags` |
| [`run_link_pass`](#run_link_pass) | the link stage's computation: the current objects, then the link the build's job names, which runs only when one of its inputs moved |
| [`prepare_link`](#prepare_link) | before the link stage's operation: when the artifact on disk is not what the link last wrote to `artifact`, or nothing was linked yet, LINK_ARTIFACT moves to a value it never held, the database's next revision, so the link runs again |
| [`prepare_object_cache`](#prepare_object_cache) | key the loaded modules and read `obj/` under their keys (compile/cache), reported as the cache phase carved out of resolve, which it precedes |
| [`module_in_current_project`](#module_in_current_project) | |

| Constants | |
| :--- | :--- |
| [`LOWER`](#constants) | a module's code lowered into a mirl module on a heap of its own, with the facts its lowering kept, keyed by its back-half key |
| [`DEFINED`](#constants) | the symbols a module's lowered code defines under a binding other objects see, keyed by its back-half key: each one's session id, a little-endian u32, and none for a module that declares no tests |
| [`CODEGEN`](#constants) | a module's object, mink's, and the facts its lowering kept, keyed by its back-half key: its lowered module compiled |
| [`INSTANCE_SEMA`](#constants) | one instance typed on its own (sema_product.InstanceResult), keyed by the session's id of its symbol, which is its identity: it reads its declaring module's typed definition and the typed surfaces instance_deps names, never a module that asks for it |
| [`INSTANCE_DEMAND`](#constants) | every instance the build's modules ask for, closed through what each instance's body asks for, and the instance unit each emitted one falls in (compile/demand): an instance is emitted when the code of a module the build compiles reaches it, and in a test build when its tests do |
| [`UNIT_DEMAND`](#constants) | one instance unit's members, keyed by the unit's key: each member's symbol, in the order the unit lowers them |
| [`UNIT_LOWER`](#constants) | an instance unit's members lowered from their own typing into a mirl module, keyed by the unit's key, read once by its codegen as a module's lowering is |
| [`UNIT_CODEGEN`](#constants) | an instance unit's object and the facts its lowering kept, keyed by the unit's key: its lowered unit compiled, as a module's codegen does |
| [`LINK`](#constants) | the build's artifact, written by the link job from the current objects |
| [`LINK_CONFIG`](#constants) | what the link is configured with beyond the build context, set by the build |
| [`LINK_ARTIFACT`](#constants) | moves whenever the artifact on disk is not the one the link last wrote, so a missing or overwritten artifact is linked again |

---

## Functions

### run_resolve_pass

```mach
pub fun run_resolve_pass(p: *project.Project) err[fail.Fail]
```

a rejected module is one module's answer, so the pass folds it and carries
on over the rest; an internal failure ends the pass where it met it

### run_sema_pass

```mach
pub fun run_sema_pass(p: *project.Project) err[fail.Fail]
```

a rejected module is one module's answer, so the pass folds it and carries
on over the rest; an internal failure ends the pass where it met it

### retain_modules

```mach
pub fun retain_modules(p: *project.Project, rejected: bool) err[fail.Fail]
```

every module the load reached, gated out or not, is held under the session's active retainer: the
project borrows each one's parse. a load closes its retainer's round unless a caller keeps the round
open across builds, and then whatever no retainer holds is retired

### back_half_key

```mach
pub fun back_half_key(stable: module.StableModuleId, test_object: bool) u64
```

a module's lowering and codegen products are keyed by its stable id, and by the
test bit for its test object

### back_half_key_is_test

```mach
pub fun back_half_key_is_test(key: u64) bool
```

### back_half_key_for

```mach
pub fun back_half_key_for(p: *project.Project, m: *project.ModuleEntry) u64
```

the normal object's key, the same in every build that compiles the module

### test_key_for

```mach
pub fun test_key_for(m: *project.ModuleEntry) u64
```

### has_test_object

```mach
pub fun has_test_object(p: *project.Project, m: *project.ModuleEntry) bool
```

the module gets a test object in this build: a test build, and a module whose
source declares a test or a `#[testing]` declaration

### codegen_reusable

```mach
pub fun codegen_reusable(p: *project.Project, mid: module.ModuleId) res[bool, fail.Fail]
```

### test_build

```mach
pub fun test_build(p: *project.Project) bool
```

### prepare_codegen_pass

```mach
pub fun prepare_codegen_pass(p: *project.Project) err[fail.Fail]
```

the import libraries the declarations name, which every lowering reads, registered before any is

### run_test_pass

```mach
pub fun run_test_pass(p: *project.Project) err[fail.Fail]
```

the test stage's computation: each test object's typed inputs, then each test object

### run_codegen_pass

```mach
pub fun run_codegen_pass(p: *project.Project) err[fail.Fail]
```

each module in topological order, its front-end products given back once nothing after reads
them: an importer's lowering reads the definitions of every module it reaches, so a module is
held until the last module reaching it is lowered, and a test build lowers every module again

### unit_output

```mach
pub fun unit_output(p: *project.Project, base: str, origin: module.ModuleId, bucket: u32, ext: str) res[str, fail.Fail]
```

where a unit's file of kind `ext` goes under `base`: beside its origin's, as
`<module path>.instances.<bucket>.<ext>`, its directory made. the project owns the path

### test_codegen_reusable

```mach
pub fun test_codegen_reusable(p: *project.Project, mid: module.ModuleId) res[bool, fail.Fail]
```

the test object's codegen product is current or cached, so no worker generates it

### compile_module

```mach
pub fun compile_module(p: *project.Project, diags: *diagnostic.DiagnosticStore, m: *Module, name: str) res[Object, fail.Fail]
```

the mirl module `m` compiled into an object, which owns its storage in the session's backing
allocator, the warnings mirl gave reported on `diags`. compiling spends the module

### run_link_pass

```mach
pub fun run_link_pass(p: *project.Project) err[fail.Fail]
```

the link stage's computation: the current objects, then the link the build's job names,
which runs only when one of its inputs moved

### prepare_link

```mach
pub fun prepare_link(p: *project.Project, artifact: str) err[fail.Fail]
```

before the link stage's operation: when the artifact on disk is not what the link last wrote
to `artifact`, or nothing was linked yet, LINK_ARTIFACT moves to a value it never held, the
database's next revision, so the link runs again

### prepare_object_cache

```mach
pub fun prepare_object_cache(p: *project.Project) err[fail.Fail]
```

key the loaded modules and read `obj/` under their keys (compile/cache),
reported as the cache phase carved out of resolve, which it precedes

### module_in_current_project

```mach
pub fun module_in_current_project(p: *project.Project, fqn: intern.StrId) bool
```

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `LOWER` | `query.Kind` | `query.Kind{ name: "lower", role: query.DERIVED, owner: query.KEY_MODULE, origin: diagnostic.ORIGIN_LOWER, compute: lower_compute, finalize: lowered_finalize, replay: true }` | a module's code lowered into a mirl module on a heap of its own, with the facts its lowering kept, keyed by its back-half key |
| `DEFINED` | `query.Kind` | `query.Kind{name: "defined", role: query.DERIVED, owner: query.KEY_MODULE, origin: diagnostic.ORIGIN_LOWER, compute: defined_compute}` | the symbols a module's lowered code defines under a binding other objects see, keyed by its back-half key: each one's session id, a little-endian u32, and none for a module that declares no tests |
| `CODEGEN` | `query.Kind` | `query.Kind{ name: "codegen", role: query.DERIVED, owner: query.KEY_MODULE, origin: diagnostic.ORIGIN_CODEGEN, compute: codegen_compute, finalize: codegen_finalize, replay: true }` | a module's object, mink's, and the facts its lowering kept, keyed by its back-half key: its lowered module compiled |
| `INSTANCE_SEMA` | `query.Kind` | `query.Kind{ name: "instance sema", role: query.DERIVED, owner: query.KEY_UNOWNED, origin: diagnostic.ORIGIN_SEMA, compute: instance_sema_compute, finalize: instance_sema_finalize }` | one instance typed on its own (sema_product.InstanceResult), keyed by the session's id of its symbol, which is its identity: it reads its declaring module's typed definition and the typed surfaces instance_deps names, never a module that asks for it |
| `INSTANCE_DEMAND` | `query.Kind` | `query.Kind{ name: "instance demand", role: query.DERIVED, owner: query.KEY_UNOWNED, origin: diagnostic.ORIGIN_SEMA, compute: instance_demand_compute, finalize: instance_demand_finalize, equal: instance_demand_equal }` | every instance the build's modules ask for, closed through what each instance's body asks for, and the instance unit each emitted one falls in (compile/demand): an instance is emitted when the code of a module the build compiles reaches it, and in a test build when its tests do |
| `UNIT_DEMAND` | `query.Kind` | `query.Kind{name: "unit demand", role: query.DERIVED, owner: query.KEY_UNIT, origin: diagnostic.ORIGIN_LOWER, compute: unit_demand_compute}` | one instance unit's members, keyed by the unit's key: each member's symbol, in the order the unit lowers them |
| `UNIT_LOWER` | `query.Kind` | `query.Kind{ name: "unit lower", role: query.DERIVED, owner: query.KEY_UNIT, origin: diagnostic.ORIGIN_LOWER, compute: unit_lower_compute, finalize: lowered_finalize, replay: true }` | an instance unit's members lowered from their own typing into a mirl module, keyed by the unit's key, read once by its codegen as a module's lowering is |
| `UNIT_CODEGEN` | `query.Kind` | `query.Kind{ name: "unit codegen", role: query.DERIVED, owner: query.KEY_UNIT, origin: diagnostic.ORIGIN_CODEGEN, compute: unit_codegen_compute, finalize: codegen_finalize }` | an instance unit's object and the facts its lowering kept, keyed by the unit's key: its lowered unit compiled, as a module's codegen does |
| `LINK` | `query.Kind` | `query.Kind{name: "link", role: query.DERIVED, owner: query.KEY_UNOWNED, origin: diagnostic.ORIGIN_LINK, compute: link_compute}` | the build's artifact, written by the link job from the current objects |
| `LINK_CONFIG` | `query.Kind` | `query.Kind{name: "link config", role: query.INPUT, owner: query.KEY_UNOWNED}` | what the link is configured with beyond the build context, set by the build |
| `LINK_ARTIFACT` | `query.Kind` | `query.Kind{name: "link artifact", role: query.INPUT, owner: query.KEY_UNOWNED}` | moves whenever the artifact on disk is not the one the link last wrote, so a missing or overwritten artifact is linked again |

