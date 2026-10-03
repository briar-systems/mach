# mach.lang.me.pass

the contract every middle-end pass fills. a pass is one function over a
context, true when it changed the module, beside the predicate that says
whether it applies to a run at all. each pass declares itself once in its own
module; the pipeline's schedule decides when it runs, which passes follow it
and how the module is verified after it

## rec Context

```mach
pub rec Context;
```

what a pass reads besides its own code: every pass takes the same record and
reads only what it needs

## def Run

```mach
pub def Run: fun(*Context) res[bool, fail.Fail]
```

a pass over `ctx.module`, true when it changed it

## def Gate

```mach
pub def Gate: fun(*Context) bool
```

whether a pass applies to a run

## rec Pass

```mach
pub rec Pass;
```

a member of the contract

## fun context

```mach
pub fun context(m: *me_ir.Module, tgt: *lang_target.Target, workspace: *scratch.Workspace) Context;
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

