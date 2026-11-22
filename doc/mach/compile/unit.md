[mach](../README.md) / [compile](README.md) / **unit**

# mach.compile.unit

one unit's object, normal or test, made in two steps: a unit is one module, or every module of an
artifact where the target's family compiles it whole. `lower` lowers the unit's
code into a mirl module on a heap of its own and hands both back as one value, which its
owner keeps until `compile` has bound the offers of other modules' bodies it was given,
verified it, written it as text when asked and compiled it into an object; `dnit` then tears
the heap down. compiling reads the lowered unit and its job and writes only what it returns,
so compiles of different units can run side by side on one shared target (mirl#406). each
step times the stages it runs and keeps the counts each owner holds of the module, for the
readout

Source: [`src/compile/unit.mach`](../../../src/compile/unit.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.compile.backend`](backend.md), [`mach.compile.backend.compile`](backend/compile.md), [`mach.compile.offer`](offer.md), [`mach.lower.context`](../lower/context.md).

| Type | | |
| :--- | :--- | :--- |
| [`Codegen`](#codegen) | `rec` | how mirl compiles a module: its optimisation setting, whether the schedule's own checks run, whether the linker may relax the code, whether the objects state frames for unwind tables, the float relaxations permitted, and whether a vector operation taken lane by lane is refused, as `simd = "require"` asks, and whether the object is closed, as the one object of a finished module that nothing else links against |
| [`Lowered`](#lowered) | `rec` | a unit lowered and not yet compiled: the heap its module lives on, the module, the facts its lowering kept and what lowering it measured |
| [`Job`](#job) | `rec` | how one lowered unit is compiled, all of it read only to the step |
| [`Stats`](#stats) | `rec` | what compiling one lowered unit measured: mach's verifier and mirl's own stages and counts |
| [`Made`](#made) | `rec` | a unit's object and the warnings mirl gave compiling it, owned by the caller, and what the compile measured |

| On `Lowered` | |
| :--- | :--- |
| [`lower`](#lower) | the modules the `count` requests at `reqs` name lowered for `back`'s target into one mirl module on a heap drawing its spans from `spans`, the pool every unit of the build shares under its lock, the value held in `out` until `dnit`, or why it was not lowered |
| [`dnit`](#dnit) | tear down what `lower` made in `out`: the heap and the module on it, unless compiling tore them down |

| On `Job` | |
| :--- | :--- |
| [`compile`](#compile) | the object of the lowered unit `l`, compiled as `job` says, or why it was not made, unplaced |

---

## Codegen

```mach
pub rec Codegen {
    optimise: Optimise;
    checked:  bool;
    relax:    bool;
    unwind:   bool;
    floats:   mirl.pass.relaxation.Relaxations;
    require:  bool;
    report:   bool;
    closed:   bool;
}
```

how mirl compiles a module: its optimisation setting, whether the schedule's own checks run, whether
the linker may relax the code, whether the objects state frames for unwind tables, the float relaxations permitted, and whether a vector
operation taken lane by lane is refused, as `simd = "require"` asks, and whether the object is closed, as the one
object of a finished module that nothing else links against

## Lowered

```mach
pub rec Lowered {
    src:       std.allocator.heap.Source;
    h:         std.allocator.heap.Heap;
    ir:        std.allocator.Allocator;
    m:         Module;
    facts:     mach.lower.context.Unit;
    lower:     std.chrono.duration.Duration;
    functions: u64;
    ir_count:  u64;
    offers:    Slice[mach.compile.offer.Candidate];
    spent:     bool;
}
```

a unit lowered and not yet compiled: the heap its module lives on, the module, the facts
its lowering kept and what lowering it measured. it lives where `lower` was told to put it
until `dnit`

| Field | Type | |
| :--- | :--- | :--- |
| `lower` | `std.chrono.duration.Duration` | lowering the module into mirl |
| `functions` | `u64` | the module's functions, as mirl's module holds them |
| `ir_count` | `u64` | the instructions lowering built into its bodies, as mirl's bodies hold them |
| `offers` | `Slice[mach.compile.offer.Candidate]` | the other modules' offers of their bodies the module may bind before it is compiled (compile/offer), views on the heap of offers handed over, which outlive the compile |
| `spent` | `bool` | compiling the module tore down the heap |

### lower

```mach
pub fun lower(reqs: *mach.lower.context.LowerRequest, count: usize, back: *mach.compile.backend.Back, spans: *std.allocator.heap.Pool, out: *std.allocator.Allocator) res[*Lowered, mach.base.fail.Fail]
```

the modules the `count` requests at `reqs` name lowered for `back`'s target into one mirl module
on a heap drawing its spans from `spans`, the pool every unit of the build shares under its lock, the
value held in `out` until `dnit`, or why it was not lowered. the requests' `malloc` holds the facts
and any refusal

### dnit

```mach
pub fun dnit(out: *std.allocator.Allocator, l: *Lowered)
```

tear down what `lower` made in `out`: the heap and the module on it, unless compiling tore them
down. the facts are the holder's, who gives them back or hands them on

## Job

```mach
pub rec Job {
    back:     *mach.compile.backend.Back;
    codegen:  Codegen;
    out:      *std.allocator.Allocator;
    ir_path:  str;
    asm_path: str;
    name:     str;
    timed:    bool;
    owner:    str;
}
```

how one lowered unit is compiled, all of it read only to the step

| Field | Type | |
| :--- | :--- | :--- |
| `back` | `*mach.compile.backend.Back` | the assembled target, shared by every step and never written |
| `codegen` | `Codegen` | how mirl compiles the module |
| `out` | `*std.allocator.Allocator` | where the object lives, which outlives the step |
| `ir_path` | `str` | where mirl's text of the module is written before it compiles, "" for nowhere |
| `asm_path` | `str` | where masc's listing of the object is written as it compiles, "" for nowhere |
| `name` | `str` | the name of the object it makes, the module's or its test object's, no other object's in a link, which refusals name and the object's format may name symbols after |
| `timed` | `bool` | whether mirl times and counts its stages, which only a readout reads |
| `owner` | `str` | the dependency whose code the module is, which a refusal names, nil for the project being built |

### compile

```mach
pub fun compile(job: *Job, l: *Lowered) res[Made, mach.compile.backend.compile.Unplaced]
```

the object of the lowered unit `l`, compiled as `job` says, or why it was not made, unplaced.
compiling spends the module and tears down the unit's heap, so a lowered unit is compiled once

## Stats

```mach
pub rec Stats {
    verify:   std.chrono.duration.Duration;
    verifies: u64;
    blocks:   u64;
    bytes:    u64;
    peak:     u64;
    mirl:     mirl.base.stats.Stats;
}
```

what compiling one lowered unit measured: mach's verifier and mirl's own stages and counts

| Field | Type | |
| :--- | :--- | :--- |
| `verify` | `std.chrono.duration.Duration` | mirl's verifier over the lowered module |
| `verifies` | `u64` | the verifier runs mach made over it |
| `blocks` | `u64` | the blocks the unit's heap allocated, lowering and compiling, as std's heap counts them, and not what the compile allocates on the object's allocator, which outlives the step |
| `bytes` | `u64` | the bytes of those allocations |
| `peak` | `u64` | the most bytes the unit's heap mapped |
| `mirl` | `mirl.base.stats.Stats` | mirl's stages and counts, masc's beside them, all zero for an untimed job |

## Made

```mach
pub rec Made {
    compiled: mach.compile.backend.compile.Compiled;
    stats:    Stats;
}
```

a unit's object and the warnings mirl gave compiling it, owned by the caller, and what the
compile measured

