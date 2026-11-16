[mach](../README.md) / [compile](README.md) / **frontend**

# mach.compile.frontend

the front end's products: a module's parse, its resolution and the surface it exports, its
types and the typed surface importers read, each a kind declared here beside its computation.
a computation reads what it depends on through the query engine, and a host reads a product
through the readers here: inside an operation by acquiring it, between operations as this
project's operations last published it

<sub>source: [`src/compile/frontend.mach`](../../../src/compile/frontend.mach) · imported by 11 modules</sub>

<sub>`ast` is [`mach.front.ast`](../front/ast.md) · `report_phase` is [`mach.report.phase`](../report/phase.md) · `project` is [`mach.compile.project`](project.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `resolve` is [`mach.front.resolve`](../front/resolve.md) · `sema_product` is [`mach.front.sema.product`](../front/sema/product.md) · `module` is [`mach.session.module`](../session/module.md) · `sema_instance` is [`mach.front.sema.instance`](../front/sema/instance.md) · `collections_vector` is `std.collections.vector` · `query` is [`mach.session.query`](../session/query.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`ParsedModule`](#parsedmodule) | `rec` | a parse product: the syntax tree it owns and how the parse stood |

| On `ParsedModule` | |
| :--- | :--- |
| [`parse`](#parse) | a module's parse in the open operation |
| [`parsed`](#parsed) | a module's parse as this project's operations last published it |

| Functions | |
| :--- | :--- |
| [`resolution`](#resolution) | a module's resolution in the open operation |
| [`syntax`](#syntax) | a module's syntax tree: inside an operation the parse it acquires, which a computation reading it depends on, and between operations the one this project's operations last published |
| [`resolved`](#resolved) | a module's resolution as this project's operations last published it, nil when none did |
| [`typed`](#typed) | a module's types as this project's operations last published them, nil when none did |
| [`load_status`](#load_status) | the standing of the load walk alone, over every module it reached |
| [`module_status`](#module_status) | the standing of one module: its load's and that of each product this project's operations published for it |
| [`frontend_status`](#frontend_status) | the standing of the front end over every module |
| [`read_definition`](#read_definition) | a module's definition as far as a phase reads it, through the products of this operation |
| [`acquire_sema`](#acquire_sema) | a module's sema product in the open operation, its field graph installed in the projection |
| [`record_load_views`](#record_load_views) | set each module's load view from what the walk left it; an unchanged view is no change |
| [`instance_deps`](#instance_deps) | what typing the instance `item` reads besides its declaring module's definition: the typed surfaces of every module the declaring module imports, and of every module declaring a type its arguments or pack name with what each of those imports, transitively, each field graph installed |
| [`instance_reads`](#instance_reads) | the modules the instance `item` names: its declaring module first, then every module declaring a type its arguments or pack name |
| [`instance_deps_dnit`](#instance_deps_dnit) | release what instance_deps made |
| [`begin_query_phase`](#begin_query_phase) | open a query operation over the project: the build context must be as it was frozen, and the type projections the last operation installed are reset |
| [`refresh_diagnostics`](#refresh_diagnostics) | the session's store is rebuilt from the project's and the query presentation's, dropping the warnings a declaration's `#[expect]` acknowledges and the ones the profile's `allow` silences, then reporting each judged expectation nothing fulfilled |
| [`products_dnit`](#products_dnit) | give back a lowered module's front-end products, which nothing the build runs after its lowering reads: its syntax tree, its resolution, its lowering scope and what its typing holds beyond the interface its importers' lowerings read |
| [`finish_query_phase`](#finish_query_phase) | close a query operation over the project and show it: end it, hold its failure where the session presents it, collect the diagnostics of every product the roots reach into the session's presentation and refresh the session's store from it |

| Constants | |
| :--- | :--- |
| [`PARSE`](#constants) | a file's parse: the syntax tree and how the parse stood, keyed by the file |
| [`LOAD_VIEW`](#constants) | the digest of what the load walk decided for a module, keyed by its stable id |
| [`MODULE_NUMBER`](#constants) | the module id this build numbers a module with, keyed by its stable id |
| [`RESOLVE`](#constants) | a module's resolution, keyed by its stable id |
| [`EXPORTS`](#constants) | what a module's resolution exports to an importer, keyed by its stable id |
| [`SEMA`](#constants) | a module's types, keyed by its stable id |
| [`TYPED_EXPORTS`](#constants) | the typed surface a module exports to its importers, shared by every one, keyed by its stable id |
| [`INSTANCE_SURFACES`](#constants) | the typed surfaces of every module a module imports, transitively, keyed by its stable id: what an instance of the module's templates reads of the modules around it |

---

## ParsedModule

```mach
pub rec ParsedModule {
    a:      *ast.Ast;
    status: report_phase.PhaseStatus;
}
```

a parse product: the syntax tree it owns and how the parse stood

### parse

```mach
pub fun parse(p: *project.Project, m: *project.ModuleEntry) res[ParsedModule, fail.Fail]
```

a module's parse in the open operation

### parsed

```mach
pub fun parsed(p: *project.Project, m: *project.ModuleEntry) opt[ParsedModule]
```

a module's parse as this project's operations last published it

## Functions

### resolution

```mach
pub fun resolution(p: *project.Project, m: *project.ModuleEntry) res[*resolve.ResolveResult, fail.Fail]
```

a module's resolution in the open operation

### syntax

```mach
pub fun syntax(p: *project.Project, m: *project.ModuleEntry) *ast.Ast
```

a module's syntax tree: inside an operation the parse it acquires, which a computation reading
it depends on, and between operations the one this project's operations last published. nil
when the module has none, and inside an operation when the parse could not be acquired: the
operation keeps that failure and ends with it, so nothing built on the nil is published

### resolved

```mach
pub fun resolved(p: *project.Project, m: *project.ModuleEntry) *resolve.ResolveResult
```

a module's resolution as this project's operations last published it, nil when none did

### typed

```mach
pub fun typed(p: *project.Project, m: *project.ModuleEntry) *sema_product.SemaResult
```

a module's types as this project's operations last published them, nil when none did

### load_status

```mach
pub fun load_status(p: *project.Project) report_phase.PhaseStatus
```

the standing of the load walk alone, over every module it reached

### module_status

```mach
pub fun module_status(p: *project.Project, m: *project.ModuleEntry) report_phase.PhaseStatus
```

the standing of one module: its load's and that of each product this project's operations
published for it

### frontend_status

```mach
pub fun frontend_status(p: *project.Project) report_phase.PhaseStatus
```

the standing of the front end over every module

### read_definition

```mach
pub fun read_definition(raw: ptr, mid: module.ModuleId, phase: sema_product.DefinitionPhase) res[sema_product.Definition, fail.Fail]
```

a module's definition as far as a phase reads it, through the products of this operation

### acquire_sema

```mach
pub fun acquire_sema(p: *project.Project, mid: module.ModuleId) res[*sema_product.SemaResult, fail.Fail]
```

a module's sema product in the open operation, its field graph installed in the projection

### record_load_views

```mach
pub fun record_load_views(p: *project.Project) err[fail.Fail]
```

set each module's load view from what the walk left it; an unchanged view is no change

### instance_deps

```mach
pub fun instance_deps(p: *project.Project, item: *sema_instance.Instance) res[sema_product.SemaDeps, fail.Fail]
```

what typing the instance `item` reads besides its declaring module's definition: the typed
surfaces of every module the declaring module imports, and of every module declaring a type
its arguments or pack name with what each of those imports, transitively, each field graph
installed. reading them is what makes the instance's typing depend on them

### instance_reads

```mach
pub fun instance_reads(p: *project.Project, item: *sema_instance.Instance, out: *collections_vector.Vector[module.ModuleId]) err[fail.Fail]
```

the modules the instance `item` names: its declaring module first, then every module declaring
a type its arguments or pack name

### instance_deps_dnit

```mach
pub fun instance_deps_dnit(p: *project.Project, deps: *sema_product.SemaDeps)
```

release what instance_deps made

### begin_query_phase

```mach
pub fun begin_query_phase(p: *project.Project) res[query.Operation, fail.Fail]
```

open a query operation over the project: the build context must be as it was frozen, and the
type projections the last operation installed are reset

### refresh_diagnostics

```mach
pub fun refresh_diagnostics(p: *project.Project) err[fail.Fail]
```

the session's store is rebuilt from the project's and the query presentation's,
dropping the warnings a declaration's `#[expect]` acknowledges and the ones the
profile's `allow` silences, then reporting each judged expectation nothing
fulfilled. a warning replayed from the cache passes through here as a fresh one does

### products_dnit

```mach
pub fun products_dnit(p: *project.Project, mid: module.ModuleId) err[fail.Fail]
```

give back a lowered module's front-end products, which nothing the build runs after its
lowering reads: its syntax tree, its resolution, its lowering scope and what its typing holds
beyond the interface its importers' lowerings read. its expectations are collected first and
held, so a later refresh judges them as it would have. called inside the operation that
lowered it, once its own products and every reader of them are done

### finish_query_phase

```mach
pub fun finish_query_phase(p: *project.Project, operation: query.Operation,
    roots: *query.QueryKey, count: usize,
    result: err[fail.Fail]) err[fail.Fail]
```

close a query operation over the project and show it: end it, hold its failure where the
session presents it, collect the diagnostics of every product the roots reach into the
session's presentation and refresh the session's store from it

| Parameter | |
| :--- | :--- |
| `p` | the project |
| `operation` | the operation begin_query_phase opened |
| `roots` | the products whose diagnostics the phase presents |
| `count` | how many roots |
| `result` | the phase's own result; an operation failure that is a message displaces it; given back unless it is ok and the refresh fails |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `PARSE` | `query.Kind` | `query.Kind{ name: "parse", role: query.DERIVED, owner: query.KEY_FILE, origin: diagnostic.ORIGIN_LOAD, compute: parse_compute, finalize: parse_finalize }` | a file's parse: the syntax tree and how the parse stood, keyed by the file |
| `LOAD_VIEW` | `query.Kind` | `query.Kind{name: "load view", role: query.INPUT, owner: query.KEY_MODULE}` | the digest of what the load walk decided for a module, keyed by its stable id |
| `MODULE_NUMBER` | `query.Kind` | `query.Kind{name: "module number", role: query.INPUT, owner: query.KEY_MODULE}` | the module id this build numbers a module with, keyed by its stable id |
| `RESOLVE` | `query.Kind` | `query.Kind{ name: "resolve", role: query.DERIVED, owner: query.KEY_MODULE, origin: diagnostic.ORIGIN_RESOLVE, compute: resolve_compute, finalize: resolve_finalize }` | a module's resolution, keyed by its stable id |
| `EXPORTS` | `query.Kind` | `query.Kind{ name: "exports", role: query.DERIVED, owner: query.KEY_MODULE, origin: diagnostic.ORIGIN_RESOLVE, compute: exports_compute, finalize: exports_finalize, equal: exports_equal }` | what a module's resolution exports to an importer, keyed by its stable id |
| `SEMA` | `query.Kind` | `query.Kind{ name: "sema", role: query.DERIVED, owner: query.KEY_MODULE, origin: diagnostic.ORIGIN_SEMA, compute: sema_compute, finalize: sema_finalize }` | a module's types, keyed by its stable id |
| `TYPED_EXPORTS` | `query.Kind` | `query.Kind{ name: "typed exports", role: query.DERIVED, owner: query.KEY_MODULE, origin: diagnostic.ORIGIN_SEMA, compute: typed_exports_compute, finalize: surface_finalize, equal: surface_equal }` | the typed surface a module exports to its importers, shared by every one, keyed by its stable id |
| `INSTANCE_SURFACES` | `query.Kind` | `query.Kind{ name: "instance surfaces", role: query.DERIVED, owner: query.KEY_MODULE, origin: diagnostic.ORIGIN_SEMA, compute: instance_surfaces_compute, finalize: instance_surfaces_finalize }` | the typed surfaces of every module a module imports, transitively, keyed by its stable id: what an instance of the module's templates reads of the modules around it |

