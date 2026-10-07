# mach.lang.fe.comptime.scope

the scope a comptime evaluation runs in: the description of the build, the stages'
bindings over a module, its frames and the gates it decided

## def GateState

```mach
pub def GateState: u8
```

## val GATE_STATE_UNKNOWN

```mach
pub val GATE_STATE_UNKNOWN:  GateState = 0
```

## val GATE_STATE_TRUE

```mach
pub val GATE_STATE_TRUE:     GateState = 1
```

## val GATE_STATE_FALSE

```mach
pub val GATE_STATE_FALSE:    GateState = 2
```

## val GATE_STATE_REJECTED

```mach
pub val GATE_STATE_REJECTED: GateState = 3
```

## rec NamedConst

```mach
pub rec NamedConst;
```

## rec FrameMark

```mach
pub rec FrameMark;
```

## rec ComptimeEnv

```mach
pub rec ComptimeEnv;
```

## rec BuildFacts

```mach
pub rec BuildFacts;
```

the build a program is compiled by and for: its mode, the compiler, the project, the
names its target entry gives the os, isa, abi and platform, the binary, and whether it
is a union build

## fun build_facts

```mach
pub fun build_facts(mode_id: u32, pie: u32, compiler_name: intern.StrId, compiler_ver: intern.StrId) BuildFacts;
```

the build facts of a build that names no project, artifact or target entry

## rec SourceFacts

```mach
pub rec SourceFacts;
```

the module being compiled: the source map its files are read from, its fully qualified
name, its file relative to the root of the project that owns it, and that project's id
and version

## def Stage

```mach
pub def Stage: u8
```

the stages that bind comptime state over a module, in the order they run; each binds into its
own bindings and reads those of the stages before it

## val STAGE_LOAD

```mach
pub val STAGE_LOAD:    Stage = 0
```

## val STAGE_RESOLVE

```mach
pub val STAGE_RESOLVE: Stage = 1
```

## val STAGE_SEMA

```mach
pub val STAGE_SEMA:    Stage = 2
```

## val STAGE_LOWER

```mach
pub val STAGE_LOWER:   Stage = 3
```

## def EmbedPathKind

```mach
pub def EmbedPathKind: u8
```

what the load made of an `embed` argument

EMBED_PATH_RESOLVED: it names the file at `text`
EMBED_PATH_NO_LOCATION: the declaring file has no path on disk to resolve it against
EMBED_PATH_REFUSED: its path template is refused, for the reason `text`
EMBED_PATH_ESCAPED: it names a file outside the project root, which is never read

## val EMBED_PATH_RESOLVED

```mach
pub val EMBED_PATH_RESOLVED:    EmbedPathKind = 0
```

## val EMBED_PATH_NO_LOCATION

```mach
pub val EMBED_PATH_NO_LOCATION: EmbedPathKind = 1
```

## val EMBED_PATH_REFUSED

```mach
pub val EMBED_PATH_REFUSED:     EmbedPathKind = 2
```

## val EMBED_PATH_ESCAPED

```mach
pub val EMBED_PATH_ESCAPED:     EmbedPathKind = 3
```

## rec EmbedPath

```mach
pub rec EmbedPath;
```

## rec Bindings

```mach
pub rec Bindings;
```

what one stage binds over one module, which that stage alone writes and every later stage reads

entries: its constants, innermost last
deferred: the names whose float width only type checking can read
gate_states: the gates it decided, by condition
strings: the string each decorator argument evaluates to, by argument; only the load records them
embeds: what each `embed` argument names, by argument; only the load records them

## fun bindings_init

```mach
pub fun bindings_init(alloc: *A.Allocator) Bindings;
```

## fun bindings_dnit

```mach
pub fun bindings_dnit(b: *Bindings);
```

## fun bindings_copy

```mach
pub fun bindings_copy(b: *Bindings, alloc: *A.Allocator) res[Bindings, fail.Fail];
```

an owned copy of `b` from `alloc`, which reads as `b` does

## fun lowering_over

```mach
pub fun lowering_over(env: ComptimeEnv, load: *Bindings, resolved: *Bindings, typed: *Bindings, alloc: *A.Allocator) ComptimeCtx;
```

a lowering scope over the description `env` that reads the bindings the load, resolve and sema
made, and binds its own from `alloc`; nothing it binds reaches the bindings it reads

## rec ComptimeCtx

```mach
pub rec ComptimeCtx;
```

the scope one stage evaluates a module in: the shared description of the build and the module,
the bindings of the stages before it, which it only reads, and its own, which it alone writes

stage: the stage whose bindings `own` holds
below: each earlier stage's bindings by stage, nil where the scope reads none

## fun environment

```mach
pub fun environment(c: *ComptimeCtx) ComptimeEnv;
```

## fun init

```mach
pub fun init(alloc: *A.Allocator, target: resolved.Facts, build: BuildFacts) ComptimeCtx;
```

a load scope over the target and build `target` and `build` describe, with no module yet

## fun stage_init

```mach
pub fun stage_init(load: *ComptimeCtx, stage: Stage, alloc: *A.Allocator) ComptimeCtx;
```

the scope `stage` evaluates a module in, over the load's scope of the module: the load's
description and bindings, with every stage between them left for `layer_set`. what the stage
binds is allocated from `alloc`, which outlives the product the bindings are published to

## fun layer_set

```mach
pub fun layer_set(c: *ComptimeCtx, stage: Stage, b: *Bindings);
```

read the bindings `b` an earlier stage made, beneath the scope's own

## fun layer_at

```mach
pub fun layer_at(c: *ComptimeCtx, stage: Stage) *Bindings;
```

the bindings of the earlier stage `stage` the scope reads, nil where it reads none

## fun reader_init

```mach
pub fun reader_init(c: *ComptimeCtx) ComptimeCtx;
```

a scope that reads everything `c` reads and binds, with nothing of its own, for evaluating in
another stage's or another module's scope without writing to it

## fun publish

```mach
pub fun publish(c: *ComptimeCtx) Bindings;
```

hand the bindings the scope's stage made to its product; the scope keeps none

## fun set_source_context

```mach
pub fun set_source_context(c: *ComptimeCtx, sources: *lang_source.SourceMap, module: intern.StrId, file: intern.StrId,
owner_id: intern.StrId, owner_ver: intern.StrId);
```

the module a context compiles, what `$mach.source.*` and `$mach.project.*` read

module: the module's fully qualified name
file: its file, relative to the root of the project that owns it
owner_id: the owning project's `[project].id`
owner_ver: the owning project's `[project].version`

## fun dnit

```mach
pub fun dnit(c: *ComptimeCtx);
```

release the scope's own bindings; what it reads below belongs to the stages that made it

## fun prepare_gates

```mach
pub fun prepare_gates(c: *ComptimeCtx, expr_count: u32) err[fail.Fail];
```

size the scope's own gate decisions for a module tree of `expr_count` expressions

## fun gate_state

```mach
pub fun gate_state(c: *ComptimeCtx, eid: ast_id.ExprId) GateState;
```

the decision on a gate: the scope's own, else the latest stage's below it that made one

## fun set_gate_state

```mach
pub fun set_gate_state(c: *ComptimeCtx, eid: ast_id.ExprId, state: GateState);
```

## fun defer_float_width

```mach
pub fun defer_float_width(c: *ComptimeCtx, name: intern.StrId) err[fail.Fail];
```

## fun float_width_deferred

```mach
pub fun float_width_deferred(c: *ComptimeCtx, name: intern.StrId) bool;
```

## fun bind

```mach
pub fun bind(c: *ComptimeCtx, name: intern.StrId, value: comptime_value.CTValue) err[fail.Fail];
```

## fun bind_gated

```mach
pub fun bind_gated(c: *ComptimeCtx, name: intern.StrId, value: comptime_value.CTValue, gated: bool) err[fail.Fail];
```

## fun frame_open

```mach
pub fun frame_open(c: *ComptimeCtx) res[FrameMark, fail.Fail];
```

## fun frame_close

```mach
pub fun frame_close(c: *ComptimeCtx, mark: FrameMark) err[fail.Fail];
```

## fun lookup

```mach
pub fun lookup(c: *ComptimeCtx, name: intern.StrId) opt[NamedConst];
```

the constant `name` is bound to: the scope's own latest binding, else the latest stage's below it

## fun decorator_string_set

```mach
pub fun decorator_string_set(c: *ComptimeCtx, arg: ast_id.ExprId, value: intern.StrId) err[fail.Fail];
```

record the string the decorator argument `arg` evaluates to

## fun decorator_string_at

```mach
pub fun decorator_string_at(c: *ComptimeCtx, arg: ast_id.ExprId) intern.StrId;
```

the string the decorator argument `arg` evaluates to, as the load recorded it; STR_NIL when it is
not a constant string

## fun decorator_embed_set

```mach
pub fun decorator_embed_set(c: *ComptimeCtx, arg: ast_id.ExprId, path: EmbedPath) err[fail.Fail];
```

record what the `embed` argument `arg` names

## fun decorator_embed_at

```mach
pub fun decorator_embed_at(c: *ComptimeCtx, arg: ast_id.ExprId) opt[EmbedPath];
```

what the `embed` argument `arg` names, as the load recorded it; none when it recorded nothing

## fun decorator_embed_file

```mach
pub fun decorator_embed_file(c: *ComptimeCtx, arg: ast_id.ExprId) intern.StrId;
```

the file the `embed` argument `arg` names, as the load resolved it; STR_NIL when it names none
the build reads

## fun decorators_clear

```mach
pub fun decorators_clear(c: *ComptimeCtx);
```

drop every decorator record of the scope's own, before they are recorded again

## fun field_type_of_binding

```mach
pub fun field_type_of_binding[T](c: *ComptimeCtx, cap_ctx: *T, caps: comptime_capability.PhaseCapabilities[T], name: intern.StrId) res[opt[u32], comptime_failure.EvalFail];
```

