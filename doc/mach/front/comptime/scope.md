[mach](../../README.md) / [front](../README.md) / [comptime](README.md) / **scope**

# mach.front.comptime.scope

the scope a comptime evaluation runs in: the description of the build, the stages'
bindings over a module, its frames and the gates it decided

<sub>source: [`src/front/comptime/scope.mach`](../../../../src/front/comptime/scope.mach) · imported by 28 modules</sub>

<sub>`intern` is [`mach.base.intern`](../../base/intern.md) · `comptime_value` is [`mach.front.comptime.value`](value.md) · `A` is `std.allocator` · `target_facts` is [`mach.target.facts`](../../target/facts.md) · `extent` is [`mach.type.query.extent`](../../type/query/extent.md) · `SourceFacts` is private · `map` is `std.collections.map` · `fail` is [`mach.base.fail`](../../base/fail.md) · `Stage` is private · `base_source` is [`mach.base.source`](../../base/source.md) · `ast_id` is [`mach.front.ast.id`](../ast/id.md) · `comptime_capability` is [`mach.front.comptime.capability`](capability.md) · `comptime_failure` is [`mach.front.comptime.failure`](failure.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`GateState`](#gatestate) | `def` |  |
| [`NamedConst`](#namedconst) | `rec` |  |
| [`FrameMark`](#framemark) | `rec` |  |
| [`ComptimeEnv`](#comptimeenv) | `rec` | laying: what the build lays its types out by, borrowed from its back end, nil where it         opened no target |
| [`BuildFacts`](#buildfacts) | `rec` | the build a program is compiled by and for: its mode, the compiler, the project, the names its target entry gives the os, isa, abi and platform, and the binary |
| [`EmbedPathKind`](#embedpathkind) | `def` | what the load made of an `embed` argument |
| [`EmbedPath`](#embedpath) | `rec` | artifact: the artifact whose output a resolved template names, nil when it names none |
| [`Bindings`](#bindings) | `rec` | what one stage binds over one module, which that stage alone writes and every later stage reads |
| [`ComptimeCtx`](#comptimectx) | `rec` | the scope one stage evaluates a module in: the shared description of the build and the module, the bindings of the stages before it, which it only reads, and its own, which it alone writes |

| On `GateState` | |
| :--- | :--- |
| [`GATE_STATE_UNKNOWN`](#gatestate) · [`GATE_STATE_TRUE`](#gatestate) · [`GATE_STATE_FALSE`](#gatestate) · [`GATE_STATE_REJECTED`](#gatestate) | |

| On `ComptimeEnv` | |
| :--- | :--- |
| [`lowering_over`](#lowering_over) | a lowering scope over the description `env` that reads the bindings the load, resolve and sema made, and binds its own from `alloc`; nothing it binds reaches the bindings it reads |

| On `BuildFacts` | |
| :--- | :--- |
| [`build_facts`](#build_facts) | the build facts of a build that names no project, artifact or target entry |
| [`init`](#init) | a load scope over the target and build `target` and `build` describe, with no module yet |

| On `EmbedPathKind` | |
| :--- | :--- |
| [`EMBED_PATH_RESOLVED`](#embedpathkind) · [`EMBED_PATH_NO_LOCATION`](#embedpathkind) · [`EMBED_PATH_REFUSED`](#embedpathkind) · [`EMBED_PATH_ESCAPED`](#embedpathkind) | |

| On `Bindings` | |
| :--- | :--- |
| [`bindings_init`](#bindings_init) | |
| [`bindings_dnit`](#bindings_dnit) | release a binding list; a nil list, or one never made, holds nothing |
| [`bindings_copy`](#bindings_copy) | an owned copy of `b` from `alloc`, which reads as `b` does |

| On `ComptimeCtx` | |
| :--- | :--- |
| [`environment`](#environment) | |
| [`stage_init`](#stage_init) | the scope `stage` evaluates a module in, over the load's scope of the module: the load's description and bindings, with every stage between them left for `layer_set` |
| [`lowering_prepare`](#lowering_prepare) | make or bring up to date the scope lowering binds a module's constants in, over what the module's load scope `load`, its resolution and its typing bound, nil where there is none |
| [`layer_set`](#layer_set) | read the bindings `b` an earlier stage made, beneath the scope's own |
| [`layer_at`](#layer_at) | the bindings of the earlier stage `stage` the scope reads, nil where it reads none |
| [`reader_init`](#reader_init) | a scope that reads everything `c` reads and binds, with nothing of its own, for evaluating in another stage's or another module's scope without writing to it |
| [`publish`](#publish) | hand the bindings the scope's stage made to its product; the scope keeps none |
| [`set_source_context`](#set_source_context) | the module a context compiles, what `$mach.source.*` and `$mach.project.*` read |
| [`dnit`](#dnit) | release the scope's own bindings; what it reads below belongs to the stages that made it |
| [`prepare_gates`](#prepare_gates) | size the scope's own gate decisions for a module tree of `expr_count` expressions |
| [`gate_state`](#gate_state) | the decision on a gate: the scope's own, else the latest stage's below it that made one |
| [`set_gate_state`](#set_gate_state) · [`defer_float_width`](#defer_float_width) · [`float_width_deferred`](#float_width_deferred) · [`bind`](#bind) · [`frame_begin`](#frame_begin) · [`frame_end`](#frame_end) | |
| [`lookup`](#lookup) | the constant `name` is bound to: the scope's own latest binding, else the latest stage's below it |
| [`decorator_string_set`](#decorator_string_set) | record the string the decorator argument `arg` evaluates to |
| [`decorator_string_at`](#decorator_string_at) | the string the decorator argument `arg` evaluates to, as the load recorded it; STR_NIL when it is not a constant string |
| [`decorator_embed_set`](#decorator_embed_set) | record what the `embed` argument `arg` names |
| [`decorator_embed_at`](#decorator_embed_at) | what the `embed` argument `arg` names, as the load recorded it; none when it recorded nothing |
| [`decorator_embed_file`](#decorator_embed_file) | the file the `embed` argument `arg` names, as the load resolved it; STR_NIL when it names none the build reads |
| [`decorators_clear`](#decorators_clear) | drop every decorator record of the scope's own, before they are recorded again |
| [`field_type_of_binding`](#field_type_of_binding) | |

| Constants | |
| :--- | :--- |
| [`STAGE_LOAD`](#constants) · [`STAGE_RESOLVE`](#constants) · [`STAGE_SEMA`](#constants) · [`STAGE_LOWER`](#constants) | |

---

## GateState

```mach
pub def GateState: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `GATE_STATE_UNKNOWN` | `0` |  |
| `GATE_STATE_TRUE` | `1` |  |
| `GATE_STATE_FALSE` | `2` |  |
| `GATE_STATE_REJECTED` | `3` |  |

## NamedConst

```mach
pub rec NamedConst {
    name:  intern.StrId;
    value: comptime_value.CTValue;
}
```

## FrameMark

```mach
pub rec FrameMark {
    owner:        ptr;
    entries_len:  u32;
    deferred_len: u32;
    generation:   u64;
    parent:       u64;
}
```

## ComptimeEnv

```mach
pub rec ComptimeEnv {
    alloc:  *A.Allocator;
    target: target_facts.Facts;
    laying: *extent.Laying;
    build:  BuildFacts;
    source: SourceFacts;
}
```

laying: what the build lays its types out by, borrowed from its back end, nil where it
        opened no target

### lowering_over

```mach
pub fun lowering_over(env: ComptimeEnv, load: *Bindings, resolved: *Bindings, typed: *Bindings, alloc: *A.Allocator) ComptimeCtx
```

a lowering scope over the description `env` that reads the bindings the load, resolve and sema
made, and binds its own from `alloc`; nothing it binds reaches the bindings it reads

## BuildFacts

```mach
pub rec BuildFacts {
    mode_id:       u32;
    pie:           u32;
    compiler_name: intern.StrId;
    compiler_ver:  intern.StrId;
    project_id:    intern.StrId;
    project_ver:   intern.StrId;
    os_name:       intern.StrId;
    isa_name:      intern.StrId;
    abi_name:      intern.StrId;
    platform_name: intern.StrId;
    bin_name:      intern.StrId;
}
```

the build a program is compiled by and for: its mode, the compiler, the project, the
names its target entry gives the os, isa, abi and platform, and the binary

### build_facts

```mach
pub fun build_facts(mode_id: u32, pie: u32, compiler_name: intern.StrId, compiler_ver: intern.StrId) BuildFacts
```

the build facts of a build that names no project, artifact or target entry

### init

```mach
pub fun init(alloc: *A.Allocator, target: target_facts.Facts, build: BuildFacts) ComptimeCtx
```

a load scope over the target and build `target` and `build` describe, with no module yet

## EmbedPathKind

```mach
pub def EmbedPathKind: u8
```

what the load made of an `embed` argument

| Field | Type | |
| :--- | :--- | :--- |
| `EMBED_PATH_RESOLVED` |  | it names the file at `text` |
| `EMBED_PATH_NO_LOCATION` |  | the declaring file has no path on disk to resolve it against |
| `EMBED_PATH_REFUSED` |  | its path template is refused, for the reason `text` |
| `EMBED_PATH_ESCAPED` |  | it names a file outside the project root, which is never read |

| Name | Value | |
| :--- | :--- | :--- |
| `EMBED_PATH_RESOLVED` | `0` |  |
| `EMBED_PATH_NO_LOCATION` | `1` |  |
| `EMBED_PATH_REFUSED` | `2` |  |
| `EMBED_PATH_ESCAPED` | `3` |  |

## EmbedPath

```mach
pub rec EmbedPath {
    kind:     EmbedPathKind;
    text:     intern.StrId;
    artifact: intern.StrId;
}
```

artifact: the artifact whose output a resolved template names, nil when it names none

## Bindings

```mach
pub rec Bindings {
    alloc:        *A.Allocator;
    entries:      *NamedConst;
    entries_len:  u32;
    entries_cap:  u32;
    shadows:      *u32;
    shadows_cap:  u32;
    latest:       map.Map[intern.StrId, u32];
    deferred:     *intern.StrId;
    deferred_len: u32;
    deferred_cap: u32;
    gate_states:  *GateState;
    gate_count:   u32;
    strings:      map.Map[u32, intern.StrId];
    embeds:       map.Map[u32, EmbedPath];
}
```

what one stage binds over one module, which that stage alone writes and every later stage reads

| Field | Type | |
| :--- | :--- | :--- |
| `entries` | `*NamedConst` | its constants, innermost last |
| `shadows` | `*u32` | per entry, one past the index of the entry of its name it shadows, 0 where none |
| `latest` | `map.Map[intern.StrId, u32]` | by name, one past the index of its innermost entry |
| `deferred` | `*intern.StrId` | the names whose float width only type checking can read |
| `gate_states` | `*GateState` | the gates it decided, by condition |
| `strings` | `map.Map[u32, intern.StrId]` | the string each decorator argument evaluates to, by argument; only the load records them |
| `embeds` | `map.Map[u32, EmbedPath]` | what each `embed` argument names, by argument; only the load records them |

### bindings_init

```mach
pub fun bindings_init(alloc: *A.Allocator) Bindings
```

### bindings_dnit

```mach
pub fun bindings_dnit(b: *Bindings)
```

release a binding list; a nil list, or one never made, holds nothing

### bindings_copy

```mach
pub fun bindings_copy(b: *Bindings, alloc: *A.Allocator) res[Bindings, fail.Fail]
```

an owned copy of `b` from `alloc`, which reads as `b` does

## ComptimeCtx

```mach
pub rec ComptimeCtx {
    env:              ComptimeEnv;
    stage:            Stage;
    own:              Bindings;
    below:            [STAGE_COUNT]*Bindings;
    frame_generation: u64;
    frame_top:        u64;
}
```

the scope one stage evaluates a module in: the shared description of the build and the module,
the bindings of the stages before it, which it only reads, and its own, which it alone writes

| Field | Type | |
| :--- | :--- | :--- |
| `stage` | `Stage` | the stage whose bindings `own` holds |
| `below` | `[STAGE_COUNT]*Bindings` | each earlier stage's bindings by stage, nil where the scope reads none |

### environment

```mach
pub fun environment(c: *ComptimeCtx) ComptimeEnv
```

### stage_init

```mach
pub fun stage_init(load: *ComptimeCtx, stage: Stage, alloc: *A.Allocator) ComptimeCtx
```

the scope `stage` evaluates a module in, over the load's scope of the module: the load's
description and bindings, with every stage between them left for `layer_set`. what the stage
binds is allocated from `alloc`, which outlives the product the bindings are published to

### lowering_prepare

```mach
pub fun lowering_prepare(scope: *ComptimeCtx, made: *bool, load: *ComptimeCtx, expr_count: u32,
    resolved: *Bindings, typed: *Bindings) err[fail.Fail]
```

make or bring up to date the scope lowering binds a module's constants in, over what the
module's load scope `load`, its resolution and its typing bound, nil where there is none.
`made` says the scope exists; it is made on the first call with room for the gate decisions
of a tree of `expr_count` expressions, and each call points its layers at the products given

### layer_set

```mach
pub fun layer_set(c: *ComptimeCtx, stage: Stage, b: *Bindings)
```

read the bindings `b` an earlier stage made, beneath the scope's own

### layer_at

```mach
pub fun layer_at(c: *ComptimeCtx, stage: Stage) *Bindings
```

the bindings of the earlier stage `stage` the scope reads, nil where it reads none

### reader_init

```mach
pub fun reader_init(c: *ComptimeCtx) ComptimeCtx
```

a scope that reads everything `c` reads and binds, with nothing of its own, for evaluating in
another stage's or another module's scope without writing to it

### publish

```mach
pub fun publish(c: *ComptimeCtx) Bindings
```

hand the bindings the scope's stage made to its product; the scope keeps none

### set_source_context

```mach
pub fun set_source_context(c: *ComptimeCtx, sources: *base_source.SourceMap, module: intern.StrId, file: intern.StrId,
    owner_id: intern.StrId, owner_ver: intern.StrId)
```

the module a context compiles, what `$mach.source.*` and `$mach.project.*` read

| Parameter | |
| :--- | :--- |
| `module` | the module's fully qualified name |
| `file` | its file, relative to the root of the project that owns it |
| `owner_id` | the owning project's `[project].id` |
| `owner_ver` | the owning project's `[project].version` |

### dnit

```mach
pub fun dnit(c: *ComptimeCtx)
```

release the scope's own bindings; what it reads below belongs to the stages that made it.
a nil scope holds nothing

### prepare_gates

```mach
pub fun prepare_gates(c: *ComptimeCtx, expr_count: u32) err[fail.Fail]
```

size the scope's own gate decisions for a module tree of `expr_count` expressions

### gate_state

```mach
pub fun gate_state(c: *ComptimeCtx, eid: ast_id.ExprId) GateState
```

the decision on a gate: the scope's own, else the latest stage's below it that made one

### set_gate_state

```mach
pub fun set_gate_state(c: *ComptimeCtx, eid: ast_id.ExprId, state: GateState)
```

### defer_float_width

```mach
pub fun defer_float_width(c: *ComptimeCtx, name: intern.StrId) err[fail.Fail]
```

### float_width_deferred

```mach
pub fun float_width_deferred(c: *ComptimeCtx, name: intern.StrId) bool
```

### bind

```mach
pub fun bind(c: *ComptimeCtx, name: intern.StrId, value: comptime_value.CTValue) err[fail.Fail]
```

### frame_begin

```mach
pub fun frame_begin(c: *ComptimeCtx) res[FrameMark, fail.Fail]
```

### frame_end

```mach
pub fun frame_end(c: *ComptimeCtx, mark: FrameMark) err[fail.Fail]
```

### lookup

```mach
pub fun lookup(c: *ComptimeCtx, name: intern.StrId) opt[NamedConst]
```

the constant `name` is bound to: the scope's own latest binding, else the latest stage's below it

### decorator_string_set

```mach
pub fun decorator_string_set(c: *ComptimeCtx, arg: ast_id.ExprId, value: intern.StrId) err[fail.Fail]
```

record the string the decorator argument `arg` evaluates to

### decorator_string_at

```mach
pub fun decorator_string_at(c: *ComptimeCtx, arg: ast_id.ExprId) intern.StrId
```

the string the decorator argument `arg` evaluates to, as the load recorded it; STR_NIL when it is
not a constant string

### decorator_embed_set

```mach
pub fun decorator_embed_set(c: *ComptimeCtx, arg: ast_id.ExprId, path: EmbedPath) err[fail.Fail]
```

record what the `embed` argument `arg` names

### decorator_embed_at

```mach
pub fun decorator_embed_at(c: *ComptimeCtx, arg: ast_id.ExprId) opt[EmbedPath]
```

what the `embed` argument `arg` names, as the load recorded it; none when it recorded nothing

### decorator_embed_file

```mach
pub fun decorator_embed_file(c: *ComptimeCtx, arg: ast_id.ExprId) intern.StrId
```

the file the `embed` argument `arg` names, as the load resolved it; STR_NIL when it names none
the build reads

### decorators_clear

```mach
pub fun decorators_clear(c: *ComptimeCtx)
```

drop every decorator record of the scope's own, before they are recorded again

### field_type_of_binding

```mach
pub fun field_type_of_binding[T](c: *ComptimeCtx, cap_ctx: *T, caps: comptime_capability.PhaseCapabilities[T], name: intern.StrId) res[opt[u32], comptime_failure.EvalFail]
```

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `STAGE_LOAD` | `Stage` | `0` |  |
| `STAGE_RESOLVE` | `Stage` | `1` |  |
| `STAGE_SEMA` | `Stage` | `2` |  |
| `STAGE_LOWER` | `Stage` | `3` |  |

