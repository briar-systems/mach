# mach.lang.be.codegen.regalloc.spill

what a spilled value costs and how it is carried: rematerialization of
constants, pieces that reload a run of reads once, values spilled across
calls, and the frame slots the spills take

## fun slots_allocate

```mach
pub fun slots_allocate(ctx: *regalloc_context.Context) err[fail.Fail];
```

## fun plan_remat

```mach
pub fun plan_remat(tgt: *lang_target.Binding, ctx: *regalloc_context.Context) res[bool, fail.Fail];
```

a spilled value its one definition builds from a constant is built again
where it is read instead of kept in a slot: the definition goes, a
copy that reads it copies the constant, and every other reader reads a
fresh value defined by a copy of the definition just before it, which the
next round allocates as an interval of its own. no store, no slot, no
reload, so a constant hoisted out of a loop costs the loop, at worst, the
materialization it would have had in place. a value is offered this once,
and the fresh values never, so the rounds end

## fun discount_remat

```mach
pub fun discount_remat(tgt: *lang_target.Binding, ctx: *regalloc_context.Context) err[fail.Fail];
```

a value built again where it is read costs no store and no slot when it is
spilled, so it weighs half as much to keep in a register as one that would
be reloaded: of two values read as often, the one that can be rebuilt is the
one the scan gives up. the test is remat_def's, made for every value
in one pass

## fun plan_pieces

```mach
pub fun plan_pieces(tgt: *lang_target.Binding, ctx: *regalloc_context.Context) res[bool, fail.Fail];
```

second chance for a spilled value: its reads inside one block, between
the calls and the writes of the value, form runs, and a run of two or more
reads that would each reload is carried by a piece, a fresh value loaded
once before the first read. the next round scans the piece as an interval
of its own, so it takes a register only where one is free; a piece the
next round spills gives its reads back to the value and that value earns
no further pieces. every value is offered pieces once, so the rounds end

## fun across_calls

```mach
pub fun across_calls(tgt: *lang_target.Binding, ctx: *regalloc_context.Context) err[fail.Fail];
```

