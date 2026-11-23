[mach](../README.md) / [front](README.md) / **load**

# mach.front.load

module loading: every module a build reaches, its gates decided and its imports bound

from the modules it is asked for, the loader finds each one they import through the source
provider and has its host parse it, decides each module's declaring gates and binds the public
constants those gates read, in the build target's frame and under every target of a union
build. it leaves a load record on each module, with one `Binding` per import declaration for
name resolution to bind from. the host owns the module table and the parses, and a reload
walks one changed module again and says whether its load surface survived.

Source: [`src/front/load.mach`](../../../src/front/load.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.base.source`](../base/source.md), [`mach.base.source.provider`](../base/source/provider.md), [`mach.front.ast`](ast.md), [`mach.front.ast.id`](ast/id.md), [`mach.front.comptime.gate`](comptime/gate.md), [`mach.front.comptime.scope`](comptime/scope.md), [`mach.report.phase`](../report/phase.md), [`mach.session`](../session.md), [`mach.session.module`](../session/module.md), [`mach.target.facts`](../target/facts.md).

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
| [`module_dnit`](#module_dnit) | release what a module's load record owns: its contexts, walk marks, union frames, lists and demand index |
| [`walked`](#walked) | whether the walk reached a declaration in the module's build target's frame |
| [`binding_for`](#binding_for) | the record of what the import declaration `decl` binds, none when the load recorded none |

| On `Loader` | |
| :--- | :--- |
| [`init`](#init) | a loader with no modules, no host and no union tuples |
| [`dnit`](#dnit) | release what the loader owns; each module's record is released by its row's owner |
| [`bind`](#bind) | point the loader at what runs it, which may have moved since its last use |
| [`unbind`](#unbind) | drop the host `bind` gave, whose context lives only as long as the walk it serves |
| [`tuples_set`](#tuples_set) | make a union build: every gate is decided under each of `count` tuples, which the loader owns |
| [`module_find`](#module_find) | the id of the module named `fqn`, none when the load has not reached it |
| [`module_of`](#module_of) | the id of the module a load product names by stable id, MODULE_NIL when the load has not reached it |
| [`gated_at`](#gated_at) | whether the module is gated out of this build |
| [`module_count`](#module_count) | how many modules the load has reached |
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
| [`BINDING_MODULE`](#constants), [`BINDING_MEMBER`](#constants), [`BINDING_UNBOUND`](#constants) | |

---

## State

```mach
pub def State: u8
```

how far the load has reached a module

## Binding

```mach
pub rec Binding {
    decl:     mach.front.ast.id.DeclId;
    kind:     BindingKind;
    name:     mach.base.intern.StrId;
    module:   mach.session.module.StableModuleId;
    span:     mach.base.source.Span;
    constant: bool;
}
```

the record of one import declaration, which resolve binds from rather than reading its path

| Field | Type | |
| :--- | :--- | :--- |
| `decl` | `mach.front.ast.id.DeclId` | the `use` or `fwd` |
| `kind` | `BindingKind` | what it binds |
| `name` | `mach.base.intern.StrId` | the name a module binding binds it under, or the member a member binding names |
| `module` | `mach.session.module.StableModuleId` | the module it names |
| `span` | `mach.base.source.Span` | its path |
| `constant` | `bool` | a `use` of a member under the member's own name, so the member's public constant is bound for the importer's gates |

## Tuple

```mach
pub rec Tuple {
    target:        mach.target.facts.Facts;
    os_name:       mach.base.intern.StrId;
    isa_name:      mach.base.intern.StrId;
    abi_name:      mach.base.intern.StrId;
    platform_name: mach.base.intern.StrId;
}
```

one target a union build decides every gate under

## Module

```mach
pub rec Module {
    status:       mach.report.phase.PhaseStatus;
    home:         mach.base.source.provider.Home;
    artifact:     mach.base.intern.StrId;
    gated:        bool;
    outcome:      mach.front.comptime.gate.GateOutcome;
    ctx:          mach.front.comptime.scope.ComptimeCtx;
    frame:        u32;
    frames:       *mach.front.comptime.scope.ComptimeCtx;
    frame_count:  u32;
    walked:       *bool;
    frame_walked: *bool;
    decl_count:   u32;
    reached:      *bool;
    reach_count:  u32;
    deps:         std.collections.vector.Vector[mach.session.module.StableModuleId];
    lowers:       std.collections.vector.Vector[mach.session.module.StableModuleId];
    consts:       std.collections.vector.Vector[Const];
    bindings:     std.collections.vector.Vector[Binding];
    binders:      DemandIndex;
}
```

the load product of one module: the gates it decided and the constants it bound, in its
context, the modules it reaches and a record of each import declaration. modules are named by
stable id

| Field | Type | |
| :--- | :--- | :--- |
| `status` | `mach.report.phase.PhaseStatus` | the standing of the module's load |
| `home` | `mach.base.source.provider.Home` | the package that owns it and its file there |
| `artifact` | `mach.base.intern.StrId` | the artifact whose walk first reached it, what `$bin.name` reads |
| `gated` | `bool` | it was first reached from an artifact this target does not build, or a union reached it only under another target's arms (see build_reach) |
| `outcome` | `mach.front.comptime.gate.GateOutcome` | the outcome of its module-scope gates |
| `ctx` | `mach.front.comptime.scope.ComptimeCtx` | its build target's frame, which every later phase reads |
| `frame` | `u32` | the frame the walk is in: FRAME_BUILD or a union tuple |
| `frames` | `*mach.front.comptime.scope.ComptimeCtx` | its frame under each union tuple, which only the load reads |
| `walked` | `*bool` | the declarations the walk reached in its build target's frame, by declaration |
| `frame_walked` | `*bool` | the declarations the walk reached under each union tuple, `decl_count` per tuple |
| `reached` | `*bool` | the union tuples a walk reached it under |
| `deps` | `std.collections.vector.Vector[mach.session.module.StableModuleId]` | the modules it imports |
| `lowers` | `std.collections.vector.Vector[mach.session.module.StableModuleId]` | the modules it imports through a `use` or `fwd` not marked `#[testing]`, which a build that runs no tests emits along: a module reached only through `#[testing] use` is checked and emitted only in a test build |
| `consts` | `std.collections.vector.Vector[Const]` | the public constants it exports to its importers' gates |
| `bindings` | `std.collections.vector.Vector[Binding]` | one record per import declaration the walk reached |
| `binders` | `DemandIndex` | where a demand finds each name a declaration list binds, built once per syntax (see demand_index) |

### module_dnit

```mach
pub fun module_dnit(a: *std.allocator.Allocator, m: *Module)
```

release what a module's load record owns: its contexts, walk marks, union frames, lists and demand index

| Parameter | |
| :--- | :--- |
| `a` | the allocator the record's storage came from, the loader's |
| `m` | the record to release |

### walked

```mach
pub fun walked(m: *Module, did: mach.front.ast.id.DeclId) bool
```

whether the walk reached a declaration in the module's build target's frame

### binding_for

```mach
pub fun binding_for(m: *Module, decl: mach.front.ast.id.DeclId) opt[*Binding]
```

the record of what the import declaration `decl` binds, none when the load recorded none

## Row

```mach
pub rec Row {
    name:   mach.base.intern.StrId;
    stable: mach.session.module.StableModuleId;
    file:   mach.base.source.FileId;
    load:   *Module;
}
```

a module as the load's host holds it: its identity and its load record

## Host

```mach
pub rec Host {
    ctx:     ptr;
    add:     fun(ptr, mach.base.intern.StrId, mach.session.module.StableModuleId) res[mach.session.module.ModuleId, mach.base.fail.Fail];
    row:     fun(ptr, mach.session.module.ModuleId) Row;
    syntax:  fun(ptr, mach.session.module.ModuleId) *mach.front.ast.Ast;
    text:    fun(ptr, str) opt[mach.base.source.provider.Text];
    parse:   fun(ptr, mach.session.module.ModuleId, mach.base.source.FileId, bool) err[mach.base.fail.Fail];
    context: fun(ptr, mach.base.intern.StrId) res[mach.front.comptime.scope.ComptimeCtx, mach.base.fail.Fail];
    done:    fun(ptr, mach.session.module.ModuleId) err[mach.base.fail.Fail];
}
```

what the load asks of the program that runs it, which owns the module table and the parses.
the host runs each of the load's entries (module_load, module_load_one, module_reload and
decorators_record) whole, and replaces no module's syntax while one runs but through `parse`

| Field | Type | |
| :--- | :--- | :--- |
| `ctx` | `ptr` | the host's own state |
| `add` | `fun(ptr, mach.base.intern.StrId, mach.session.module.StableModuleId) res[mach.session.module.ModuleId, mach.base.fail.Fail]` | make a row for a module with this name and stable id, with no file or syntax yet, and give its id; ids are given in order from zero |
| `row` | `fun(ptr, mach.session.module.ModuleId) Row` | a module's row; the load record it points at stays where it is |
| `syntax` | `fun(ptr, mach.session.module.ModuleId) *mach.front.ast.Ast` | a module's current syntax, nil where it has none |
| `text` | `fun(ptr, str) opt[mach.base.source.provider.Text]` | the text a module's file at this path was read ahead with, borrowed from the host, or none for the load to read it itself |
| `parse` | `fun(ptr, mach.session.module.ModuleId, mach.base.source.FileId, bool) err[mach.base.fail.Fail]` | give the module this file and make its syntax current; `reported` says the load reached it by walking, so a progress readout lists it |
| `context` | `fun(ptr, mach.base.intern.StrId) res[mach.front.comptime.scope.ComptimeCtx, mach.base.fail.Fail]` | the comptime context of a module of `artifact` under the build target, with nothing of the module itself |
| `done` | `fun(ptr, mach.session.module.ModuleId) err[mach.base.fail.Fail]` | a module and everything it imports finished loading |

## Loader

```mach
pub rec Loader {
    s:           *mach.session.Session;
    alloc:       *std.allocator.Allocator;
    host:        Host;
    sources:     mach.base.source.provider.Provider;
    diags:       *mach.base.diagnostic.DiagnosticStore;
    by_name:     std.collections.map.Map[mach.base.intern.StrId, mach.session.module.ModuleId];
    by_stable:   std.collections.map.Map[mach.session.module.StableModuleId, mach.session.module.ModuleId];
    states:      std.collections.vector.Vector[State];
    held:        std.collections.vector.Vector[Held];
    entry:       u64;
    named:       std.collections.map.Map[mach.base.intern.StrId, Named];
    demands:     std.collections.vector.Vector[Demand];
    union:       bool;
    tuples:      *Tuple;
    tuple_count: u32;
    tuple_cap:   u32;
    filter:      *bool;
    foreign:     bool;
    artifact:    mach.base.intern.StrId;
    dirty:       bool;
}
```

the front end's loading stage: from the modules it is asked for, it finds every module they
reach through the source provider, decides each one's declaring gates and binds the constants
those gates read, and leaves a load record on each

| Field | Type | |
| :--- | :--- | :--- |
| `host` | `Host` | what runs the load, bound before each use |
| `sources` | `mach.base.source.provider.Provider` | the module namespace, bound before each use |
| `diags` | `*mach.base.diagnostic.DiagnosticStore` | where the load reports, bound before each use |
| `by_name` | `std.collections.map.Map[mach.base.intern.StrId, mach.session.module.ModuleId]` | each module's id by its fqn |
| `by_stable` | `std.collections.map.Map[mach.session.module.StableModuleId, mach.session.module.ModuleId]` | each module's id by its stable id |
| `states` | `std.collections.vector.Vector[State]` | how far each module is loaded, by id |
| `held` | `std.collections.vector.Vector[Held]` | what the loader holds of each module, by id |
| `entry` | `u64` | the entry running now, numbering each from 1 |
| `named` | `std.collections.map.Map[mach.base.intern.StrId, Named]` | whether each path a `use` or `fwd` spells names a module, as the provider answered the entry it was asked in |
| `demands` | `std.collections.vector.Vector[Demand]` | the gates and constants the load is deciding, innermost last |
| `union` | `bool` | the build decides every gate under each of `tuples` as well as its own target |
| `filter` | `*bool` | the tuples the walk is under now, nil for all of them |
| `foreign` | `bool` | the walk is under an artifact this target does not build |
| `artifact` | `mach.base.intern.StrId` | the artifact a module reached now is attributed to |
| `dirty` | `bool` | a module reached under a new tuple after it loaded was walked again, which can give it dependencies the host's order placed after it |

### init

```mach
pub fun init(s: *mach.session.Session, a: *std.allocator.Allocator) Loader
```

a loader with no modules, no host and no union tuples

| Parameter | |
| :--- | :--- |
| `s` | the session whose interner, sources and module ids the load uses |
| `a` | the allocator the loader's tables grow from |
| **returns** | the loader, bound with `bind` before each use and released with `dnit` |

### dnit

```mach
pub fun dnit(l: *Loader)
```

release what the loader owns; each module's record is released by its row's owner

### bind

```mach
pub fun bind(l: *Loader, host: Host, sources: mach.base.source.provider.Provider, diags: *mach.base.diagnostic.DiagnosticStore)
```

point the loader at what runs it, which may have moved since its last use

### unbind

```mach
pub fun unbind(l: *Loader)
```

drop the host `bind` gave, whose context lives only as long as the walk it serves

### tuples_set

```mach
pub fun tuples_set(l: *Loader, tuples: *Tuple, count: u32, cap: u32)
```

make a union build: every gate is decided under each of `count` tuples, which the loader owns

### module_find

```mach
pub fun module_find(l: *Loader, fqn: mach.base.intern.StrId) opt[mach.session.module.ModuleId]
```

the id of the module named `fqn`, none when the load has not reached it

### module_of

```mach
pub fun module_of(l: *Loader, sid: mach.session.module.StableModuleId) mach.session.module.ModuleId
```

the id of the module a load product names by stable id, MODULE_NIL when the load has not reached it

### gated_at

```mach
pub fun gated_at(l: *Loader, mid: mach.session.module.ModuleId) bool
```

whether the module is gated out of this build

### module_count

```mach
pub fun module_count(l: *Loader) u32
```

how many modules the load has reached

| Parameter | |
| :--- | :--- |
| `l` | the loader |
| **returns** | the module count, one past the highest module index |

### module_load_one

```mach
pub fun module_load_one(l: *Loader, fqn: mach.base.intern.StrId) res[mach.session.module.ModuleId, mach.base.fail.Fail]
```

parse the module `fqn` names without walking it, as a root a later phase reads alone

### module_load

```mach
pub fun module_load(l: *Loader, fqn: mach.base.intern.StrId) res[mach.session.module.ModuleId, mach.base.fail.Fail]
```

load the module `fqn` names and everything it reaches: each is parsed, its gates decided and
its imports loaded first, then handed to the host as done

### module_reload

```mach
pub fun module_reload(l: *Loader, mid: mach.session.module.ModuleId) res[bool, mach.base.fail.Fail]
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
pub fun build_reach(l: *Loader, reached: *bool) err[mach.base.fail.Fail]
```

mark in `reached`, by module index, every module the build target's own frame reaches from the
modules marked there already: through each import declaration at module scope and in each arm
its frame selected. a module the union loaded only under another tuple's arms stays unmarked

### decorators_record

```mach
pub fun decorators_record(l: *Loader, mid: mach.session.module.ModuleId) err[mach.base.fail.Fail]
```

records the load's reading of the module's decorator arguments, in the build target's frame the
walk bound: the string each argument that takes one evaluates to, and what each `embed` argument names

## Functions

### embeds_record

```mach
pub fun embeds_record(s: *mach.session.Session, c: *mach.front.comptime.scope.ComptimeCtx, a: *mach.front.ast.Ast, fqn: mach.base.intern.StrId,
    reached: *bool) err[mach.base.fail.Fail]
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

