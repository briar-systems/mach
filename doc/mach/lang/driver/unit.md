# mach.lang.driver.unit

one object, of a module or an instance unit, made in two steps. `lower` lowers the unit's
code into a mirl module on a heap of its own and hands both back as one value, which its
owner keeps until `compile` has verified it, written it as text when asked and compiled it
into an object; `release` then tears the heap down. compiling reads the lowered unit and its
job and writes only what it returns, so compiles of different units can run side by side on
one shared target (mirl#406). each step times the stages it runs and keeps the counts each
owner holds of the module, for the readout

## rec Codegen

```mach
pub rec Codegen;
```

how mirl compiles a module: its level, whether the schedule's own checks run, whether
the linker may relax the code, and the float relaxations permitted

## rec UnitLowering

```mach
pub rec UnitLowering;
```

an instance unit's lowering: its request, its name and its members in the order they lower

## tag Lowering

```mach
pub tag Lowering: u8 {
    module: *lower_context.LowerRequest;
    unit:   UnitLowering;
}
```

what a step lowers: a module, or an instance unit. the request's `malloc` holds the facts
and any refusal

## rec Lowered

```mach
pub rec Lowered;
```

a unit lowered and not yet compiled: the heap its module lives on, the module, the facts
its lowering kept and what lowering it measured. it lives where `lower` was told to put it
until `release`

lower: lowering the module into mirl
functions: the module's functions, as mirl's module holds them
ir_count: the instructions lowering built into its bodies, as mirl's bodies hold them

## rec Job

```mach
pub rec Job;
```

how one lowered unit is compiled, all of it read only to the step

back: the assembled target, shared by every step and never written
codegen: how mirl compiles the module
out: where the object lives, which outlives the step
ir_path: where mirl's text of the module is written before it compiles, "" for nowhere
asm_path: where masc's listing of the object is written as it compiles, "" for nowhere
name: the name of the object it makes, the module's, its test object's or the unit's, no other object's in a link, which
          refusals name and the object's format may name symbols after
timed: whether mirl times and counts its stages, which only a readout reads
sources: the texts a failure or warning of the compile points into

## rec Stats

```mach
pub rec Stats;
```

what compiling one lowered unit measured: mach's verifier and mirl's own stages and counts

verify: mirl's verifier over the lowered module
verifies: the verifier runs mach made over it
blocks: the blocks the unit's heap allocated, lowering and compiling, as std's heap counts them, and not what the
          compile allocates on the object's allocator, which outlives the step
bytes: the bytes of those allocations
peak: the most bytes the unit's heap mapped
mirl: mirl's stages and counts, masc's beside them, all zero for an untimed job

## rec Made

```mach
pub rec Made;
```

a unit's object and the warnings mirl gave compiling it, owned by the caller, and what the
compile measured

## fun lower

```mach
pub fun lower(lowering: Lowering, back: *backend.Back, spans: *heap.Pool, out: *A.Allocator) res[*Lowered, fail.Fail];
```

the unit `lowering` names lowered for `back`'s target onto a heap drawing its spans from
`spans`, which the unit owns alone and so takes no lock, the value held in `out` until
`release`, or why it was not lowered

## fun release

```mach
pub fun release(out: *A.Allocator, l: *Lowered);
```

tear down what `lower` made in `out`: the heap and the module on it. the facts are the
holder's, who gives them back or hands them on

## fun compile

```mach
pub fun compile(job: *Job, l: *Lowered) res[Made, fail.Fail];
```

the object of the lowered unit `l`, compiled as `job` says, or why it was not made.
compiling spends the module, so a lowered unit is compiled once

