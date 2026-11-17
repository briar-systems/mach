[mach](../README.md) / [target](README.md) / **facts**

# mach.target.facts

what a program may ask of its target while it compiles, read once from the back end a
build opens: the tag ids of mink's rows, masc's selection, and what mirl's assembled
target lays out and guarantees

Source: [`src/target/facts.mach`](../../../src/target/facts.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.float`](../base/float.md), [`mach.target.assembled`](assembled.md), [`mach.target.ct`](ct.md), [`mach.target.resolved`](resolved.md).

| Type | | |
| :--- | :--- | :--- |
| [`VaList`](#valist) | `tag` | what mirl says of the `va_list` of the target's own convention |
| [`Listed`](#listed) | `rec` | the size and alignment of a `va_list` |
| [`Registers`](#registers) | `rec` | the vector registers a vector aligns to: `narrow` the narrowest and `widest` the widest the build meets, in bits |
| [`Nans`](#nans) | `rec` | the nan a target's float operations return in each format, binary16, binary32 and binary64 at the index of mirl's format, which mirl's folder reads |
| [`Facts`](#facts) | `rec` | what a program may ask of its target while it compiles: the tag ids, the extents of its primitives and vectors, the selection its extensions are read against, what a float operation makes of a NaN, and whether its functions take `asm`, as mirl's capabilities of the target state it |

| On `Nans` | |
| :--- | :--- |
| [`nans_none`](#nans_none) | no format's rule stated, as where no target is selected |
| [`nans_at`](#nans_at) | the rule of the format of width `w`, binary64 for any other width |
| [`nans_held`](#nans_held) | whether any format's rule is stated, as it is wherever a target is selected |

| On `Facts` | |
| :--- | :--- |
| [`of`](#of) | the facts of the target `t` mirl assembled as `x`: mink's rows by their tag ids, masc's selection, and what mirl lays out and guarantees |
| [`none`](#none) | no target's facts, as where no target is selected |

---

## VaList

```mach
pub tag VaList: u8 {
    listed: Listed;
    unlisted;
    refused: str;
}
```

what mirl says of the `va_list` of the target's own convention

| Case | Payload | |
| :--- | :--- | :--- |
| `listed` | `Listed` | its size and alignment under the target's data layout |
| `unlisted` |  | the convention has no `va_list` |
| `refused` | `str` | mirl answers none for the target, with why |

## Listed

```mach
pub rec Listed {
    size:  u32;
    align: u32;
}
```

the size and alignment of a `va_list`

## Registers

```mach
pub rec Registers {
    narrow: u32;
    widest: u32;
}
```

the vector registers a vector aligns to: `narrow` the narrowest and `widest` the widest
the build meets, in bits

## Nans

```mach
pub rec Nans {
    formats: [3]mirl.target.capability.Nans;
}
```

the nan a target's float operations return in each format, binary16, binary32
and binary64 at the index of mirl's format, which mirl's folder reads

### nans_none

```mach
pub fun nans_none() Nans
```

no format's rule stated, as where no target is selected

### nans_at

```mach
pub fun nans_at(n: *Nans, w: mach.base.float.FloatWidth) mirl.target.capability.Nans
```

the rule of the format of width `w`, binary64 for any other width

### nans_held

```mach
pub fun nans_held(n: *Nans) bool
```

whether any format's rule is stated, as it is wherever a target is selected

## Facts

```mach
pub rec Facts {
    os_id:         u32;
    arch_id:       u32;
    abi_id:        u32;
    pointer_width: u32;
    vector_bits:   u32;
    register_bits: u32;
    selected:      mach.target.resolved.Selected;
    has_assembly:  bool;
    # whether a pointer is an address, or on a logical target a reference to a declared type
    flat_addressing: bool;
    # the nan each float format's operations return, which comptime folds by
    nans: Nans;
    # the `va_list` of the target's convention, which `abi_type("va_list")` declares
    va_list: VaList;
    # the scalar multiplies mirl runs in time independent of their operands, which
    # `$mach.build.ct_mul` reads
    multiplies: mach.target.ct.CtMulMask;
    # the build guarantees every timing mode a soft constant-time function's code may be held
    # under, so a hard function may call a soft one
    soft_guaranteed: bool;
}
```

what a program may ask of its target while it compiles: the tag ids, the
extents of its primitives and vectors, the selection its extensions are read
against, what a float operation makes of a NaN, and whether its functions take `asm`,
as mirl's capabilities of the target state it

### of

```mach
pub fun of(a: *std.allocator.Allocator, x: *mach.target.assembled.Assembled, t: *mach.target.resolved.Target) res[Facts, mach.base.fail.Fail]
```

the facts of the target `t` mirl assembled as `x`: mink's rows by their tag ids, masc's
selection, and what mirl lays out and guarantees

### none

```mach
pub fun none() Facts
```

no target's facts, as where no target is selected

