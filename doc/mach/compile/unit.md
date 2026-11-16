[mach](../README.md) / [compile](README.md) / **unit**

# mach.compile.unit

one object, of a module or an instance unit, made in two steps. `lower` lowers the unit's
code into a mirl module on a heap of its own and hands both back as one value, which its
owner keeps until `compile` has verified it, written it as text when asked and compiled it
into an object; `dnit` then tears the heap down. compiling reads the lowered unit and its
job and writes only what it returns, so compiles of different units can run side by side on
one shared target (mirl#406). each step times the stages it runs and keeps the counts each
owner holds of the module, for the readout

<sub>source: [`src/compile/unit.mach`](../../../src/compile/unit.mach) · imported by 1 module</sub>

<sub>`relaxation` is `mirl.pass.relaxation` · `lower_context` is [`mach.lower.context`](../lower/context.md) · `intern` is [`mach.base.intern`](../base/intern.md) · `sema_product` is [`mach.front.sema.product`](../front/sema/product.md) · `backend` is [`mach.compile.backend`](backend.md) · `heap` is `std.allocator.heap` · `A` is `std.allocator` · `fail` is [`mach.base.fail`](../base/fail.md) · `chrono_duration` is `std.chrono.duration` · `base_source` is [`mach.base.source`](../base/source.md) · `mirl_stats` is `mirl.base.stats` · `backend_compile` is [`mach.compile.backend.compile`](backend/compile.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Codegen`](#codegen) | `rec` | how mirl compiles a module: its level, whether the schedule's own checks run, whether the linker may relax the code, and the float relaxations permitted |
| [`UnitLowering`](#unitlowering) | `rec` | an instance unit's lowering: its request, its name and its members in the order they lower |
| [`Lowering`](#lowering) | `tag` | what a step lowers: a module, or an instance unit |
| [`Lowered`](#lowered) | `rec` | a unit lowered and not yet compiled: the heap its module lives on, the module, the facts its lowering kept and what lowering it measured |
| [`Job`](#job) | `rec` | how one lowered unit is compiled, all of it read only to the step |
| [`Stats`](#stats) | `rec` | what compiling one lowered unit measured: mach's verifier and mirl's own stages and counts |
| [`Made`](#made) | `rec` | a unit's object and the warnings mirl gave compiling it, owned by the caller, and what the compile measured |

| On `Lowering` | |
| :--- | :--- |
| [`lower`](#lower) | the unit `lowering` names lowered for `back`'s target onto a heap drawing its spans from `spans`, which the unit owns alone and so takes no lock, the value held in `out` until `dnit`, or why it was not lowered |

| On `Lowered` | |
| :--- | :--- |
| [`dnit`](#dnit) | tear down what `lower` made in `out`: the heap and the module on it |

| On `Job` | |
| :--- | :--- |
| [`compile`](#compile) | the object of the lowered unit `l`, compiled as `job` says, or why it was not made |

---

## Codegen

```mach
pub rec Codegen {
    level:   u8;
    checked: bool;
    relax:   bool;
    floats:  relaxation.Relaxations;
}
```

how mirl compiles a module: its level, whether the schedule's own checks run, whether
the linker may relax the code, and the float relaxations permitted

## UnitLowering

```mach
pub rec UnitLowering {
    req:      *lower_context.LowerRequest;
    name:     intern.StrId;
    members:  **sema_product.InstanceResult;
    defining: *lower_context.Defining;
    count:    u32;
}
```

an instance unit's lowering: its request, its name and its members in the order they lower

## Lowering

```mach
pub tag Lowering: u8 {
    module: *lower_context.LowerRequest;
    unit:   UnitLowering;
}
```

what a step lowers: a module, or an instance unit. the request's `malloc` holds the facts
and any refusal

### lower

```mach
pub fun lower(lowering: Lowering, back: *backend.Back, spans: *heap.Pool, out: *A.Allocator) res[*Lowered, fail.Fail]
```

the unit `lowering` names lowered for `back`'s target onto a heap drawing its spans from
`spans`, which the unit owns alone and so takes no lock, the value held in `out` until
`dnit`, or why it was not lowered

## Lowered

```mach
pub rec Lowered {
    src:       heap.Source;
    h:         heap.Heap;
    ir:        A.Allocator;
    m:         Module;
    facts:     lower_context.Unit;
    lower:     chrono_duration.Duration;
    functions: u64;
    ir_count:  u64;
}
```

a unit lowered and not yet compiled: the heap its module lives on, the module, the facts
its lowering kept and what lowering it measured. it lives where `lower` was told to put it
until `dnit`

| Field | Type | |
| :--- | :--- | :--- |
| `lower` | `chrono_duration.Duration` | lowering the module into mirl |
| `functions` | `u64` | the module's functions, as mirl's module holds them |
| `ir_count` | `u64` | the instructions lowering built into its bodies, as mirl's bodies hold them |

### dnit

```mach
pub fun dnit(out: *A.Allocator, l: *Lowered)
```

tear down what `lower` made in `out`: the heap and the module on it. the facts are the
holder's, who gives them back or hands them on

## Job

```mach
pub rec Job {
    back:     *backend.Back;
    codegen:  Codegen;
    out:      *A.Allocator;
    ir_path:  str;
    asm_path: str;
    name:     str;
    timed:    bool;
    sources:  *base_source.SourceMap;
}
```

how one lowered unit is compiled, all of it read only to the step

| Field | Type | |
| :--- | :--- | :--- |
| `back` | `*backend.Back` | the assembled target, shared by every step and never written |
| `codegen` | `Codegen` | how mirl compiles the module |
| `out` | `*A.Allocator` | where the object lives, which outlives the step |
| `ir_path` | `str` | where mirl's text of the module is written before it compiles, "" for nowhere |
| `asm_path` | `str` | where masc's listing of the object is written as it compiles, "" for nowhere |
| `name` | `str` | the name of the object it makes, the module's, its test object's or the unit's, no other object's in a link, which refusals name and the object's format may name symbols after |
| `timed` | `bool` | whether mirl times and counts its stages, which only a readout reads |
| `sources` | `*base_source.SourceMap` | the texts a failure or warning of the compile points into |

### compile

```mach
pub fun compile(job: *Job, l: *Lowered) res[Made, fail.Fail]
```

the object of the lowered unit `l`, compiled as `job` says, or why it was not made.
compiling spends the module, so a lowered unit is compiled once

## Stats

```mach
pub rec Stats {
    verify:   chrono_duration.Duration;
    verifies: u64;
    blocks:   u64;
    bytes:    u64;
    peak:     u64;
    mirl:     mirl_stats.Stats;
}
```

what compiling one lowered unit measured: mach's verifier and mirl's own stages and counts

| Field | Type | |
| :--- | :--- | :--- |
| `verify` | `chrono_duration.Duration` | mirl's verifier over the lowered module |
| `verifies` | `u64` | the verifier runs mach made over it |
| `blocks` | `u64` | the blocks the unit's heap allocated, lowering and compiling, as std's heap counts them, and not what the compile allocates on the object's allocator, which outlives the step |
| `bytes` | `u64` | the bytes of those allocations |
| `peak` | `u64` | the most bytes the unit's heap mapped |
| `mirl` | `mirl_stats.Stats` | mirl's stages and counts, masc's beside them, all zero for an untimed job |

## Made

```mach
pub rec Made {
    compiled: backend_compile.Compiled;
    stats:    Stats;
}
```

a unit's object and the warnings mirl gave compiling it, owned by the caller, and what the
compile measured

