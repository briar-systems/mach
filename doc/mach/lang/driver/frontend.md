# mach.lang.driver.frontend

the front end's products: a module's parse, its resolution and the surface it exports, its
types and the typed surface importers read, each a kind declared here beside its computation.
a computation reads what it depends on through the query engine, and a host reads a product
through the readers here: inside an operation by acquiring it, between operations as this
project's operations last published it

## val PARSE

```mach
pub val PARSE: query.Kind = query.Kind;
```

a file's parse: the syntax tree and how the parse stood, keyed by the file

## val LOAD_VIEW

```mach
pub val LOAD_VIEW: query.Kind = query.Kind;
```

the digest of what the load walk decided for a module, keyed by its stable id

## val MODULE_NUMBER

```mach
pub val MODULE_NUMBER: query.Kind = query.Kind;
```

the module id this build numbers a module with, keyed by its stable id

## val RESOLVE

```mach
pub val RESOLVE: query.Kind = query.Kind;
```

a module's resolution, keyed by its stable id

## val EXPORTS

```mach
pub val EXPORTS: query.Kind = query.Kind;
```

what a module's resolution exports to an importer, keyed by its stable id

## val SEMA

```mach
pub val SEMA: query.Kind = query.Kind;
```

a module's types, keyed by its stable id

## val TYPED_EXPORTS

```mach
pub val TYPED_EXPORTS: query.Kind = query.Kind;
```

the typed surface a module exports to its importers, shared by every one, keyed by its stable id

## fun parse

```mach
pub fun parse(p: *project.Project, m: *project.ModuleEntry) res[ParsedModule, fail.Fail];
```

a module's parse in the open operation

## fun resolution

```mach
pub fun resolution(p: *project.Project, m: *project.ModuleEntry) res[*resolve.ResolveResult, fail.Fail];
```

a module's resolution in the open operation

## fun syntax

```mach
pub fun syntax(p: *project.Project, m: *project.ModuleEntry) *ast.Ast;
```

a module's syntax tree: inside an operation the parse it acquires, which a computation reading
it depends on, and between operations the one this project's operations last published. nil
when the module has none, and inside an operation when the parse could not be acquired: the
operation keeps that failure and ends with it, so nothing built on the nil is published

## fun parsed

```mach
pub fun parsed(p: *project.Project, m: *project.ModuleEntry) opt[ParsedModule];
```

a module's parse as this project's operations last published it

## fun resolved

```mach
pub fun resolved(p: *project.Project, m: *project.ModuleEntry) *resolve.ResolveResult;
```

a module's resolution as this project's operations last published it, nil when none did or its
front end was skipped

## fun typed

```mach
pub fun typed(p: *project.Project, m: *project.ModuleEntry) *sema_product.SemaResult;
```

a module's types as this project's operations last published them, nil when none did or its
front end was skipped

## fun load_status

```mach
pub fun load_status(p: *project.Project) lang_phase.PhaseStatus;
```

the standing of the load walk alone, over every module it reached

## fun module_status

```mach
pub fun module_status(p: *project.Project, m: *project.ModuleEntry) lang_phase.PhaseStatus;
```

the standing of one module: its load's and that of each product this project's operations
published for it

## fun frontend_status

```mach
pub fun frontend_status(p: *project.Project) lang_phase.PhaseStatus;
```

the standing of the front end over every module

## rec ParsedModule

```mach
pub rec ParsedModule;
```

a parse product: the syntax tree it owns and how the parse stood

## fun read_definition

```mach
pub fun read_definition(raw: ptr, mid: module.ModuleId, phase: sema_product.DefinitionPhase) res[sema_product.Definition, fail.Fail];
```

a module's definition as far as a phase reads it, through the products of this operation

## fun acquire_sema

```mach
pub fun acquire_sema(p: *project.Project, mid: module.ModuleId) res[*sema_product.SemaResult, fail.Fail];
```

a module's sema product in the open operation, its field graph installed in the projection

## fun record_load_views

```mach
pub fun record_load_views(p: *project.Project) err[fail.Fail];
```

set each module's load view from what the walk left it; an unchanged view is no change

## fun begin_query_phase

```mach
pub fun begin_query_phase(p: *project.Project) res[query.Operation, fail.Fail];
```

open a query operation over the project: the build context must be as it was frozen, and the
type projections the last operation installed are reset

## fun refresh_diagnostics

```mach
pub fun refresh_diagnostics(p: *project.Project) err[fail.Fail];
```

the session's store is rebuilt from the project's and the query presentation's,
dropping the warnings a declaration's `#[expect]` acknowledges and the ones the
profile's `allow` silences, then reporting each judged expectation nothing
fulfilled. a warning replayed from the cache passes through here as a fresh one does

## fun release_lowered

```mach
pub fun release_lowered(p: *project.Project, mid: module.ModuleId) err[fail.Fail];
```

give back a lowered module's front-end products, which nothing the build runs after its
lowering reads: its syntax tree, its resolution, its lowering scope and what its typing holds
beyond the interface its importers' lowerings read. its expectations are collected first and
held, so a later refresh judges them as it would have. called inside the operation that
lowered it, once its own products and every reader of them are done

## fun finish_query_phase

```mach
pub fun finish_query_phase(p: *project.Project, operation: query.Operation,
roots: *query.QueryKey, count: usize,
result: err[fail.Fail]) err[fail.Fail];
```

close a query operation over the project and show it: end it, hold its failure where the
session presents it, collect the diagnostics of every product the roots reach into the
session's presentation and refresh the session's store from it

p: the project
operation: the operation begin_query_phase opened
roots: the products whose diagnostics the phase presents
count: how many roots
result: the phase's own result; an operation failure that is a message displaces it; given
           back unless it is ok and the refresh fails

