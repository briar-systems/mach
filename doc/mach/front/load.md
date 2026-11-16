[mach](../README.md) / [front](README.md) / **load**

# mach.front.load

module loading: every module a build reaches, its gates decided and its imports bound

from the modules it is asked for, the loader finds each one they import through the source
provider and has its host parse it, decides each module's declaring gates and binds the public
constants those gates read, in the build target's frame and under every target of a union
build. it leaves a load record on each module, with one `Binding` per import declaration for
name resolution to bind from. the host owns the module table and the parses, and a reload
walks one changed module again and says whether its load surface survived.

<sub>source: [`src/front/load.mach`](../../../src/front/load.mach) · imported by 8 modules</sub>

<sub>`ast_id` is [`mach.front.ast.id`](ast/id.md) · `BindingKind` is private · `intern` is [`mach.base.intern`](../base/intern.md) · `module` is [`mach.session.module`](../session/module.md) · `base_source` is [`mach.base.source`](../base/source.md) · `target_facts` is [`mach.target.facts`](../target/facts.md) · `report_phase` is [`mach.report.phase`](../report/phase.md) · `provider` is [`mach.base.source.provider`](../base/source/provider.md) · `comptime_gate` is [`mach.front.comptime.gate`](comptime/gate.md) · `comptime_scope` is [`mach.front.comptime.scope`](comptime/scope.md) · `collections_vector` is `std.collections.vector` · `Const` is private · `DemandIndex` is private · `A` is `std.allocator` · `fail` is [`mach.base.fail`](../base/fail.md) · `ast` is [`mach.front.ast`](ast.md) · `session` is [`mach.session`](../session.md) · `diagnostic` is [`mach.base.diagnostic`](../base/diagnostic.md) · `map` is `std.collections.map` · `Held` is private · `Named` is private · `Demand` is private</sub>

| Type | | |
| :--- | :--- | :--- |
| [`State`](#state) | `def` | how far the load has reached a module |
| [`Binding`](#binding) | `rec` | the record of one import declaration, which resolve binds from rather than reading its path |
| [`Tuple`](#tuple) | `rec` | one target a union build decides every gate under |
| [`Module`](#module) | `rec` | the load product of one module: the gates it decided and the constants it bound, in its context, the modules it reaches and a record of each import declaration |
| [`Row`](#row) | `rec` | a module as the load's host holds it: its identity and its load record |
| [`Host`](#host) | `rec` | what the load asks of the program that runs it, which owns the module table and the parses |
| [`Loader`](#loader) | `rec` | the front end's loading stage: from the modules it is asked for, it finds every module they reach through the source provider, decides each one's declaring gates and binds the constants those gates read, and leaves a load record on each |

| On `Module` | |
| :--- | :--- |
| [`module_dnit`](#module_dnit) | |
| [`walked`](#walked) | whether the walk reached a declaration in the module's build target's frame |
| [`binding_for`](#binding_for) | the record of what the import declaration `decl` binds, none when the load recorded none |

| On `Loader` | |
| :--- | :--- |
| [`init`](#init) | |
| [`dnit`](#dnit) | release what the loader owns; each module's record is released by its row's owner |
| [`bind`](#bind) | point the loader at what runs it, which may have moved since its last use |
| [`tuples_set`](#tuples_set) | make a union build: every gate is decided under each of `count` tuples, which the loader owns |
| [`module_find`](#module_find) | the id of the module named `fqn`, none when the load has not reached it |
| [`module_of`](#module_of) | the id of the module a load product names by stable id, MODULE_NIL when the load has not reached it |
| [`gated_at`](#gated_at) | whether the module is gated out of this build |
| [`module_count`](#module_count) | |
| [`module_load_one`](#module_load_one) | parse the module `fqn` names without walking it, as a root a later phase reads alone |
| [`module_load`](#module_load) | load the module `fqn` names and everything it reaches: each is parsed, its gates decided and its imports loaded first, then handed to the host as done |
| [`module_reload`](#module_reload) | reparse and re-walk one loaded module in place, after its text changed, and say whether its load surface survived: the modules it reaches, the public constants it declares to importers' gates, its own gate outcome and its target gating |
| [`build_reach`](#build_reach) | mark in `reached`, by module index, every module the build target's own frame reaches from the modules marked there already: through each import declaration at module scope and in each arm its frame selected |
| [`decorators_record`](#decorators_record) | records the load's reading of the module's decorator arguments, in the build target's frame the walk bound: the string each argument that takes one evaluates to, and what each `embed` argument names |

| Functions | |
| :--- | :--- |
| [`embeds_record`](#embeds_record) | records into the load scope `c` what each `embed` argument of a module names, reading the string `c` holds for it: a declaration `reached` does not mark is left out, where it marks any, and so is an argument with no string, which type checking reports |

| Constants | |
| :--- | :--- |
| [`FRAME_BUILD`](#constants) | the frame the load walk reads and binds in: the build target's, or a union tuple's by index |
| [`BINDING_MODULE`](#constants) · [`BINDING_MEMBER`](#constants) · [`BINDING_UNBOUND`](#constants) | |

---

## State

```mach
pub def State: u8
```

how far the load has reached a module

## Binding

```mach
pub rec Binding {
    decl:     ast_id.DeclId;
    kind:     BindingKind;
    name:     intern.StrId;
    module:   module.StableModuleId;
    span:     base_source.Span;
    constant: bool;
}
```

the record of one import declaration, which resolve binds from rather than reading its path

| Field | Type | |
| :--- | :--- | :--- |
| `decl` | `ast_id.DeclId` | the `use` or `fwd` |
| `kind` | `BindingKind` | what it binds |
| `name` | `intern.StrId` | the name a module binding binds it under, or the member a member binding names |
| `module` | `module.StableModuleId` | the module it names |
| `span` | `base_source.Span` | its path |
| `constant` | `bool` | a `use` of a member under the member's own name, so the member's public constant is bound for the importer's gates |

## Tuple

```mach
pub rec Tuple {
    target:        target_facts.Facts;
    os_name:       intern.StrId;
    isa_name:      intern.StrId;
    abi_name:      intern.StrId;
    platform_name: intern.StrId;
}
```

one target a union build decides every gate under

## Module

```mach
pub rec Module {
    status:       report_phase.PhaseStatus;
    home:         provider.Home;
    artifact:     intern.StrId;
    gated:        bool;
    outcome:      comptime_gate.GateOutcome;
    ctx:          comptime_scope.ComptimeCtx;
    frame:        u32;
    frames:       *comptime_scope.ComptimeCtx;
    frame_count:  u32;
    walked:       *bool;
    frame_walked: *bool;
    decl_count:   u32;
    reached:      *bool;
    reach_count:  u32;
    deps:         collections_vector.Vector[module.StableModuleId];
    consts:       collections_vector.Vector[Const];
    bindings:     collections_vector.Vector[Binding];
    binders:      DemandIndex;
}
```

the load product of one module: the gates it decided and the constants it bound, in its
context, the modules it reaches and a record of each import declaration. modules are named by
stable id

| Field | Type | |
| :--- | :--- | :--- |
| `status` | `report_phase.PhaseStatus` | the standing of the module's load |
| `home` | `provider.Home` | the package that owns it and its file there |
| `artifact` | `intern.StrId` | the artifact whose walk first reached it, what `$bin.name` reads |
| `gated` | `bool` | it was first reached from an artifact this target does not build, or a union reached it only under another target's arms (see build_reach) |
| `outcome` | `comptime_gate.GateOutcome` | the outcome of its module-scope gates |
| `ctx` | `comptime_scope.ComptimeCtx` | its build target's frame, which every later phase reads |
| `frame` | `u32` | the frame the walk is in: FRAME_BUILD or a union tuple |
| `frames` | `*comptime_scope.ComptimeCtx` | its frame under each union tuple, which only the load reads |
| `walked` | `*bool` | the declarations the walk reached in its build target's frame, by declaration |
| `frame_walked` | `*bool` | the declarations the walk reached under each union tuple, `decl_count` per tuple |
| `reached` | `*bool` | the union tuples a walk reached it under |
| `deps` | `collections_vector.Vector[module.StableModuleId]` | the modules it imports |
| `consts` | `collections_vector.Vector[Const]` | the public constants it exports to its importers' gates |
| `bindings` | `collections_vector.Vector[Binding]` | one record per import declaration the walk reached |
| `binders` | `DemandIndex` | where a demand finds each name a declaration list binds, built once per syntax (see demand_index) |

### module_dnit

```mach
pub fun module_dnit(a: *A.Allocator, m: *Module)
```

### walked

```mach
pub fun walked(m: *Module, did: ast_id.DeclId) bool
```

whether the walk reached a declaration in the module's build target's frame

### binding_for

```mach
pub fun binding_for(m: *Module, decl: ast_id.DeclId) opt[*Binding]
```

the record of what the import declaration `decl` binds, none when the load recorded none

## Row

```mach
pub rec Row {
    name:   intern.StrId;
    stable: module.StableModuleId;
    file:   base_source.FileId;
    load:   *Module;
}
```

a module as the load's host holds it: its identity and its load record

## Host

```mach
pub rec Host {
    ctx:     ptr;
    add:     fun(ptr, intern.StrId, module.StableModuleId) res[module.ModuleId, fail.Fail];
    row:     fun(ptr, module.ModuleId) Row;
    syntax:  fun(ptr, module.ModuleId) *ast.Ast;
    parse:   fun(ptr, module.ModuleId, base_source.FileId, bool) err[fail.Fail];
    context: fun(ptr, intern.StrId) res[comptime_scope.ComptimeCtx, fail.Fail];
    done:    fun(ptr, module.ModuleId) err[fail.Fail];
}
```

what the load asks of the program that runs it, which owns the module table and the parses.
the host runs each of the load's entries (module_load, module_load_one, module_reload and
decorators_record) whole, and replaces no module's syntax while one runs but through `parse`

| Field | Type | |
| :--- | :--- | :--- |
| `ctx` | `ptr` | the host's own state |
| `add` | `fun(ptr, intern.StrId, module.StableModuleId) res[module.ModuleId, fail.Fail]` | make a row for a module with this name and stable id, with no file or syntax yet, and give its id; ids are given in order from zero |
| `row` | `fun(ptr, module.ModuleId) Row` | a module's row; the load record it points at stays where it is |
| `syntax` | `fun(ptr, module.ModuleId) *ast.Ast` | a module's current syntax, nil where it has none |
| `parse` | `fun(ptr, module.ModuleId, base_source.FileId, bool) err[fail.Fail]` | give the module this file and make its syntax current; `reported` says the load reached it by walking, so a progress readout lists it |
| `context` | `fun(ptr, intern.StrId) res[comptime_scope.ComptimeCtx, fail.Fail]` | the comptime context of a module of `artifact` under the build target, with nothing of the module itself |
| `done` | `fun(ptr, module.ModuleId) err[fail.Fail]` | a module and everything it imports finished loading |

## Loader

```mach
pub rec Loader {
    s:           *session.Session;
    alloc:       *A.Allocator;
    host:        Host;
    sources:     provider.Provider;
    diags:       *diagnostic.DiagnosticStore;
    by_name:     map.Map[intern.StrId, module.ModuleId];
    by_stable:   map.Map[module.StableModuleId, module.ModuleId];
    states:      collections_vector.Vector[State];
    held:        collections_vector.Vector[Held];
    entry:       u64;
    named:       map.Map[intern.StrId, Named];
    demands:     collections_vector.Vector[Demand];
    union:       bool;
    tuples:      *Tuple;
    tuple_count: u32;
    tuple_cap:   u32;
    filter:      *bool;
    foreign:     bool;
    artifact:    intern.StrId;
    dirty:       bool;
}
```

the front end's loading stage: from the modules it is asked for, it finds every module they
reach through the source provider, decides each one's declaring gates and binds the constants
those gates read, and leaves a load record on each

| Field | Type | |
| :--- | :--- | :--- |
| `host` | `Host` | what runs the load, bound before each use |
| `sources` | `provider.Provider` | the module namespace, bound before each use |
| `diags` | `*diagnostic.DiagnosticStore` | where the load reports, bound before each use |
| `by_name` | `map.Map[intern.StrId, module.ModuleId]` | each module's id by its fqn |
| `by_stable` | `map.Map[module.StableModuleId, module.ModuleId]` | each module's id by its stable id |
| `states` | `collections_vector.Vector[State]` | how far each module is loaded, by id |
| `held` | `collections_vector.Vector[Held]` | what the loader holds of each module, by id |
| `entry` | `u64` | the entry running now, numbering each from 1 |
| `named` | `map.Map[intern.StrId, Named]` | whether each path a `use` or `fwd` spells names a module, as the provider answered the entry it was asked in |
| `demands` | `collections_vector.Vector[Demand]` | the gates and constants the load is deciding, innermost last |
| `union` | `bool` | the build decides every gate under each of `tuples` as well as its own target |
| `filter` | `*bool` | the tuples the walk is under now, nil for all of them |
| `foreign` | `bool` | the walk is under an artifact this target does not build |
| `artifact` | `intern.StrId` | the artifact a module reached now is attributed to |
| `dirty` | `bool` | a module reached under a new tuple after it loaded was walked again, which can give it dependencies the host's order placed after it |

### init

```mach
pub fun init(s: *session.Session, a: *A.Allocator) Loader
```

### dnit

```mach
pub fun dnit(l: *Loader)
```

release what the loader owns; each module's record is released by its row's owner

### bind

```mach
pub fun bind(l: *Loader, host: Host, sources: provider.Provider, diags: *diagnostic.DiagnosticStore)
```

point the loader at what runs it, which may have moved since its last use

### tuples_set

```mach
pub fun tuples_set(l: *Loader, tuples: *Tuple, count: u32, cap: u32)
```

make a union build: every gate is decided under each of `count` tuples, which the loader owns

### module_find

```mach
pub fun module_find(l: *Loader, fqn: intern.StrId) opt[module.ModuleId]
```

the id of the module named `fqn`, none when the load has not reached it

### module_of

```mach
pub fun module_of(l: *Loader, sid: module.StableModuleId) module.ModuleId
```

the id of the module a load product names by stable id, MODULE_NIL when the load has not reached it

### gated_at

```mach
pub fun gated_at(l: *Loader, mid: module.ModuleId) bool
```

whether the module is gated out of this build

### module_count

```mach
pub fun module_count(l: *Loader) u32
```

### module_load_one

```mach
pub fun module_load_one(l: *Loader, fqn: intern.StrId) res[module.ModuleId, fail.Fail]
```

parse the module `fqn` names without walking it, as a root a later phase reads alone

### module_load

```mach
pub fun module_load(l: *Loader, fqn: intern.StrId) res[module.ModuleId, fail.Fail]
```

load the module `fqn` names and everything it reaches: each is parsed, its gates decided and
its imports loaded first, then handed to the host as done

### module_reload

```mach
pub fun module_reload(l: *Loader, mid: module.ModuleId) res[bool, fail.Fail]
```

reparse and re-walk one loaded module in place, after its text changed, and say whether its load
surface survived: the modules it reaches, the public constants it declares to importers' gates,
its own gate outcome and its target gating. a surface that survived leaves every other module as
it is; one that did not needs a full load. the record is rebuilt from a fresh comptime context,
since gate states and load marks are keyed by the old syntax tree's ids

| Parameter | |
| :--- | :--- |
| `l` | the loader that loaded the module |
| `mid` | the module whose text changed |
| **returns** | ok(true) when the surface is unchanged; ok(false) when the caller must reload; or the parse or walk failure |

### build_reach

```mach
pub fun build_reach(l: *Loader, reached: *bool) err[fail.Fail]
```

mark in `reached`, by module index, every module the build target's own frame reaches from the
modules marked there already: through each import declaration at module scope and in each arm
its frame selected. a module the union loaded only under another tuple's arms stays unmarked

### decorators_record

```mach
pub fun decorators_record(l: *Loader, mid: module.ModuleId) err[fail.Fail]
```

records the load's reading of the module's decorator arguments, in the build target's frame the
walk bound: the string each argument that takes one evaluates to, and what each `embed` argument names

## Functions

### embeds_record

```mach
pub fun embeds_record(s: *session.Session, c: *comptime_scope.ComptimeCtx, a: *ast.Ast, fqn: intern.StrId,
    reached: *bool) err[fail.Fail]
```

records into the load scope `c` what each `embed` argument of a module names, reading the string
`c` holds for it: a declaration `reached` does not mark is left out, where it marks any, and so
is an argument with no string, which type checking reports

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `FRAME_BUILD` | `u32` | `0xFFFFFFFF` | the frame the load walk reads and binds in: the build target's, or a union tuple's by index |
| `BINDING_MODULE` | `BindingKind` | `0` |  |
| `BINDING_MEMBER` | `BindingKind` | `1` |  |
| `BINDING_UNBOUND` | `BindingKind` | `2` |  |

