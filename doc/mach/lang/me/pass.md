# mach.lang.me.pass

the contract every middle-end pass fills. most passes are a visit to one
function, true when it changed it, beside the functions it skips; the driver
here walks the module once for them. a pass whose unit is the whole module
runs over the context instead. each pass declares itself once in its own
module; the pipeline's schedule decides when it runs, which passes follow it
and how the module is verified after it

## rec Context

```mach
pub rec Context;
```

what a pass reads besides its own code: every pass takes the same record and
reads only what it needs

## rec Function

```mach
pub rec Function;
```

the one function a visit is given

## def Run

```mach
pub def Run: fun(*Context) res[bool, fail.Fail]
```

a pass over `ctx.module` as a whole, true when it changed it

## def Visit

```mach
pub def Visit: fun(*Function, *A.Allocator) res[bool, fail.Fail]
```

a pass over one function, true when it changed it; `alloc` is the workspace,
reclaimed after every visit

## def Gate

```mach
pub def Gate: fun(*Context) bool
```

whether a pass applies to a run

## def Filter

```mach
pub def Filter: fun(*Function) bool
```

whether a pass visits a function its skip flags leave in

## val SKIP_EXTERN

```mach
pub val SKIP_EXTERN:    u32 = 0x01
```

the functions a visiting pass skips, as flags

## val SKIP_EMPTY

```mach
pub val SKIP_EMPTY:     u32 = 0x02
```

## val SKIP_NO_INSTRS

```mach
pub val SKIP_NO_INSTRS: u32 = 0x04
```

## val SKIP_VOLATILE

```mach
pub val SKIP_VOLATILE:  u32 = 0x08
```

## val SKIP_SCALAR

```mach
pub val SKIP_SCALAR:    u32 = 0x10
```

## val SKIP_OBLIVIOUS

```mach
pub val SKIP_OBLIVIOUS: u32 = 0x20
```

## rec Pass

```mach
pub rec Pass;
```

a member of the contract: `visit` with its eligibility, or `run`

## fun context

```mach
pub fun context(m: *me_ir.Module, tgt: *resolved.Target, workspace: *scratch.Workspace) Context;
```

a context over `m` for `tgt` sharing `workspace`, offering nothing else, with
every setting off

## fun applies

```mach
pub fun applies(p: *Pass, ctx: *Context) bool;
```

whether `p` applies to the run `ctx` describes

## fun run

```mach
pub fun run(p: *Pass, ctx: *Context) res[bool, fail.Fail];
```

`p` over `ctx` when it applies; false when it does not

